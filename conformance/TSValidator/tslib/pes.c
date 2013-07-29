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

// we need these for stdint.h
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS 1
#endif // __STDC_LIMIT_MACROS

// we need these for log.h
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS 1
#endif // __STDC_FORMAT_MACROS

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>

#include "bs.h"
#include "pes.h"
#include "libts_common.h"
#include "log.h"

pes_packet_t *pes_new(size_t payload_size) {
	pes_packet_t *pes = (pes_packet_t*) calloc(1, sizeof(pes_packet_t));
	if ((pes != NULL) && (payload_size > 0)) {
		pes->payload.len = payload_size;
		pes->payload.bytes = (uint8_t *) malloc(payload_size);
	}
	return pes;
}

void pes_free(pes_packet_t *pes) {
	if (pes == NULL)
		return;
	if (pes->payload.bytes != NULL) {
		free(pes->payload.bytes);
		pes->payload.bytes = NULL;
	}
	free(pes);
}

int pes_read(pes_packet_t *pes, uint8_t *buf, size_t len) {
	if (buf == NULL || pes == NULL)
		return 0;

	bs_t b;
	bs_init(&b, buf, len);

	int header_bytes = pes_read_header(&pes->header, &b);
	if (header_bytes > 0) {
		pes->payload.len = len - header_bytes;
		pes->payload.bytes = (uint8_t *) realloc(pes->payload.bytes, pes->payload.len);
		bs_read_bytes(&b, pes->payload.bytes, pes->payload.len);
	}
	return bs_pos(&b);
}

