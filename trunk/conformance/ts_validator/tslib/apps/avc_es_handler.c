/*.
 * h264bitstream - a library for reading and writing H.264 video
 * Copyright (C) 2005-2007 Auroras Entertainment, LLC
 *.
 * Written by Alex Izvorski <aizvorski@gmail.com>
 * Modified by Vlad Zbarsky <zbarsky@cornell.edu>
 *.
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *.
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *.
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

/*
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include <string.h>
#include <errno.h>

// TODO wrap platform-specific stuff w/ ifdef's
#include <linux/limits.h>
*/

#include "common.h"
#include "libts_common.h"

#include "avc_es_handler.h"
#include "pes.h"
#include "log.h"

demux_pid_handler_t *avc_es_handler_new(mpeg2ts_stream_t *m2s, mpeg2ts_program_t *m2p, uint32_t elementary_PID,
	segment_type seg_type, FILE *es_outfile) {
	demux_pid_handler_t *handler = calloc(1, sizeof(demux_pid_handler_t));
	avc_es_data_t *avc_es_data = avc_es_data_new();
	if (!handler || !avc_es_data) {
		SAFE_REPORT_TS_ERR(-1);
		goto out_of_mem_wrap_up;
	}

	avc_es_data->m2s 			= m2s;
	avc_es_data->m2p 			= m2p;
	avc_es_data->elementary_PID	= elementary_PID;

	avc_es_data->seg_type 		= seg_type;

	avc_es_data->es_outfile = es_outfile;

	handler->process_ts_packet	= avc_es_process_ts_packet;
	handler->arg_destructor		= avc_es_data_free;
	handler->arg				= avc_es_data;

	return handler;
out_of_mem_wrap_up:
	if (avc_es_data)
		avc_es_data_free(avc_es_data);
	if (handler)
		free(handler);
	return NULL;
}

avc_es_data_t *avc_es_data_new() {
	avc_es_data_t *avc_es_data = calloc(1, sizeof(avc_es_data_t));
	if (!avc_es_data)
		return NULL;
	avc_es_data->buf = malloc(AVC_ES_INIT_BUF_LEN);
	if (avc_es_data->buf)
		avc_es_data->buf_len = AVC_ES_INIT_BUF_LEN;
	else {
		avc_es_data_free(avc_es_data);
		return NULL;
	}
	return avc_es_data;
}

int avc_es_data_free(avc_es_data_t *avc_es_data) {
	if (!avc_es_data)
		return 0;
	if (avc_es_data->buf)
		free(avc_es_data->buf);
	if (avc_es_data->prev_nal) {
		h264_free(avc_es_data->prev_nal);
		avc_es_data->prev_nal = NULL;
	}

	int ret = 0;
	// FIXME support scrambled streams
	if (tslib_errno == 0 && !avc_es_data->scrambled_stream &&
		(avc_es_data->pes_bytes_read > 0 || avc_es_data->pes_bytes_read < avc_es_data->pes_pack_len)) {
		if (!avc_es_data->pes_pack_unbounded) {
			LOG_ERROR_ARGS("Last PES packet in the elementary stream was a bounded one, and it wasn't completely "
				"collected (PES packet len=%d, remaining=%d)", avc_es_data->pes_pack_len,
				avc_es_data->pes_pack_len - avc_es_data->pes_bytes_read);
			ret = -91;
			SAFE_REPORT_TS_ERR(ret);
		} else {
			LOG_WARN_ARGS("Last PES packet in the elementary stream was an unbounded one, and it's not clear whether"
				" it was completely collected (%d PES bytes read)", avc_es_data->pes_bytes_read);
		}
	}

	if (avc_es_data->es_outfile) {
		fclose(avc_es_data->es_outfile);
		avc_es_data->es_outfile = NULL;
	}

	free(avc_es_data);
	return ret;
}

extern int saw_program_pcr;

