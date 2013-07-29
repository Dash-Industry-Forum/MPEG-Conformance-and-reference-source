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

#ifndef _PSI_H_
#define _PSI_H_

#include <stdint.h>

#include "bs.h"
#include "common.h"
#include "vqarray.h"

// PAT

typedef struct {
	uint32_t program_number;
	uint32_t program_map_PID; // a.k.a. network pid for prog 0
} program_info_t;

typedef struct {
	uint32_t table_id;
	uint32_t section_syntax_indicator;
	uint32_t section_length;
	uint32_t transport_stream_id;
	uint32_t version_number;
	uint32_t current_next_indicator;
	uint32_t section_number;
	uint32_t last_section_number;

	program_info_t *programs;
	size_t _num_programs;
	uint32_t CRC_32;
} program_association_section_t;

program_association_section_t *program_association_section_new();
void program_association_section_free(program_association_section_t *pas);
int program_association_section_read(program_association_section_t *pas, uint8_t *buf, size_t buf_len);
int program_association_section_print(const program_association_section_t *pas, char* str, size_t str_len);

// PMT

typedef struct {
	uint32_t stream_type;
	uint32_t elementary_PID;
	uint32_t ES_info_length;
	vqarray_t *descriptors;
} elementary_stream_info_t;

typedef struct {
	uint32_t table_id;
	uint32_t section_syntax_indicator;
	uint32_t section_length;
	uint32_t program_number;
	uint32_t version_number;
	uint32_t current_next_indicator;
	uint32_t section_number;
	uint32_t last_section_number;
	uint32_t PCR_PID;
	uint32_t program_info_length;
	vqarray_t *descriptors;
	vqarray_t *es_info;
	uint32_t CRC_32;
} program_map_section_t;

program_map_section_t *program_map_section_new();
void program_map_section_free(program_map_section_t *pms);

int program_map_section_read(program_map_section_t *pms, uint8_t *buf, size_t buf_size);
int program_map_section_write(program_map_section_t *pms, uint8_t *buf, size_t buf_size);
int program_map_section_print(program_map_section_t *pms, char* str, size_t str_len);

// stream types
#define STREAM_TYPE_MPEG1_VIDEO             0x01
#define STREAM_TYPE_MPEG2_VIDEO             0x02
#define STREAM_TYPE_MPEG1_AUDIO             0x03
#define STREAM_TYPE_MPEG2_AUDIO             0x04
#define STREAM_TYPE_MPEG2_PRIVATE_SECTIONS  0x05
#define STREAM_TYPE_MPEG2_PRIVATE_PES       0x06
#define STREAM_TYPE_MHEG                    0x07
#define STREAM_TYPE_MPEG2_DSMCC             0x08
#define STREAM_TYPE_ATM_MUX                 0x09
#define STREAM_TYPE_DSMCC_A                 0x0A
#define STREAM_TYPE_DSMCC_B                 0x0B
#define STREAM_TYPE_DSMCC_C                 0x0C
#define STREAM_TYPE_DSMCC_D                 0x0D
#define STREAM_TYPE_MPEG2_AUX               0x0E
#define STREAM_TYPE_MPEG2_AAC               0x0F // ADTS
#define STREAM_TYPE_MPEG4_VIDEO             0x10
#define STREAM_TYPE_MPEG4_AAC               0x11 // LATM
#define STREAM_TYPE MPEG4_SYS_PES           0x12
#define STREAM_TYPE_MPEG2_SYS_SECTION       0x13
#define STREAM_TYPE_DSMCC_SDP               0x14
#define STREAM_TYPE_METADATA_PES            0x15
#define STREAM_TYPE_METADATA_SECTIONS       0x16
#define STREAM_TYPE_METADATA_DSMCC_DATA     0x17
#define STREAM_TYPE_METADATA_DSMCC_OBJ      0x18
#define STREAM_TYPE_METADATA_DSMCC_SDP      0x19
#define STREAM_TYPE_MPEG2_IPMP              0x1A
#define STREAM_TYPE_AVC                     0x1B
#define STREAM_TYPE_MPEG4_AAC_RAW           0x1C
#define STREAM_TYPE_MPEG4_TIMED_TEXT        0x1D
#define STREAM_TYPE_AVSI                    0x1E
#define STREAM_TYPE_SVC                     0x1F
#define STREAM_TYPE_MVC                     0x20
#define STREAM_TYPE_JPEG2000                0x21
#define STREAM_TYPE_S3D_SC_MPEG2            0x22
#define STREAM_TYPE_S3D_SC_AVC              0x23
#define STREAM_TYPE_IPMP		            0x7F

#define PAT_PID			0
#define CAT_PID			1
#define TSDT_PID		2
#define IPMP_CIT_PID	3
#define DASH_PID		4
#define NULL_PID		0x1FFF

#define GENERAL_PURPOSE_PID_MIN		0x0010
#define GENERAL_PURPOSE_PID_MAX		0x1FFE

char *stream_desc(uint8_t stream_id);

// Multi-section tables are not supported
// if there is a need to support them, 
// patches are welcome

typedef struct {
	uint32_t table_id;
	uint32_t section_syntax_indicator;
	uint32_t private_indicator;
	uint32_t section_length;
} section_header_t;

#endif // _PSI_H_
