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

#ifndef _TSLIB_TS_H_
#define _TSLIB_TS_H_        

#include <stdint.h>

#include "bs.h"
#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TS_SIZE         			188
#define TS_HEADER_SIZE    			4
#define TS_SYNC_BYTE   				0x47

#define TS_PAYLOAD           		0x01
#define TS_ADAPTATION_FIELD  		0x02

#ifndef TRUE
#define	TRUE	1
#endif
#ifndef FALSE
#define FALSE	0
#endif

typedef struct {
	uint32_t transport_error_indicator;
	uint32_t payload_unit_start_indicator;
	uint32_t transport_priority;
	uint32_t PID;

	uint32_t transport_scrambling_control;
	uint32_t adaptation_field_control;
	uint32_t continuity_counter;
} ts_header_t;

typedef struct {
	uint32_t adaptation_field_length;

	uint32_t discontinuity_indicator;
	uint32_t random_access_indicator;
	uint32_t elementary_stream_priority_indicator;
	uint32_t PCR_flag;
	uint32_t OPCR_flag;
	uint32_t splicing_point_flag;
	uint32_t transport_private_data_flag;
	uint32_t adaptation_field_extension_flag;

	uint64_t program_clock_reference_base;
	uint32_t program_clock_reference_extension;
	uint64_t original_program_clock_reference_base;
	uint32_t original_program_clock_reference_extension;

	uint32_t splice_countdown;
	uint32_t transport_private_data_length;
	uint32_t adaptation_field_extension_length;
	uint32_t ltw_flag;
	uint32_t piecewise_rate_flag;
	uint32_t seamless_splice_flag;
	uint32_t ltw_valid_flag;
	uint32_t ltw_offset;
	uint32_t piecewise_rate;
	uint32_t splice_type;
	uint64_t DTS_next_AU;

	buf_t private_data_bytes;

} ts_adaptation_field_t;

typedef struct {
	ts_header_t header;
	ts_adaptation_field_t adaptation_field;
	uint8_t *bytes;		/// bytes of the *complete* TS packet, managed by the caller.

	buf_t payload;		/// start of the payload

	uint8_t *opaque;	/// opaque user-defined pointer. memory is managed by the user

} ts_packet_t;

ts_packet_t *ts_new();
void ts_free(ts_packet_t *ts);

int ts_read_header(ts_header_t *tsh, bs_t *b);
int ts_read_adaptation_field(ts_adaptation_field_t *af, bs_t *b);
int ts_read(ts_packet_t *ts, uint8_t *buf, size_t buf_size);

int ts_write_adaptation_field(ts_adaptation_field_t *af, bs_t *b);
int ts_write_header(ts_header_t *tsh, bs_t *b);
int ts_write(ts_packet_t *ts, uint8_t *buf, size_t buf_size);

int ts_print_adaptation_field(const ts_adaptation_field_t *const af, char* str, size_t str_len);
int ts_print_header(const ts_header_t *const tsh, char* str, size_t str_len);
int ts_print(const ts_packet_t *const ts, char* str, size_t str_len);

#ifdef __cplusplus
}
#endif

#endif
