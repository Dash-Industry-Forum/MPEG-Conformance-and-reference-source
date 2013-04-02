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

#ifndef _PES_H_
#define _PES_H_

#include <stdint.h>

#include "bs.h"
#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

// PTS/DTS
#define PES_PTS_FLAG 0x02
#define PES_DTS_FLAG 0x01

// PES stream ID's

#define PES_STREAM_ID_PROGRAM_STREAM_MAP       0xBC
#define PES_STREAM_ID_PROGRAM_STREAM_DIRECTORY 0xFF

#define PES_STREAM_ID_PRIVATE_1                0xBD
#define PES_STREAM_ID_PADDING                  0xBE
#define PES_STREAM_ID_PRIVATE_2                0xBF
#define PES_STREAM_ID_AUDIO_MIN                0xC0 
#define PES_STREAM_ID_AUDIO_MAX                0xDF
#define PES_STREAM_ID_VIDEO_MIN                0xEF
#define PES_STREAM_ID_VIDEO_MAX                0xEF
#define PES_STREAM_ID_ECM                      0xF0
#define PES_STREAM_ID_EMM                      0xF1
#define PES_STREAM_ID_DSMCC                    0xF2
#define PES_STREAM_ID_MHEG                     0xF3
#define PES_STREAM_ID_H222_1_TYPE_A            0xF4
#define PES_STREAM_ID_H222_1_TYPE_B            0xF5
#define PES_STREAM_ID_H222_1_TYPE_C            0xF6
#define PES_STREAM_ID_H222_1_TYPE_D            0xF7
#define PES_STREAM_ID_H222_1_TYPE_E            0xF8
#define PES_STREAM_ID_ANCILLARY                0xF9
#define PES_STREAM_ID_MPEG4_SYS_SL             0xFA
#define PES_STREAM_ID_MP4_SYS_FLEXMUX          0xFB
#define PES_STREAM_ID_METADATA                 0xFC
#define PES_STREAM_ID_EXTENDED                 0xFD
#define PES_STREAM_ID_RESERVED                 0xFE

#define PES_STREAM_ID_EXT_IPMP_CONTROL         0x00
#define PES_STREAM_ID_EXT_IPMP                 0x01
#define PES_STREAM_ID_EXT_MPEG4_TIMED_TEXT_MIN 0x02
#define PES_STREAM_ID_EXT_MPEG4_TIMED_TEXT_MAX 0x0F
#define PES_STREAM_ID_EXT_AVSI_VIDEO_MIN       0x10 
#define PES_STREAM_ID_EXT_AVSI_VIDEO_MAX       0x1F

#define HAS_PES_HEADER(SID)  (((SID) != PES_STREAM_ID_PROGRAM_STREAM_MAP )	\
         && ((SID) != PES_STREAM_ID_PADDING )                               \
         && ((SID) != PES_STREAM_ID_PRIVATE_2 )                             \
         && ((SID) != PES_STREAM_ID_ECM )                                   \
         && ((SID) != PES_STREAM_ID_EMM )                                   \
         && ((SID) != PES_STREAM_ID_PROGRAM_STREAM_DIRECTORY )              \
         && ((SID) != PES_STREAM_ID_DSMCC )                                 \
         && ((SID) != PES_STREAM_ID_H222_1_TYPE_E ) )

// Trick Mode
#define PES_DSM_TRICK_MODE_CTL_FAST_FORWARD    0x00
#define PES_DSM_TRICK_MODE_CTL_SLOW_MOTION     0x01
#define PES_DSM_TRICK_MODE_CTL_FREEZE_FRAME    0x02
#define PES_DSM_TRICK_MODE_CTL_FAST_REVERSE    0x03
#define PES_DSM_TRICK_MODE_CTL_SLOW_REVERSE    0x04

#define PES_PACKET_START_CODE_PREFIX           0x000001

typedef struct {
	uint32_t stream_id;
	uint32_t PES_packet_length;
	uint32_t PES_scrambling_control;
	uint32_t PES_priority;
	uint32_t data_alignment_indicator;
	uint32_t copyright;
	uint32_t original_or_copy;
	uint32_t PTS_DTS_flags;
	uint32_t ESCR_flag;
	uint32_t ES_rate_flag;
	uint32_t DSM_trick_mode_flag;
	uint32_t additional_copy_info_flag;
	uint32_t PES_CRC_flag;
	uint32_t PES_extension_flag;
	uint32_t PES_header_data_length;
	uint64_t PTS;
	uint64_t DTS;
	uint64_t ESCR_base;
	uint32_t ESCR_extension;
	uint32_t ES_rate;
	uint32_t trick_mode_control;
	uint32_t field_id;
	uint32_t intra_slice_refresh;
	uint32_t frequency_truncation;
	uint32_t rep_cntrl;
	uint32_t additional_copy_info;
	uint32_t previous_PES_packet_CRC;
	uint32_t PES_private_data_flag;
	uint32_t pack_header_field_flag;
	uint32_t program_packet_sequence_counter_flag;
	uint32_t PSTD_buffer_flag;
	uint32_t PES_extension_flag_2;
	uint8_t PES_private_data[16];
	uint32_t pack_field_length;
	uint32_t program_packet_sequence_counter;
	uint32_t MPEG1_MPEG2_identifier;
	uint32_t original_stuff_length;
	uint32_t PSTD_buffer_scale;
	uint32_t PSTD_buffer_size;
	uint32_t PES_extension_field_length;
	uint32_t stream_id_extension_flag;
	uint32_t stream_id_extension;
	uint32_t tref_extension_flag;
	uint64_t TREF;
} pes_header_t;

typedef struct {
	pes_header_t header;
	buf_t payload;
	void *opaque;
} pes_packet_t;

#define PES_HEADER_LEN_ESTIMATE	66

pes_packet_t *pes_new(size_t payload_size);
void pes_free(pes_packet_t *pes);

int pes_read_header(pes_header_t *ph, bs_t *b);
int pes_read(pes_packet_t *pes, uint8_t *buf, size_t len);

int pes_write_header(pes_header_t *ph, bs_t *b);
int pes_write(pes_packet_t *pes, uint8_t *buf, size_t len);

int pes_print_header(pes_header_t *pes_header, char* str, size_t str_len);
int pes_print(pes_packet_t *pes, char* str, size_t str_len);

#ifdef __cplusplus
}
#endif

#endif // _PES_H_
