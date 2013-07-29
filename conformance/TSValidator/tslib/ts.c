/*
 Copyright (c) 2012, Alex Giladi <alex.giladi@gmail.com>
 Copyright (c) 2013- Alex Giladi <alex.giladi@gmail.com> and Vlad Zbarsky <zbarsky@cornell.edu>
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 * Neither the name of the <organization> nor the
 names of its contributors may be used to endorse or promote products
 derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stddef.h>
#include <string.h>

#include "ts.h"
#include "libts_common.h"
#include "log.h"

// FIXME: when reading, we read in place; when writing we write from allocated memory
// figure out memory management -- either copy everything, or use flags
// until done, we can read or write, not both.
// idea: have an "expand" flag, if off -- everything is in place, on -- everything is nicely copied and allocated

volatile int tslib_errno = 0;

ts_packet_t *ts_new() {
	ts_packet_t *ts = calloc(1, sizeof(ts_packet_t));
	return ts;
}

void ts_free(ts_packet_t *ts) {
	if (ts == NULL)
		return;
	if (ts->payload.bytes != NULL)
		free(ts->payload.bytes);
	if (ts->adaptation_field.private_data_bytes.bytes != NULL)
		free(ts->adaptation_field.private_data_bytes.bytes);
	free(ts);
}

int ts_read_header(ts_header_t *tsh, bs_t *b) {
	if (tsh == NULL)
		return 0;

	int start_pos = bs_pos(b);

	uint8_t sync_byte = bs_read_u8(b);
	if (sync_byte != TS_SYNC_BYTE) {
		LOG_ERROR_ARGS("Got 0x%02X instead of expected sync byte 0x%02X", sync_byte, TS_SYNC_BYTE);
		SAFE_REPORT_TS_ERR(-10);
		return 0;
	}

	tsh->transport_error_indicator = bs_read_u1(b);
	if (tsh->transport_error_indicator) {
		LOG_ERROR("At least one uncorrectable bit error exists in this TS packet");
		SAFE_REPORT_TS_ERR(-11);
		return 0;
	}

	tsh->payload_unit_start_indicator = bs_read_u1(b);
	tsh->transport_priority = bs_read_u1(b);
	tsh->PID = bs_read_u(b, 13);

	tsh->transport_scrambling_control = bs_read_u(b, 2);
	tsh->adaptation_field_control = bs_read_u(b, 2);
	tsh->continuity_counter = bs_read_u(b, 4);

	return (bs_pos(b) - start_pos);
}

int ts_read_adaptation_field(ts_adaptation_field_t* af, bs_t* b) {
	af->adaptation_field_length = bs_read_u8(b);
	int start_pos = bs_pos(b);

	if (af->adaptation_field_length > 0) {
		af->discontinuity_indicator = bs_read_u1(b);
		af->random_access_indicator = bs_read_u1(b);
		af->elementary_stream_priority_indicator = bs_read_u1(b);
		af->PCR_flag = bs_read_u1(b);
		af->OPCR_flag = bs_read_u1(b);
		af->splicing_point_flag = bs_read_u1(b);
		af->transport_private_data_flag = bs_read_u1(b);
		af->adaptation_field_extension_flag = bs_read_u1(b);

		if (af->adaptation_field_length > 1) {
			if (af->PCR_flag) {
				af->program_clock_reference_base = bs_read_ull(b, 33);
				bs_skip_u(b, 6);
				af->program_clock_reference_extension = bs_read_u(b, 9);
			}
			if (af->OPCR_flag) {
				af->original_program_clock_reference_base = bs_read_ull(b, 33);
				bs_skip_u(b, 6);
				af->original_program_clock_reference_extension = bs_read_u(b, 9);
			}
			if (af->splicing_point_flag) {
				af->splice_countdown = bs_read_u8(b); //FIXME: it's signed, two's compliment #
			}

			if (af->transport_private_data_flag) {
				af->transport_private_data_length = bs_read_u8(b);

				if (af->transport_private_data_length > 0) {
					af->private_data_bytes.len = af->transport_private_data_length;
					af->private_data_bytes.bytes = malloc(af->private_data_bytes.len);
					bs_read_bytes(b, af->private_data_bytes.bytes, af->transport_private_data_length);
				}
			}

			if (af->adaptation_field_extension_flag) {
				af->adaptation_field_extension_length = bs_read_u8(b);
				int afe_start_pos = bs_pos(b);

				af->ltw_flag = bs_read_u1(b);
				af->piecewise_rate_flag = bs_read_u1(b);
				af->seamless_splice_flag = bs_read_u1(b);
				bs_skip_u(b, 5);

				if (af->ltw_flag) {
					af->ltw_valid_flag = bs_read_u1(b);
					af->ltw_offset = bs_read_u(b, 15);
				}
				if (af->piecewise_rate_flag) {
					bs_skip_u(b, 2);
					af->piecewise_rate = bs_read_u(b, 22);
				}
				if (af->seamless_splice_flag) {
					af->splice_type = bs_read_u(b, 4);
					af->DTS_next_AU = bs_read_90khz_timestamp(b);

				}

				int res_len = af->adaptation_field_extension_length + bs_pos(b) - afe_start_pos;
				if (res_len > 0)
					bs_skip_bytes(b, res_len);

			}
		}

		int stuffing_bytes_len = af->adaptation_field_length - (bs_pos(b) - start_pos);
		bs_skip_bytes(b, stuffing_bytes_len);
	}

	return (1 + bs_pos(b) - start_pos);
}

int ts_read(ts_packet_t *ts, uint8_t *buf, size_t buf_size) {
	if (buf == NULL || buf_size < TS_SIZE || ts == NULL) {
		SAFE_REPORT_TS_ERR(-1);
		return 0;
	}

	bs_t b;
	bs_init(&b, buf, TS_SIZE);
	memset(&(ts->header), 0x00, sizeof(ts_header_t));
	if (!ts_read_header(&(ts->header), &b)) {
		SAFE_REPORT_TS_ERR(-2);
		return 0;
	}

	if (ts->header.adaptation_field_control & TS_ADAPTATION_FIELD) {
		memset(&(ts->adaptation_field), 0x00, sizeof(ts_adaptation_field_t));
		if (!ts_read_adaptation_field(&(ts->adaptation_field), &b)) {
			SAFE_REPORT_TS_ERR(-3);
			return 0;
		}
	}

	if (ts->header.adaptation_field_control & TS_PAYLOAD) {
		ts->payload.len = TS_SIZE - bs_pos(&b);
		ts->payload.bytes = malloc(ts->payload.len);
		bs_read_bytes(&b, ts->payload.bytes, ts->payload.len);
	}

	// FIXME read and interpret pointer field

	return bs_pos(&b);
}

int ts_adaptation_field_extension_min_length(ts_adaptation_field_t* af) {
	if (!af->adaptation_field_extension_flag)
		return 0;

	int len = 1;
	if (af->ltw_flag)
		len += 2;
	if (af->piecewise_rate_flag)
		len += 3;
	if (af->seamless_splice_flag)
		len += 5;

	return len;
}

int ts_adaptation_field_min_length(ts_adaptation_field_t* af) {
	int len = 0;

	if (af->PCR_flag)
		len += 6;

	if (af->PCR_flag)
		len += 6;

	if (af->splicing_point_flag)
		len++;

	if (af->transport_private_data_flag)
		len += af->transport_private_data_length;

	if (af->adaptation_field_extension_flag)
		len += ts_adaptation_field_extension_min_length(af) + 1; // adaptation_field_extension_length

	// if none of the bits in byte 1 are set, we don't need byte 1
	if (len == 0)
		len = !!(af->discontinuity_indicator || af->random_access_indicator || af->elementary_stream_priority_indicator);

	return len;
}

int ts_write_adaptation_field(ts_adaptation_field_t* af, bs_t* b) {

	int af_min_len = ts_adaptation_field_min_length(af);

	if (af->adaptation_field_length < af_min_len)
		af->adaptation_field_length = af_min_len;

	bs_write_u8(b, af->adaptation_field_length);
	int start_pos = bs_pos(b);
	if (af->adaptation_field_length > 0) {
		bs_write_u1(b, af->discontinuity_indicator);
		bs_write_u1(b, af->random_access_indicator);
		bs_write_u1(b, af->elementary_stream_priority_indicator);
		bs_write_u1(b, af->PCR_flag);
		bs_write_u1(b, af->OPCR_flag);
		bs_write_u1(b, af->splicing_point_flag);
		bs_write_u1(b, af->transport_private_data_flag);
		bs_write_u1(b, af->adaptation_field_extension_flag);

		if (af->PCR_flag) {
			bs_write_ull(b, 33, af->program_clock_reference_base);
			bs_write_u(b, 6, 0xFF);
			bs_write_u(b, 9, af->program_clock_reference_extension);
		}
		if (af->OPCR_flag) {
			bs_write_ull(b, 33, af->original_program_clock_reference_base);
			bs_write_u(b, 6, 0xFF);
			bs_write_u(b, 9, af->original_program_clock_reference_extension);
		}
		if (af->splicing_point_flag) {
			bs_write_u8(b, af->splice_countdown); // fixme: it's actually signed!
		}
		if (af->transport_private_data_flag) {
			bs_write_u8(b, af->transport_private_data_length);
			bs_write_bytes(b, af->private_data_bytes.bytes, af->transport_private_data_length);
		}

		if (af->adaptation_field_extension_flag) {
			af->adaptation_field_extension_length = ts_adaptation_field_extension_min_length(af);

			bs_write_u8(b, af->adaptation_field_extension_length);

			bs_write_u1(b, af->ltw_flag);
			bs_write_u1(b, af->piecewise_rate_flag);
			bs_write_u1(b, af->seamless_splice_flag);
			bs_write_u(b, 5, 0xFF);

			if (af->ltw_flag) {
				bs_write_u1(b, af->ltw_valid_flag);
				bs_write_u(b, 15, af->ltw_offset);
			}

			if (af->piecewise_rate_flag) {
				bs_write_u(b, 2, 0xFF);
				bs_write_u(b, 22, af->piecewise_rate);
			}

			if (af->seamless_splice_flag)
				bs_write_90khz_timestamp(b, af->DTS_next_AU);
		}

		bs_write_stuffing_bytes(b, 0xFF, af->adaptation_field_length - (bs_pos(b) - start_pos));

	}

	return (bs_pos(b) - start_pos + 1);
}

int ts_write_header(ts_header_t *tsh, bs_t *b) {
	int start_pos = bs_pos(b);
	bs_write_u8(b, TS_SYNC_BYTE);
	bs_write_u1(b, tsh->transport_error_indicator);
	bs_write_u1(b, tsh->payload_unit_start_indicator);
	bs_write_u1(b, tsh->transport_priority);
	bs_write_u(b, 13, tsh->PID);
	bs_write_u(b, 2, tsh->transport_scrambling_control);
	bs_write_u(b, 2, tsh->adaptation_field_control);
	bs_write_u(b, 4, tsh->continuity_counter);
	return (bs_pos(b) - start_pos);
}

int ts_write(ts_packet_t *ts, uint8_t *buf, size_t buf_size) {
	if (buf == NULL || buf_size < TS_SIZE || ts == NULL)
		return 0;

	bs_t b;
	bs_init(&b, buf, TS_SIZE);

	ts_write_header(&ts->header, &b);

	if (ts->header.adaptation_field_control & TS_ADAPTATION_FIELD)
		ts_write_adaptation_field(&ts->adaptation_field, &b);

	if (ts->header.adaptation_field_control & TS_PAYLOAD)
		bs_write_bytes(&b, ts->payload.bytes, TS_SIZE - bs_pos(&b));

	return bs_pos(&b);
}

int ts_print_header(const ts_header_t * const tsh, char *str, size_t str_len) {
	if (tsh == NULL || str == NULL || str_len < 2)
		return 0;
	int bytes = 0;

	bytes += SKIT_LOG_UINT_DBG(str + bytes, 0, tsh->transport_error_indicator, str_len);
	bytes += SKIT_LOG_UINT_DBG(str + bytes, 0, tsh->payload_unit_start_indicator, str_len - bytes);
	bytes += SKIT_LOG_UINT_DBG(str + bytes, 0, tsh->transport_priority, str_len - bytes);
	bytes += SKIT_LOG_UINT_HEX_DBG(str + bytes, 0, tsh->PID, str_len - bytes );

	bytes += SKIT_LOG_UINT_DBG(str + bytes, 0, tsh->transport_scrambling_control, str_len - bytes);
	bytes += SKIT_LOG_UINT_DBG(str + bytes, 0, tsh->adaptation_field_control, str_len - bytes);
	bytes += SKIT_LOG_UINT_DBG(str + bytes, 0, tsh->continuity_counter, str_len - bytes);

	return bytes;
}

int ts_print_adaptation_field(const ts_adaptation_field_t * const af, char *str, size_t str_len) {
	if (af == NULL || str == NULL || str_len < 2)
		return 0;

	int bytes = 0;
	bytes += SKIT_LOG_UINT_DBG( str, 1, af->adaptation_field_length, str_len);

	if (af->adaptation_field_length > 0) {
		bytes += SKIT_LOG_UINT_DBG(str + bytes, 1, af->discontinuity_indicator, str_len - bytes);
		bytes += SKIT_LOG_UINT_DBG(str + bytes, 1, af->random_access_indicator, str_len - bytes);
		bytes += SKIT_LOG_UINT_DBG(str + bytes, 1, af->elementary_stream_priority_indicator, str_len - bytes);
		bytes += SKIT_LOG_UINT_DBG(str + bytes, 1, af->PCR_flag, str_len - bytes);
		bytes += SKIT_LOG_UINT_DBG(str + bytes, 1, af->OPCR_flag, str_len - bytes);
		bytes += SKIT_LOG_UINT_DBG(str + bytes, 1, af->splicing_point_flag, str_len - bytes);
		bytes += SKIT_LOG_UINT_DBG(str + bytes, 1, af->transport_private_data_flag, str_len - bytes);
		bytes += SKIT_LOG_UINT_DBG(str + bytes, 1, af->adaptation_field_extension_flag, str_len - bytes);

		if (af->adaptation_field_length > 1) {
			if (af->PCR_flag) {
				bytes += SKIT_LOG_UINT_DBG(str + bytes, 2, af->program_clock_reference_base, str_len - bytes);
				bytes += SKIT_LOG_UINT_DBG(str + bytes, 2, af->program_clock_reference_extension, str_len - bytes);
			}
			if (af->OPCR_flag) {
				bytes += SKIT_LOG_UINT_DBG(str + bytes, 2, af->original_program_clock_reference_base, str_len - bytes);
				bytes += SKIT_LOG_UINT_DBG(str + bytes, 2, af->original_program_clock_reference_extension, str_len - bytes);
			}
			if (af->splicing_point_flag)
				bytes += SKIT_LOG_UINT_DBG(str + bytes, 2, af->splice_countdown, str_len - bytes);

			if (af->transport_private_data_flag) {
				bytes += SKIT_LOG_UINT_DBG(str + bytes, 2, af->transport_private_data_length, str_len - bytes);
				// TODO print transport_private_data, if any
			}

			if (af->adaptation_field_extension_flag) {
				bytes += SKIT_LOG_UINT_DBG(str + bytes, 2, af->adaptation_field_extension_length, str_len - bytes);
				bytes += SKIT_LOG_UINT_DBG(str + bytes, 2, af->ltw_flag, str_len - bytes);
				bytes += SKIT_LOG_UINT_DBG(str + bytes, 2, af->piecewise_rate_flag, str_len - bytes);
				bytes += SKIT_LOG_UINT_DBG(str + bytes, 2, af->seamless_splice_flag, str_len - bytes);

				if (af->ltw_flag) {
					bytes += SKIT_LOG_UINT_DBG(str + bytes, 3, af->ltw_valid_flag, str_len - bytes);
					bytes += SKIT_LOG_UINT_DBG(str + bytes, 3, af->ltw_offset, str_len - bytes);
				}
				if (af->piecewise_rate_flag) {
					// here go reserved 2 bits
					bytes += SKIT_LOG_UINT_DBG(str + bytes, 3, af->piecewise_rate , str_len - bytes);
				}
				if (af->seamless_splice_flag) {
					bytes += SKIT_LOG_UINT_DBG(str + bytes, 3, af->splice_type, str_len - bytes);
					bytes += SKIT_LOG_UINT_DBG(str + bytes, 3, af->DTS_next_AU, str_len - bytes);
				}
			}
		}
	}

	return bytes;
}

int ts_print(const ts_packet_t *const ts, char *str, size_t str_len) {
	if (ts == NULL || str == NULL || str_len < 2 || tslib_loglevel < TSLIB_LOG_LEVEL_DEBUG)
		return 0;

	int bytes = ts_print_header(&ts->header, str, str_len);

	if (ts->header.adaptation_field_control & TS_ADAPTATION_FIELD)
		bytes += ts_print_adaptation_field(&ts->adaptation_field, str + bytes, str_len - bytes);

	//bytes += SKIT_LOG_UINT_DBG(str + bytes, 0, ts->payload.len, str_len - bytes);
	bytes += SKIT_LOG_UINT64_DBG(str + bytes, "", ts->payload.len, str_len - bytes);

	return bytes;
}