int pes_read_header(pes_header_t *ph, bs_t *b) {
	int PES_packet_start = bs_pos(b);

	// we *really* care about bytes 0..19, as we can cheat and manipulate PTS/DTS this way
	if (bs_bytes_left(b) < 20)
		return 0;

	// bytes 0..2
	uint32_t pes_packet_start_code = bs_read_u24(b);
	if (pes_packet_start_code != PES_PACKET_START_CODE_PREFIX) {
		int actually_read = bs_pos(b) - PES_packet_start;
		b->p -= actually_read; // undo the read
		LOG_WARN_ARGS("PES packet starts with 0x%06X instead of expected start code 0x%06X, skipping it",
			pes_packet_start_code, PES_PACKET_START_CODE_PREFIX);
		return 0; // bail out! something is fishy!
	}

	// bytes 3..5
	ph->stream_id = bs_read_u8(b);
	ph->PES_packet_length = bs_read_u16(b);

	if (HAS_PES_HEADER(ph->stream_id)) {
		// byte 6
		bs_skip_u(b, 2);
		ph->PES_scrambling_control = bs_read_u(b, 2);
		ph->PES_priority = bs_read_u1(b);
		ph->data_alignment_indicator = bs_read_u1(b);
		ph->copyright = bs_read_u1(b);
		ph->original_or_copy = bs_read_u1(b);

		// byte 7
		ph->PTS_DTS_flags = bs_read_u(b, 2);
		ph->ESCR_flag = bs_read_u1(b);
		ph->ES_rate_flag = bs_read_u1(b);
		ph->DSM_trick_mode_flag = bs_read_u1(b);
		ph->additional_copy_info_flag = bs_read_u1(b);
		ph->PES_CRC_flag = bs_read_u1(b);
		ph->PES_extension_flag = bs_read_u1(b);

		// byte 8
		ph->PES_header_data_length = bs_read_u8(b);

		int PES_packet_optional_start = bs_pos(b);

		// byte 9..14
		if (ph->PTS_DTS_flags & PES_PTS_FLAG) {
			bs_skip_u(b, 4);
			ph->PTS = bs_read_90khz_timestamp(b);
		}
		// byte 15..19
		if (ph->PTS_DTS_flags & PES_DTS_FLAG) {
			bs_skip_u(b, 4);
			ph->DTS = bs_read_90khz_timestamp(b);
		}

		if (ph->ESCR_flag) {
			bs_skip_u(b, 2);
			ph->ESCR_base = bs_read_90khz_timestamp(b);
			ph->ESCR_extension = bs_read_u(b, 9);
			bs_skip_u1(b);
		}
		if (ph->ES_rate_flag) {
			bs_skip_u1(b);
			ph->ES_rate = bs_read_u(b, 22);
			bs_skip_u1(b);
		}
		if (ph->DSM_trick_mode_flag) {
			ph->trick_mode_control = bs_read_u(b, 3);
			switch (ph->trick_mode_control) {
			case PES_DSM_TRICK_MODE_CTL_FAST_FORWARD:
			case PES_DSM_TRICK_MODE_CTL_FAST_REVERSE:
				ph->field_id = bs_read_u(b, 2);
				ph->intra_slice_refresh = bs_read_u1(b);
				ph->frequency_truncation = bs_read_u(b, 2);
				break;

			case PES_DSM_TRICK_MODE_CTL_SLOW_MOTION:
			case PES_DSM_TRICK_MODE_CTL_SLOW_REVERSE:
				ph->rep_cntrl = bs_read_u(b, 5);
				break;

			case PES_DSM_TRICK_MODE_CTL_FREEZE_FRAME:
				ph->field_id = bs_read_u(b, 2);
				bs_skip_u(b, 3);
				break;

			default:
				bs_skip_u(b, 5);
				break;
			}
		}
		if (ph->additional_copy_info_flag) {
			bs_skip_u1(b);
			ph->additional_copy_info = bs_read_u(b, 7);
		}
		if (ph->PES_CRC_flag) {
			ph->previous_PES_packet_CRC = bs_read_u16(b);
		}
		if (ph->PES_extension_flag) {
			ph->PES_private_data_flag = bs_read_u1(b);
			ph->pack_header_field_flag = bs_read_u1(b);
			ph->program_packet_sequence_counter_flag = bs_read_u1(b);
			ph->PSTD_buffer_flag = bs_read_u1(b);
			bs_skip_u(b, 3);
			ph->PES_extension_flag_2 = bs_read_u1(b);

			if (ph->PES_private_data_flag)
				bs_read_bytes(b, ph->PES_private_data, 16);

			if (ph->pack_header_field_flag) {
				// whoever discovers the need for pack_header() is welcome to implement it.
				// I haven't.
				ph->pack_field_length = bs_read_u8(b);
				bs_skip_bytes(b, ph->pack_field_length);
			}
			if (ph->program_packet_sequence_counter_flag) {
				bs_skip_u1(b);
				ph->program_packet_sequence_counter = bs_read_u(b, 7);
				bs_skip_u1(b);
				ph->MPEG1_MPEG2_identifier = bs_read_u1(b);
				ph->original_stuff_length = bs_read_u(b, 6);
			}
			if (ph->PSTD_buffer_flag) {
				bs_skip_u(b, 2);
				ph->PSTD_buffer_scale = bs_read_u1(b);
				ph->PSTD_buffer_size = bs_read_u(b, 13);
			}
			if (ph->PES_extension_flag_2) {
				int PES_extension_field_start = bs_pos(b);
				bs_skip_u1(b);
				ph->PES_extension_field_length = bs_read_u(b, 7);
				ph->stream_id_extension_flag = bs_read_u1(b);
				if (!ph->stream_id_extension_flag) {
					ph->stream_id_extension = bs_read_u(b, 7);
				} else {
					bs_skip_u(b, 6);
					ph->tref_extension_flag = bs_read_u1(b);
					if (ph->tref_extension_flag) {
						bs_skip_u(b, 4);
						ph->TREF = bs_read_90khz_timestamp(b);
					}
				}
				int PES_extension_bytes_left = bs_pos(b) - PES_extension_field_start;
				if (PES_extension_bytes_left > 0)
					bs_skip_bytes(b, PES_extension_bytes_left);
			}
		}

		int PES_optional_bytes_read = bs_pos(b) - PES_packet_optional_start;
		int stuffing_bytes_len = ph->PES_header_data_length - PES_optional_bytes_read; // if any
		if (stuffing_bytes_len > 0)
			bs_skip_bytes(b, stuffing_bytes_len);
	}
	return (bs_pos(b) - PES_packet_start);
}

