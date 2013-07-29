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

#ifndef _TSLIB_DESCRIPTORS_H_
#define _TSLIB_DESCRIPTORS_H_        

#include <stdint.h>

#include "bs.h"
#include "common.h"
#include "vqarray.h"
#include "log.h"

typedef enum {
	DESCRIPTOR_TAG_RESERVED = 0,
	DESCRIPTOR_TAG_FORBIDDEN,
    VIDEO_STREAM_DESCRIPTOR,
    AUDIO_STREAM_DESCRIPTOR,
    HIERARCHY_DESCRIPTOR,
    REGISTRATION_DESCRIPTOR,
    DATA_STREAM_ALIGNMENT_DESCRIPTOR,
    TARGET_BACKGROUND_GRID_DESCRIPTOR, 
    VIDEO_WINDOW_DESCRIPTOR, 
    CA_DESCRIPTOR, 
    ISO_639_LANGUAGE_DESCRIPTOR,
    SYSTEM_CLOCK_DESCRIPTOR,
    MULTIPLEX_BUFFER_UTILIZATION_DESCRIPTOR,
    COPYRIGHT_DESCRIPTOR,
    MAXIMUM_BITRATE_DESCRIPTOR,
    PRIVATE_DATA_INDICATOR_DESCRIPTOR,
    SMOOTHING_BUFFER_DESCRIPTOR,
    STD_DESCRIPTOR,
    IBP_DESCRIPTOR,
    MPEG4_VIDEO_DESCRIPTOR = 27,
    MPEG4_AUDIO_DESCRIPTOR,
    IOD_DESCRIPTOR,
    SL_DESCRIPTOR,
    FMC_DESCRIPTOR,
    EXTERNAL_ES_ID_DESCRIPTOR,
    MUXCODE_DESCRIPTOR,
    FMXBUFFERSIZE_DESCRIPTOR,
    MULTIPLEXBUFFER_DESCRIPTOR,
    CONTENT_LABELING_DESCRIPTOR,
    METADATA_POINTER_DESCRIPTOR,
    METADATA_DESCRIPTOR,
    METADATA_STD_DESCRIPTOR,
    AVC_VIDEO_DESCRIPTOR,
    IPMP_DESCRIPTOR,
    AVC_TIMING_HRD_DESCRIPTOR,
    MPEG2_AAC_AUDIO_DESCRIPTOR,
    FLEXMUX_TIMING_DESCRIPTOR,
    MPEG4_TEXT_DESCRIPTOR,
    MPEG4_AUDIO_EXTENSION_DESCRIPTOR,
    AUXILIARY_VIDEO_STREAM_DESCRIPTOR,
    SVC_EXTENSION_DESCRIPTOR,
    MVC_EXTENSION_DESCRIPTOR,
    J2K_VIDEO_DESCRIPTOR,
    MVC_OPERATION_POINT_DESCRIPTOR,
    MPEG2_STEREOSCOPIC_VIDEO_FORMAT_DESCRIPTOR,
    STEREOSCOPIC_PROGRAM_INFO_DESCRIPTOR,
    STEREOSCOPIC_VIDEO_INFO_DESCRIPTOR,
} mpeg_descriptor_t; // descriptors defined in ISO/IEC 13818-1:2012


// "factory methods"
int read_descriptor_loop(vqarray_t* desc_list, bs_t* b, int length);
int write_descriptor_loop(vqarray_t* desc_list, bs_t* b);
int print_descriptor_loop(vqarray_t* desc_list, int level, char* str, size_t str_len);

typedef struct {
	uint32_t tag;
	uint32_t length;
} descriptor_t;

descriptor_t* descriptor_new();
void descriptor_free(descriptor_t* desc);
descriptor_t* descriptor_read(descriptor_t* desc, bs_t* b);
int descriptor_print(const descriptor_t* desc, int level, char* str, size_t str_len);

typedef struct {
	char ISO_639_language_code[4];
	uint32_t audio_type;
} iso639_lang_t;

typedef struct {
	descriptor_t descriptor;
	iso639_lang_t* languages;
	int _num_languages;
} language_descriptor_t;

descriptor_t* language_descriptor_new(descriptor_t* desc);
int language_descriptor_free(descriptor_t* desc);
descriptor_t* language_descriptor_read(descriptor_t* desc, bs_t* b);
int language_descriptor_print(const descriptor_t* desc, int level, char* str, size_t str_len);

#endif