static char *NAL_UNIT_NAMES[] = { // types 0 - 13
	"Unspecified", 									// 0
	"Coded slice of a non-IDR picture",				// 1
	"Coded slice data partition A",					// 2
	"Coded slice data partition B",					// 3
	"Coded slice data partition C",					// 4
	"Coded slice of an IDR picture",				// 5
	"Supplemental enhancement information (SEI)",	// 6
	"Sequence parameter set",						// 7
	"Picture parameter set",						// 8
	"Access unit delimiter",						// 9
	"End of sequence",								// 10
	"End of stream",								// 11
	"Filler data",									// 12
	"Sequence parameter set extension",				// 13
	"Prefix NAL unit",								// 14
	"Subset sequence parameter set",				// 15
};

// type 19
static char *NAL_UNIT_NAME_CODED_SLICE_AUX = "Coded slice of an auxiliary coded picture without partitioning";

// default
static char *NAL_UNIT_NAME_GENERIC = "Reserved/Unspecified/Unknown";

static char *nal_desc(h264_stream_t *h264_stream) {
	int nal_unit_type = h264_stream->nal->nal_unit_type;
	if (nal_unit_type < ARRAYSIZE(NAL_UNIT_NAMES))
		return NAL_UNIT_NAMES[nal_unit_type];
	else if (nal_unit_type == NAL_UNIT_TYPE_CODED_SLICE_AUX)
		return NAL_UNIT_NAME_CODED_SLICE_AUX;
	else
		return NAL_UNIT_NAME_GENERIC;
}