size_t pes_header_trim(pes_header_t *ph, size_t data_len) {

	size_t len = 0;
	size_t opt_len = 0;
	size_t ext_len = 0;

	if (HAS_PES_HEADER(ph->stream_id)) {
		len += 3;

		// we don't want to write DTS if it equals PTS.
		if ((ph->PTS_DTS_flags & PES_DTS_FLAG) && ph->PTS == ph->DTS)
			 ph->PTS_DTS_flags = PES_PTS_FLAG;

		if (ph->PTS_DTS_flags & PES_PTS_FLAG)
			opt_len += 5;
		if (ph->PTS_DTS_flags & PES_DTS_FLAG)
			opt_len += 5;
		if (ph->ESCR_flag)
			opt_len += 6;
		if (ph->ES_rate_flag)
			opt_len += 3;
		if (ph->DSM_trick_mode_flag)
			opt_len++;
		if (ph->additional_copy_info_flag)
			opt_len++;
		if (ph->PES_CRC_flag)
			opt_len += 2;

		if (ph->PES_extension_flag) {
			opt_len += 1;
			if (ph->PES_private_data_flag)
				opt_len += 16;

			// we do not implement pack_header.
			if (ph->program_packet_sequence_counter_flag)
				opt_len += 2;
			if (ph->PSTD_buffer_flag)
				opt_len += 2;

			if (ph->PES_extension_flag_2) {
				opt_len += 1; // extension field length

				ext_len += 1;
				if (ph->tref_extension_flag)
					ext_len += 5;
			}
			opt_len += ext_len;
		}
		len += opt_len;
	}

	// unlimited iff size cannot be expressed in 16 bits
	ph->PES_packet_length = len + data_len < UINT16_MAX ? len + data_len : 0;

	ph->PES_header_data_length = opt_len;
	ph->PES_extension_field_length = ext_len;

	// total real length
	return (len + data_len + 6);
}

