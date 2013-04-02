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
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR COmpeg transport streamNSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _AVC_ES_HANDLER_H_
#define _AVC_ES_HANDLER_H_

#include "h264_stream.h"

#include "mpeg2ts_demux.h"

#define AVC_ES_INIT_BUF_LEN		65536

typedef enum { INITIALIZATION_SEGMENT = 0, SAP12_SEGMENT, GENERAL_SEGMENT } segment_type;

typedef enum { SAW_NEITHER = 0, SAW_1ST_AUD, SAW_IDR_SLICE } SAP12_requirement_stage;

typedef struct {
	// metadata so far
	mpeg2ts_stream_t *m2s;
	mpeg2ts_program_t *m2p;
	uint32_t elementary_PID;

	// our internal data
	segment_type seg_type;
	uint8_t scrambled_stream;	// whether the elementary stream is scrambled (at TS or PES level)
	uint8_t unsupported_stream;	// whether the elementary stream is of unsupported type

	uint8_t *buf;				// internally buffering the next PES packet
	uint32_t buf_len;			// currently alloc'd length of the buf
	uint32_t pes_bytes_read;	// number of PES payload bytes collected to date (also offset within the buf)
								// set to 0 after collecting and parsing a full PES packet
	uint32_t pes_pack_len;		// length of the PES packet we're currently collecting
								// set to 0 after collecting and parsing a full PES packet
	uint32_t pes_pack_unbounded;// whether this PES packet has unbounded length

	uint64_t prev_pes_pts;						// PTS value of previous PES packet
	SAP12_requirement_stage sap12_req_stage;	// as it says
	uint32_t saw_idr;							// whether we saw at least one IDR picture slice so far

	h264_stream_t *prev_nal;	// we store the previously parsed NAL for analysis
	uint64_t avc_es_offset;		// offset within elementary stream

	FILE *es_outfile;			// if non-null, we'll extract the elementary stream to it
} avc_es_data_t;

demux_pid_handler_t *avc_es_handler_new(mpeg2ts_stream_t *m2s, mpeg2ts_program_t *m2p, uint32_t elementary_PID,
	segment_type seg_type, FILE *es_outfile);

avc_es_data_t *avc_es_data_new();
int avc_es_data_free(avc_es_data_t *avc_es_data);

int avc_es_process_ts_packet(ts_packet_t *ts, elementary_stream_info_t *es_info, avc_es_data_t *avc_es_data);

#endif // _AVC_ES_HANDLER_H_