static int parse_complete_PES_packet(avc_es_data_t *avc_es_data) {
	uint8_t *curr_ptr;
	int sz, nal_start, nal_end, nal_len, nal_ret, ret = -1;
	uint32_t processed_access_units = 0;
	h264_stream_t *parsed_nal_unit = NULL;

	for (curr_ptr = avc_es_data->buf, sz = avc_es_data->pes_pack_len; sz > 0; curr_ptr += nal_len, sz -= nal_end) {
		if (parsed_nal_unit == NULL && NULL == (parsed_nal_unit = h264_new())) {
			SAFE_REPORT_TS_ERR(-1);
			goto pcpp_wrap_up;
		}

		nal_ret = find_nal_unit(curr_ptr, sz, &nal_start, &nal_end);
		if (nal_ret < 0) {
			if (nal_start > 0 && nal_start < sz && nal_end == sz) {
				LOG_DEBUG("Checking for special case, where NAL might end exactly at the boundary of the buffer");
			} else {
				LOG_ERROR("Error while looking for NAL unit");
				ret = -83;
				SAFE_REPORT_TS_ERR(ret);
				goto pcpp_wrap_up;
			}
		} else if (nal_ret == 0)
			break; // no more NALs in this buffer

		nal_len = nal_end - nal_start;
		curr_ptr += nal_start;

		nal_ret = read_nal_unit(parsed_nal_unit, curr_ptr, nal_len);
		if (nal_ret != nal_len) {
			if (nal_ret == 0) {
				LOG_DEBUG_ARGS("Parsing of NAL unit type %d (%s) not supported yet, ignoring NAL unit #%d",
					parsed_nal_unit->nal->nal_unit_type, nal_desc(parsed_nal_unit), processed_access_units);
				goto pcpp_loop_end; // continue loop
			} else { // error
				LOG_ERROR_ARGS("Error reading NAL unit #%d of type %d (%s): ret=%d",
					processed_access_units, parsed_nal_unit->nal->nal_unit_type, nal_desc(parsed_nal_unit), nal_ret);
				ret = -81;
				SAFE_REPORT_TS_ERR(ret);
				goto pcpp_wrap_up;
			}
		}

		if (tslib_loglevel >= TSLIB_LOG_LEVEL_DEBUG) {
//			long long int stream_nal_off = (long long int) (avc_es_data->avc_es_offset + nal_start);
//			LOG_DEBUG_ARGS("Found NAL at offset %lld (0x%04llX), size %d (0x%04X), start %02d and end %02d", stream_nal_off, stream_nal_off, nal_len, nal_len, nal_start, nal_end);
			uint8_t *debug_nal_ptr = curr_ptr - 4;
			uint32_t debug_nal_print_len = (nal_len + 4 >= 16 ? 16: nal_len + 4);
			printf("DEBUG: NAL header ");
			for (int i = 0; i < debug_nal_print_len; i++) {
				printf("%02X ", debug_nal_ptr[i]);
				if ((i & 3) == 3) {
					printf("\n");
					if (i < (debug_nal_print_len - 1)) printf("DEBUG: NAL header ");
				}
			}
			if (debug_nal_print_len & 3)
				printf("\n");
//			debug_bytes(curr_ptr - 4, nal_len + 4 >= 16 ? 16: nal_len + 4);
//			debug_nal(parsed_nal_unit, parsed_nal_unit->nal);
		}
		LOG_DEBUG_ARGS("Processed NAL unit #%d of type %d (%s), with len %d, start %02d, end %02d from %sbounded "
			"PES packet (len %d)", processed_access_units, parsed_nal_unit->nal->nal_unit_type,
			nal_desc(parsed_nal_unit), nal_len, nal_start, nal_end, (avc_es_data->pes_pack_unbounded ? "un" : ""),
			avc_es_data->pes_pack_len);

		// validation checks

		// if this segment is of SAP type 1/2, validate according to 23009-1 sections 7.4 and 8.6-8.7
		if (avc_es_data->seg_type == SAP12_SEGMENT) switch (avc_es_data->sap12_req_stage) {
		case SAW_IDR_SLICE:
			break; // already done
		case SAW_NEITHER:
			if (parsed_nal_unit->nal->nal_unit_type == NAL_UNIT_TYPE_AUD) switch (parsed_nal_unit->aud->primary_pic_type) {
			case AUD_PRIMARY_PIC_TYPE_I:
			case AUD_PRIMARY_PIC_TYPE_IP:
			case AUD_PRIMARY_PIC_TYPE_IPB:
			case AUD_PRIMARY_PIC_TYPE_ISI:
			case AUD_PRIMARY_PIC_TYPE_ISIPSP:
			case AUD_PRIMARY_PIC_TYPE_ISIPSPB:
				avc_es_data->sap12_req_stage = SAW_1ST_AUD;
				break;
			default:
				LOG_ERROR_ARGS("First AUD indicates a primary picture type %d, instead of one of I picture types",
					parsed_nal_unit->aud->primary_pic_type);
				ret = -86;
				SAFE_REPORT_TS_ERR(ret);
				goto pcpp_wrap_up;
			}
			if (avc_es_data->sap12_req_stage == SAW_1ST_AUD) // always set, if this NAL was an AUD indicating a [potential] I picture slice
				break; // else don't break, fall thru in case we'll see a picture slice w/o AUD
		case SAW_1ST_AUD:
			switch (parsed_nal_unit->nal->nal_unit_type) {
			case NAL_UNIT_TYPE_CODED_SLICE_IDR:
				// TODO also verify that the IDR is the top left corner one
				avc_es_data->sap12_req_stage = SAW_IDR_SLICE;
				break;
			case NAL_UNIT_TYPE_AUD: // if we see 2nd AUD before 1st picture slice, that's no good
				LOG_ERROR("Encountered 2nd AUD before 1st picture slice");
				ret = -86;
				SAFE_REPORT_TS_ERR(ret);
				goto pcpp_wrap_up;
			case NAL_UNIT_TYPE_CODED_SLICE_NON_IDR:
			case NAL_UNIT_TYPE_CODED_SLICE_AUX:
				LOG_ERROR_ARGS("Encountered a non-IDR picture slice of type %d (%s)",
					parsed_nal_unit->nal->nal_unit_type, nal_desc(parsed_nal_unit));
				ret = -86;
				SAFE_REPORT_TS_ERR(ret);
				goto pcpp_wrap_up;
			default: // skip
				break;
			}
		}

		// must encounter PCR information before the first IDR frame
		if (!avc_es_data->saw_idr && parsed_nal_unit->nal->nal_unit_type == NAL_UNIT_TYPE_CODED_SLICE_IDR) {
			if (!saw_program_pcr) {
				LOG_ERROR("No PCR before the first coded slice of an IDR picture");
				ret = -87;
				SAFE_REPORT_TS_ERR(ret);
				goto pcpp_wrap_up;
			}
			avc_es_data->saw_idr = 1;
		}

		// save as prev NAL
		if (avc_es_data->prev_nal)
			h264_free(avc_es_data->prev_nal);
		avc_es_data->prev_nal = parsed_nal_unit;
		parsed_nal_unit = NULL;

pcpp_loop_end:
		if (parsed_nal_unit) {
			h264_free(parsed_nal_unit);
			parsed_nal_unit = NULL;
		}
		avc_es_data->avc_es_offset += nal_end;
		processed_access_units++;
	}

	if (processed_access_units == 0) {
		LOG_ERROR_ARGS("Could not find any NAL units in %sbounded PES packet of length %d",
			(avc_es_data->pes_pack_unbounded ? "un" : ""), avc_es_data->pes_pack_len);
		ret = -89;
		SAFE_REPORT_TS_ERR(ret);
		goto pcpp_wrap_up;
	} else {
		LOG_DEBUG_ARGS("Processed %d access units from %sbounded PES packet of length %d", processed_access_units,
			(avc_es_data->pes_pack_unbounded ? "un" : ""), avc_es_data->pes_pack_len);
	}

	ret = 0;
pcpp_wrap_up:
	return ret;
}