int pes_header_write(pes_header_t *ph, bs_t *b) {

	// TODO: add support for PES-level stuffing

	int start_pos = bs_pos(b);
	bs_write_u24(b, PES_PACKET_START_CODE_PREFIX);
	bs_write_u8(b, ph->stream_id);
	bs_write_u16(b, ph->PES_packet_length);

	if (HAS_PES_HEADER(ph->stream_id)) {

		bs_write_u(b, 2, 0x02);
		bs_write_u(b, 2, ph->PES_scrambling_control);
		bs_write_u1(b, ph->PES_priority);
		bs_write_u1(b, ph->data_alignment_indicator);
		bs_write_u1(b, ph->copyright);
		bs_write_u1(b, ph->original_or_copy);

		bs_write_u(b, 2, ph->PTS_DTS_flags);
		bs_write_u1(b, ph->ESCR_flag);
		bs_write_u1(b, ph->ES_rate_flag);
		bs_write_u1(b, ph->DSM_trick_mode_flag);
		bs_write_u1(b, ph->additional_copy_info_flag);
		bs_write_u1(b, ph->PES_CRC_flag);
		bs_write_u1(b, ph->PES_extension_flag);

		bs_write_u8(b, ph->PES_header_data_length);

		if (ph->PTS_DTS_flags & PES_PTS_FLAG) {
			bs_write_u(b, 4, 0x02);
			bs_write_90khz_timestamp(b, ph->PTS);
		}

		if (ph->PTS_DTS_flags & PES_DTS_FLAG) {
			bs_write_u(b, 4, 0x01);
			bs_write_90khz_timestamp(b, ph->DTS);
		}

		if (ph->ESCR_flag) {
			bs_write_reserved(b, 2);
			bs_write_90khz_timestamp(b, ph->ESCR_base);
			bs_write_u(b, 9, ph->ESCR_extension);
			bs_write_marker_bit(b);
		}
		if (ph->ES_rate_flag) {
			bs_write_marker_bit(b);
			bs_write_u(b, 22, ph->ES_rate);
			bs_write_marker_bit(b);
		}
		if (ph->DSM_trick_mode_flag) {
			bs_write_u(b, 3, ph->trick_mode_control);

			switch (ph->trick_mode_control) {
			case PES_DSM_TRICK_MODE_CTL_FAST_FORWARD:
			case PES_DSM_TRICK_MODE_CTL_FAST_REVERSE:
				bs_write_u(b, 2, ph->field_id);
				bs_write_u1(b, ph->intra_slice_refresh);
				bs_write_u(b, 2, ph->frequency_truncation);

				break;

			case PES_DSM_TRICK_MODE_CTL_SLOW_MOTION:
			case PES_DSM_TRICK_MODE_CTL_SLOW_REVERSE:
				bs_write_u(b, 5, ph->rep_cntrl);
				break;

			case PES_DSM_TRICK_MODE_CTL_FREEZE_FRAME:
				bs_write_u(b, 2, ph->field_id);
				bs_write_reserved(b, 3);
				break;

			default:
				bs_write_reserved(b, 5);
				break;
			}
		}

		if (ph->additional_copy_info_flag) {
			bs_write_marker_bit(b);
			bs_write_u(b, 7, ph->additional_copy_info);
		}
		if (ph->PES_CRC_flag) {
			bs_write_u16(b, ph->previous_PES_packet_CRC);
		}
		if (ph->PES_extension_flag) {
			bs_write_u1(b, ph->PES_private_data_flag);
			bs_write_u1(b, 0); // pack_header not supported
			bs_write_u1(b, ph->program_packet_sequence_counter_flag);
			bs_write_u1(b, ph->PSTD_buffer_flag);
			bs_write_reserved(b, 3);
			bs_write_u1(b, ph->PES_extension_flag_2);

			if (ph->PES_private_data_flag) {
				bs_write_bytes(b, ph->PES_private_data, 16);
			}

			if (ph->program_packet_sequence_counter_flag) {
				bs_write_marker_bit(b);
				bs_write_u(b, 7, ph->program_packet_sequence_counter);
				bs_write_marker_bit(b);
				bs_write_u1(b, ph->MPEG1_MPEG2_identifier);
				bs_write_u(b, 6, ph->original_stuff_length);
			}

			if (ph->PSTD_buffer_flag) {
				bs_write_u(b, 2, 0x01);
				bs_write_u1(b, ph->PSTD_buffer_scale);
				bs_write_u(b, 13, ph->PSTD_buffer_size);
			}

			if (ph->PES_extension_flag_2) {
				bs_write_marker_bit(b);
				bs_write_u(b, 7, ph->PES_extension_field_length);
				bs_write_u1(b, ph->stream_id_extension_flag);

				if (!ph->stream_id_extension_flag) {
					bs_write_u(b, 7, ph->stream_id_extension);
				} else {
					bs_write_reserved(b, 6);
					bs_write_u1(b, ph->tref_extension_flag);

					if (ph->tref_extension_flag) {
						bs_write_reserved(b, 4);
						bs_write_90khz_timestamp(b, ph->TREF);
					}
				}
			}
		}
	}
	return (bs_pos(b) - start_pos);
}

