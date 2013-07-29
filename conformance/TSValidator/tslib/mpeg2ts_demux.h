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

#ifndef _MPEG2TS_DEMUX_H_
#define _MPEG2TS_DEMUX_H_

#include <stdint.h>

#include "bs.h"
#include "common.h"
#include "ts.h"
#include "psi.h"
#include "descriptors.h"
#include "vqarray.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*ts_pid_processor_t)(ts_packet_t*, elementary_stream_info_t*, void*);
typedef int (*ts_pid_arg_destructor_t)(void*);

typedef int (*pes_callback_t)(void*);

typedef struct {
	uint32_t PID;
	uint32_t program_number;

	vqarray_t *pids;

	struct {
		int64_t first_pcr;
		uint32_t num_rollovers;
		int64_t pcr[2];
		double pcr_rate;
	} pcr_info;

	program_map_section_t *pmt;

} mpeg2ts_program_t;

typedef struct {
	program_association_section_t *pat;
	vqarray_t *programs;

} mpeg2ts_stream_t;

typedef struct {
	buf_t buffer;
	vqarray_t *ts_queue;
	pes_callback_t payload_callback;
	mpeg2ts_stream_t *m2s;
	uint32_t syntax;
} payload_reader_t;

typedef int (*payload_reader_callback_t)(payload_reader_t*, vqarray_t*);

typedef struct {
	void *arg;
	ts_pid_arg_destructor_t arg_destructor;
	ts_pid_processor_t process_ts_packet;
} demux_pid_handler_t;

typedef struct {
	void *arg;
	// TODO impl
} mux_pid_handler_t;

typedef struct {
	demux_pid_handler_t *demux_handler;
	// TODO: mux_pid_handler_t*
	elementary_stream_info_t *es_info;
} pid_info_t;

// FIXME: implement and add CAT, etc.

mpeg2ts_stream_t *mpeg2ts_stream_new();
void mpeg2ts_stream_free(mpeg2ts_stream_t *m2s);
int mpeg2ts_stream_read_ts_packet(mpeg2ts_stream_t *m2s, ts_packet_t *ts);

mpeg2ts_program_t *mpeg2ts_program_new();
void mpeg2ts_program_free(mpeg2ts_program_t *m2p);

int mpeg2ts_program_register_pid_processor(mpeg2ts_program_t *m2p, uint32_t PID, demux_pid_handler_t *handler);
int mpeg2ts_program_unregister_pid_processor(mpeg2ts_program_t *m2p, uint32_t PID);
//int mpeg2ts_program_read_ts_packet(mpeg2ts_program_t *m2p, ts_packet_t *ts);

#ifdef __cplusplus
}
#endif

#endif // _MPEG2TS_DEMUX_H_