int avc_es_process_ts_packet(ts_packet_t *ts, elementary_stream_info_t *es_info, avc_es_data_t *avc_es_data) {
	uint32_t bytes2copy, pes_bytes_left, buf_space_left;
	buf_t *payload = NULL;
	pes_packet_t *new_pes;
	int ret = -1, pes_packet_bytes_read;

	if (avc_es_data->pes_bytes_read > avc_es_data->buf_len) { // not expecting to be here
		LOG_ERROR_ARGS("Internal error: pes_bytes_read (%d) larger than buf_len (%d)", avc_es_data->pes_bytes_read,
			avc_es_data->buf_len);
		ret = -100;
		goto WRAP_UP;
	}

	if (avc_es_data->scrambled_stream || avc_es_data->unsupported_stream)
		return 0; // TODO implement parsing of scrambled streams

	if (ts->header.transport_scrambling_control != 0) {
		LOG_WARN("Scrambled elementary streams not supported yet");
		avc_es_data->scrambled_stream = TRUE;
		return 0;
	}

	// check that we have TS payload
	if (!(ts->header.adaptation_field_control & TS_PAYLOAD) || ts->payload.len == 0)
		return 0; // no TS payload

	// first off, check whether we reached the end of the unbounded PES packet
	if (ts->header.payload_unit_start_indicator && avc_es_data->pes_pack_unbounded) {
		// if we were collecting unbounded packet, and are upon a new one, finalize and parse the unbounded one
		avc_es_data->pes_pack_len = avc_es_data->pes_bytes_read;
		if (0 != (ret = parse_complete_PES_packet(avc_es_data))) {
			LOG_ERROR("Error parsing complete unbounded PES packet");
			SAFE_REPORT_TS_ERR(ret);
			goto WRAP_UP;
		}
		LOG_DEBUG_ARGS("Finalized and parsed complete unbounded PES packet of length %d", avc_es_data->pes_pack_len);

		avc_es_data->pes_pack_unbounded = 0;
		avc_es_data->pes_pack_len = 0;
		avc_es_data->pes_bytes_read = 0;
	}

	// now check whether we're in the process of collecting an existing PES packet, or not
	bytes2copy = 0;
	new_pes = NULL;
	if (!avc_es_data->pes_pack_unbounded && avc_es_data->pes_pack_len == 0) { // expecting a new PES packet
		if (avc_es_data->pes_bytes_read > 0) { // not expecting to be here
			LOG_ERROR_ARGS("Internal error: pes_bytes_read expected to be 0 instead of %d", avc_es_data->pes_bytes_read);
			ret = -100;
			goto WRAP_UP;
		}

		if (!ts->header.payload_unit_start_indicator) { // expecting a payload unit start indicator set
			LOG_ERROR("Going to parse PES packet header, so expecting payload_unit_start_indicator set");
			ret = -92;
			goto WRAP_UP;
		}

		// try and parse PES packet (expecting a valid PES header)
		new_pes = pes_new(ts->payload.len);
		pes_packet_bytes_read = pes_read(new_pes, ts->payload.bytes, ts->payload.len);
		if (pes_packet_bytes_read > 0) {
			if (tslib_loglevel >= TSLIB_LOG_LEVEL_INFO) {
				char text_buf[0x10000];
				pes_print(new_pes, text_buf, sizeof(text_buf));
			}
		} else { // currently starting a new PES packet, so not supposed to be here
			LOG_ERROR("Error parsing PES packet header");
			ret = -88;
			goto WRAP_UP;
		}

		if (new_pes->header.PES_scrambling_control != 0) {
			LOG_WARN("Scrambled PES packets not supported yet");
			avc_es_data->scrambled_stream = TRUE;
			ret = 0;
			goto WRAP_UP;
		}

		if (!HAS_PES_HEADER(new_pes->header.stream_id)) {
			LOG_WARN_ARGS("PES stream type 0x%08X (%s) not supported, ignoring", new_pes->header.stream_id,
				stream_desc(new_pes->header.stream_id));
			avc_es_data->unsupported_stream = TRUE;
			ret = 0;
			goto WRAP_UP;
		}

		if (new_pes->header.PTS_DTS_flags & PES_PTS_FLAG) { // no duplicate PTS values allowed
			if (avc_es_data->prev_pes_pts != 0 && avc_es_data->prev_pes_pts == new_pes->header.PTS) {
				LOG_ERROR_ARGS("Duplicate PTS timestamp (%lld) in PES packets", (long long int) new_pes->header.PTS);
				ret = -85;
				SAFE_REPORT_TS_ERR(ret);
				goto WRAP_UP;
			}
			avc_es_data->prev_pes_pts = new_pes->header.PTS;
		} else {
			LOG_ERROR("No DTS in PES packet");
			ret = -87;
			SAFE_REPORT_TS_ERR(ret);
			goto WRAP_UP;
		}

		// parse for bounded, set avc_es_data vars
		if (new_pes->header.PES_packet_length > 0) {
			uint32_t pes_header_len = pes_packet_bytes_read - new_pes->payload.len;
			avc_es_data->pes_pack_len = new_pes->header.PES_packet_length - (pes_header_len - 6); // 6 bytes for PES header fields, up to and including PES_packet_length
			avc_es_data->pes_pack_unbounded = 0;
		} else {
			avc_es_data->pes_pack_len = 0;
			avc_es_data->pes_pack_unbounded = 1;
		}
		avc_es_data->pes_bytes_read = 0;

		if (avc_es_data->pes_pack_unbounded) {
			LOG_DEBUG("Starting new unbounded PES packet");
		} else {
			LOG_DEBUG_ARGS("Starting new PES packet of length %d bytes", avc_es_data->pes_pack_len);
		}
	}

	// sanity check: not expecting payload unit start indicator, if we're collecting existing PES packet
	if (new_pes == NULL && ts->header.payload_unit_start_indicator) {
		if (avc_es_data->pes_pack_unbounded) {
			LOG_ERROR("Assertion failed: not expecting to be here");
			ret = -1;
			SAFE_REPORT_TS_ERR(ret);
			goto WRAP_UP;
		}
		LOG_ERROR_ARGS("We're collecting an existing bounded PES packet (%d bytes accumulated of %d), "
			"not expecting start of next PES packet (payload_unit_start_indicator=1)", avc_es_data->pes_bytes_read, avc_es_data->pes_pack_len);
		ret = -88;
		SAFE_REPORT_TS_ERR(ret);
		goto WRAP_UP;
	}

	// now set up from where to copy, and how much
	if (new_pes != NULL) 	// new PES packet
		payload = &new_pes->payload;
	else				// continuing existing PES packet
		payload = &ts->payload;
	if (avc_es_data->pes_pack_unbounded)
		bytes2copy = payload->len;
	else {
		pes_bytes_left = avc_es_data->pes_pack_len - avc_es_data->pes_bytes_read;
		bytes2copy = (pes_bytes_left <= payload->len ? pes_bytes_left : payload->len);
	}

	// finally copy the payload
	buf_space_left = avc_es_data->buf_len - avc_es_data->pes_bytes_read;
	if (bytes2copy > 0) {
		if (bytes2copy > buf_space_left) {
			uint32_t new_buf_len = avc_es_data->buf_len + (bytes2copy - buf_space_left);
			avc_es_data->buf = realloc(avc_es_data->buf, new_buf_len);
			if (avc_es_data->buf == NULL) {
				ret = -1;
				SAFE_REPORT_TS_ERR(ret);
				goto WRAP_UP;
			}
			avc_es_data->buf_len = new_buf_len;
		}
		if (avc_es_data->es_outfile) {
			if (bytes2copy != fwrite(payload->bytes, 1, bytes2copy, avc_es_data->es_outfile)) {
				LOG_ERROR_ARGS("Error extracting %d bytes of AVC elementary stream (PID 0x%04X) into corresponding file - %s",
					bytes2copy, avc_es_data->elementary_PID, strerror(errno));
				ret = -1;
				SAFE_REPORT_TS_ERR(ret);
				goto WRAP_UP;
			}
			LOG_DEBUG_ARGS("Read %d bytes of AVC elementary stream (PID 0x%04X) into corresponding file",
				bytes2copy, avc_es_data->elementary_PID);
		}
		memcpy(avc_es_data->buf + avc_es_data->pes_bytes_read, payload->bytes, bytes2copy);
		avc_es_data->pes_bytes_read += bytes2copy;
		if (avc_es_data->pes_pack_unbounded) {
			LOG_DEBUG_ARGS("Copied %d bytes into PES accumulation buffer (%d bytes accumulated)", bytes2copy, avc_es_data->pes_bytes_read);
		} else {
			LOG_DEBUG_ARGS("Copied %d bytes into PES accumulation buffer (%d bytes accumulated of %d)", bytes2copy, avc_es_data->pes_bytes_read, avc_es_data->pes_pack_len);
		}
	}

	// alright, now we copied the payload data from the packet (TS or PES) into accumulation buffer

	if (avc_es_data->pes_pack_unbounded) { // if collecting an unbounded packet, there's nothing more to do here
		ret = 0;
		goto WRAP_UP;
	}

	// check whether we have a complete bounded PES packet in the buffer
	pes_bytes_left = avc_es_data->pes_pack_len - avc_es_data->pes_bytes_read;
	if (pes_bytes_left > 0) { // more TS packets to read in order to assemble this bounded PES packet
		ret = 0;
		goto WRAP_UP;
	}

	// we have a complete bounded PES packet, parse it for AVC access units
	if (0 != (ret = parse_complete_PES_packet(avc_es_data))) {
		LOG_ERROR("Error parsing complete bounded PES packet");
		SAFE_REPORT_TS_ERR(ret);
		goto WRAP_UP;
	}

	// ok, we just successfully parsed a complete bounded PES packet, let's clean the vars
	avc_es_data->pes_pack_len = 0;
	avc_es_data->pes_bytes_read = 0;
	avc_es_data->pes_pack_unbounded = 0; // should be set earlier, but just in case

	ret = 0;
WRAP_UP:
	if (new_pes) {
		pes_free(new_pes);
		new_pes = NULL;
	}

	if (ret != 0)
		SAFE_REPORT_TS_ERR(ret);

	return ret;
}