int pes_print_header(pes_header_t *pes_header, char *str, size_t str_len) {
	int bytes = 0;
	bytes += SKIT_LOG_UINT32_DBG(str + bytes, "", pes_header->stream_id, str_len);
	bytes += SKIT_LOG_UINT32_DBG(str + bytes, "", pes_header->PES_packet_length, str_len - bytes);

	if (HAS_PES_HEADER(pes_header->stream_id)) {
		bytes += SKIT_LOG_UINT32_DBG(str + bytes, "", pes_header->PES_scrambling_control, str_len - bytes);
		bytes += SKIT_LOG_UINT32_DBG(str + bytes, "", pes_header->PES_priority, str_len - bytes);
		bytes += SKIT_LOG_UINT32_DBG(str + bytes, "", pes_header->data_alignment_indicator, str_len - bytes);
		bytes += SKIT_LOG_UINT32_DBG(str + bytes, "", pes_header->copyright, str_len - bytes);
		bytes += SKIT_LOG_UINT32_DBG(str + bytes, "", pes_header->original_or_copy, str_len - bytes);

		bytes += SKIT_LOG_UINT32_DBG(str + bytes, "", pes_header->PTS_DTS_flags, str_len - bytes);
		bytes += SKIT_LOG_UINT32_DBG(str + bytes, "", pes_header->ESCR_flag , str_len - bytes);
		bytes += SKIT_LOG_UINT32_DBG(str + bytes, "", pes_header->ES_rate_flag , str_len - bytes);
		bytes += SKIT_LOG_UINT32_DBG(str + bytes, "", pes_header->DSM_trick_mode_flag, str_len - bytes);
		bytes += SKIT_LOG_UINT32_DBG(str + bytes, "", pes_header->additional_copy_info_flag, str_len - bytes);
		bytes += SKIT_LOG_UINT32_DBG(str + bytes, "", pes_header->PES_CRC_flag, str_len - bytes);
		bytes += SKIT_LOG_UINT32_DBG(str + bytes, "", pes_header->PES_extension_flag, str_len - bytes);

		// byte 8
		bytes += SKIT_LOG_UINT32_DBG(str + bytes, "", pes_header->PES_header_data_length, str_len - bytes);

		// byte 9..14
		if (pes_header->PTS_DTS_flags & PES_PTS_FLAG)
			bytes += SKIT_LOG_UINT64_DBG(str + bytes, "   ", pes_header->PTS, str_len - bytes);

		// byte 15..19
		if (pes_header->PTS_DTS_flags & PES_DTS_FLAG)
			bytes += SKIT_LOG_UINT64_DBG(str + bytes, "   ", pes_header->DTS, str_len - bytes);

		if (pes_header->ESCR_flag) {
			bytes += SKIT_LOG_UINT64_DBG(str + bytes, "   ", pes_header->ESCR_base, str_len - bytes);
			bytes += SKIT_LOG_UINT32_DBG(str + bytes, "   ", pes_header->ESCR_extension, str_len - bytes);

		}
		if (pes_header->ES_rate_flag)
			bytes += SKIT_LOG_UINT32_DBG(str + bytes, "   ", pes_header->ES_rate, str_len - bytes);

		if (pes_header->DSM_trick_mode_flag) {
			bytes += SKIT_LOG_UINT32_DBG(str + bytes, "   ", pes_header->trick_mode_control, str_len - bytes);
			switch (pes_header->trick_mode_control) {
			case PES_DSM_TRICK_MODE_CTL_FAST_FORWARD:
			case PES_DSM_TRICK_MODE_CTL_FAST_REVERSE:
				bytes += SKIT_LOG_UINT32_DBG(str + bytes, "   ", pes_header->field_id , str_len - bytes)
				;
				bytes += SKIT_LOG_UINT32_DBG(str + bytes, "   ", pes_header->intra_slice_refresh, str_len - bytes)
				;
				bytes += SKIT_LOG_UINT32_DBG(str + bytes, "   ", pes_header->frequency_truncation, str_len - bytes)
				;
				break;

			case PES_DSM_TRICK_MODE_CTL_SLOW_MOTION:
			case PES_DSM_TRICK_MODE_CTL_SLOW_REVERSE:
				bytes += SKIT_LOG_UINT32_DBG(str + bytes, "   ", pes_header->rep_cntrl, str_len - bytes)
				;
				break;

			case PES_DSM_TRICK_MODE_CTL_FREEZE_FRAME:
				bytes += SKIT_LOG_UINT32_DBG(str + bytes, "   ", pes_header->field_id , str_len - bytes)
				;
				break;

			default:

				break;
			}
		}
		if (pes_header->additional_copy_info_flag) {
			bytes += SKIT_LOG_UINT32_DBG(str + bytes, "   ", pes_header->additional_copy_info, str_len - bytes);
		}
		if (pes_header->PES_CRC_flag) {
			bytes += SKIT_LOG_UINT32_DBG(str + bytes, "   ", pes_header->previous_PES_packet_CRC, str_len - bytes);
		}
		if (pes_header->PES_extension_flag) {
			bytes += SKIT_LOG_UINT32_DBG(str + bytes, "   ", pes_header->PES_private_data_flag, str_len - bytes);
			bytes += SKIT_LOG_UINT32_DBG(str + bytes, "   ", pes_header->pack_header_field_flag, str_len - bytes);
			bytes += SKIT_LOG_UINT32_DBG(str + bytes, "   ", pes_header->program_packet_sequence_counter_flag, str_len - bytes);
			bytes += SKIT_LOG_UINT32_DBG(str + bytes, "   ", pes_header->PSTD_buffer_flag, str_len - bytes);
			bytes += SKIT_LOG_UINT32_DBG(str + bytes, "   ", pes_header->PES_extension_flag_2, str_len - bytes);

			// add ph->PES_private_data_flag

			if (pes_header->pack_header_field_flag) {

				bytes += SKIT_LOG_UINT32_DBG(str + bytes, "       ", pes_header->pack_field_length, str_len - bytes);
			}
			if (pes_header->program_packet_sequence_counter_flag) {
				bytes += SKIT_LOG_UINT32_DBG(str + bytes, "       ", pes_header->program_packet_sequence_counter, str_len - bytes);
				bytes += SKIT_LOG_UINT32_DBG(str + bytes, "       ", pes_header->MPEG1_MPEG2_identifier, str_len - bytes);
				bytes += SKIT_LOG_UINT32_DBG(str + bytes, "       ", pes_header->original_stuff_length, str_len - bytes);
			}
			if (pes_header->PSTD_buffer_flag) {

				bytes += SKIT_LOG_UINT32_DBG(str + bytes, "       ", pes_header->PSTD_buffer_scale, str_len - bytes);
				bytes += SKIT_LOG_UINT32_DBG(str + bytes, "       ", pes_header->PSTD_buffer_size, str_len - bytes);
			}
			if (pes_header->PES_extension_flag_2) {
				bytes += SKIT_LOG_UINT32_DBG(str + bytes, "       ", pes_header->PES_extension_field_length, str_len - bytes);
				bytes += SKIT_LOG_UINT32_DBG(str + bytes, "       ", pes_header->stream_id_extension_flag, str_len - bytes);
				if (!pes_header->stream_id_extension_flag) {
					bytes += SKIT_LOG_UINT32_DBG(str + bytes, "           ", pes_header->stream_id_extension, str_len - bytes);
				} else {
					bytes += SKIT_LOG_UINT32_DBG(str + bytes, "           ", pes_header->tref_extension_flag, str_len - bytes);
					if (pes_header->tref_extension_flag) {
						bytes += SKIT_LOG_UINT64_DBG(str + bytes, "           ", pes_header->TREF, str_len - bytes);
					}
				}
			}
		}
	}
	return bytes;
}

int pes_print(pes_packet_t *pes, char *str, size_t str_len) {
	if (tslib_loglevel < TSLIB_LOG_LEVEL_DEBUG)
		return 0;
	int bytes = pes_print_header(&pes->header, str, str_len);
	bytes += SKIT_LOG_UINT64_DBG(str + bytes, "", pes->payload.len, str_len);
	return bytes;
}
