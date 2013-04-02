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
#include "libts_common.h"
#include "ts.h"
#include "psi.h"
#include "descriptors.h"
#include "section.h"
#include "log.h"
#include "crc32m.h"

typedef struct {
	int type;
	char name[0x100];
} _stream_types_t;

static char *first_stream_types[] = { // 0x00 - 0x23
	"ITU-T | ISO/IEC Reserved",
	"ISO/IEC 11172-2 Video",
	"Rec. ITU-T H.262 | ISO/IEC 13818-2 Video or ISO/IEC 11172-2 constrained parameter video stream (see note 2)",
	"ISO/IEC 11172-3 Audio",
	"ISO/IEC 13818-3 Audio",
	"ISO/IEC 13818-1 private_sections",
	"ISO/IEC 13818-1 PES packets containing private data",
	"ISO/IEC 13522 MHEG",
	"ISO/IEC 13818-1 Annex A DSM-CC",
	"Rec. ITU-T H.222.1",
	"ISO/IEC 13818-6 type A",
	"ISO/IEC 13818-6 type B",
	"ISO/IEC 13818-6 type C",
	"ISO/IEC 13818-6 type D",
	"ISO/IEC 13818-1 auxiliary",
	"ISO/IEC 13818-7 Audio with ADTS transport syntax",
	"ISO/IEC 14496-2 Visual",
	"ISO/IEC 14496-3 Audio with the LATM transport syntax as defined in ISO/IEC 14496-3",
	"ISO/IEC 14496-1 SL-packetized stream or FlexMux stream carried in PES packets",
	"ISO/IEC 14496-1 SL-packetized stream or FlexMux stream carried in ISO/IEC 14496_sections",
	"ISO/IEC 13818-6 Synchronized Download Protocol",
	"Metadata carried in PES packets",
	"Metadata carried in metadata_sections",
	"Metadata carried in ISO/IEC 13818-6 Data Carousel",
	"Metadata carried in ISO/IEC 13818-6 Object Carousel",
	"Metadata carried in ISO/IEC 13818-6 Synchronized Download Protocol",
	"IPMP stream (defined in ISO/IEC 13818-11, MPEG-2 IPMP)",
	"AVC video stream conforming to one or more profiles defined in Annex A of Rec. ITU-T H.264 | ISO/IEC 14496-10 or AVC video sub-bitstream of SVC as defined in 2.1.78 or MVC base view sub-bitstream, as defined in 2.1.85, or AVC video sub-bitstream of MVC, as defined in 2.1.88",
	"ISO/IEC 14496-3 Audio, without using any additional transport syntax, such as DST, ALS and SLS",
	"ISO/IEC 14496-17 Text",
	"Auxiliary video stream as defined in ISO/IEC 23002-3",
	"SVC video sub-bitstream of an AVC video stream conforming to one or more profiles defined in Annex G of Rec. ITU-T H.264 | ISO/IEC 14496-10",
	"MVC video sub-bitstream of an AVC video stream conforming to one or more profiles defined in Annex H of Rec. ITU-T H.264 | ISO/IEC 14496-10",
	"Video stream conforming to one or more profiles as defined in Rec. ITU-T T.800 | ISO/IEC 15444-1",
	"Additional view Rec. ITU-T H.262 | ISO/IEC 13818-2 video stream for service-compatible stereoscopic 3D services (see note 3 and 4)",
	"Additional view Rec. ITU-T H.264 | ISO/IEC 14496-10 video stream conforming to one or more profiles defined in Annex A for service-compatible stereoscopic 3D services (see note 3 and 4)"
};

static char *STREAM_DESC_RESERVED 		= "ISO/IEC 13818-1 Reserved";	// 0x24-0x7E
static char *STREAM_DESC_IPMP			= "IPMP Stream";				// 0x7F
static char *STREAM_DESC_USER_PRIVATE	= "User Private";				// 0x80-0xFF

char *stream_desc(uint8_t stream_id) {
	if (stream_id < ARRAYSIZE(first_stream_types))
		return first_stream_types[stream_id];
	else if (stream_id < STREAM_TYPE_IPMP)
		return STREAM_DESC_RESERVED;
	else if (stream_id == STREAM_TYPE_IPMP)
		return STREAM_DESC_IPMP;
	else
		return STREAM_DESC_USER_PRIVATE;
}

static inline uint32_t section_header_read(section_header_t *sh, bs_t *b) {
	sh->table_id = bs_read_u8(b);
	sh->section_syntax_indicator = bs_read_u1(b);
	sh->private_indicator = bs_read_u1(b);
	bs_skip_u(b,2);
	sh->section_length = bs_read_u(b,12);
	return sh->table_id;
}

program_association_section_t *program_association_section_new() {
	program_association_section_t *pas = (program_association_section_t*) calloc(1, sizeof(program_association_section_t));
	return pas;
}

void program_association_section_free(program_association_section_t *pas) {
	if (pas == NULL)
		return;

	if (pas->programs != NULL)
		free(pas->programs);
	free(pas);
}

int program_association_section_read(program_association_section_t *pas, uint8_t *buf, size_t buf_len) {
	if (pas == NULL || buf == NULL) {
		SAFE_REPORT_TS_ERR(-1);
		return 0;
	}

	bs_t *b = bs_new(buf, buf_len);

	pas->table_id = bs_read_u8(b);
	if (pas->table_id != program_association_section) {
		LOG_ERROR_ARGS("Table ID in PAT is 0x%02X instead of expected 0x%02X",
			pas->table_id, program_association_section);
		SAFE_REPORT_TS_ERR(-30);
		return 0;
	}

	// read byte 0

	pas->section_syntax_indicator = bs_read_u1(b);
	if (!pas->section_syntax_indicator) {
		LOG_ERROR("section_syntax_indicator not set in PAT");
		SAFE_REPORT_TS_ERR(-31);
		return 0;
	}
	bs_skip_u(b, 3); // TODO read the zero bit, check it to be zero
	pas->section_length = bs_read_u(b, 12);
	if (pas->section_length > MAX_SECTION_LEN) {
		LOG_ERROR_ARGS("PAT section length is 0x%02X, larger than maximum allowed 0x%02X",
			pas->section_length, MAX_SECTION_LEN);
		SAFE_REPORT_TS_ERR(-32);
		return 0;
	}

	// read bytes 1,2

	pas->transport_stream_id = bs_read_u16(b);

	// read bytes 3,4

	bs_skip_u(b, 2);
	pas->version_number = bs_read_u(b, 5);
	pas->current_next_indicator = bs_read_u1(b);
	if (!pas->current_next_indicator)
		LOG_WARN("This PAT is not yet applicable/n");

	// read byte 5

	pas->section_number = bs_read_u8(b);
	pas->last_section_number = bs_read_u8(b);
	if (pas->section_number != 0 || pas->last_section_number != 0)
		LOG_WARN("Multi-section PAT is not supported yet/n");

	// read bytes 6,7

	pas->_num_programs = (pas->section_length - 5 - 4) / 4;
	// explanation: section_length gives us the length from the end of section_length
	// we used 5 bytes for the mandatory section fields, and will use another 4 bytes for CRC
	// the remaining bytes contain program information, which is 4 bytes per iteration
	// It's much shorter in C :-)

	if (pas->_num_programs > 1)
		LOG_WARN_ARGS("%zd programs found, but only SPTS is fully supported. Patches are welcome.", pas->_num_programs);

	pas->programs = malloc(pas->_num_programs * sizeof(program_info_t));
	for (uint32_t i = 0; i < pas->_num_programs; i++) {
		pas->programs[i].program_number = bs_read_u16(b);
		bs_skip_u(b, 3);
		pas->programs[i].program_map_PID = bs_read_u(b, 13);
	}

	pas->CRC_32 = bs_read_u32(b);

	// check CRC
	crc_t pas_crc = crc_init();
	pas_crc = crc_update(pas_crc, buf, bs_pos(b) - 4);
	pas_crc = crc_finalize(pas_crc);
	if (pas_crc != pas->CRC_32) {
		LOG_ERROR_ARGS("PAT CRC_32 specified as 0x%08X, but calculated as 0x%08X", pas->CRC_32, pas_crc);
		SAFE_REPORT_TS_ERR(-33);
		return 0;
	} else {
		LOG_DEBUG("PAT CRC_32 checked successfully");
	}

	bs_free(b);
	return 1;
}

//int program_association_section_write(program_association_section_t *pat, bs_t *bs);
int program_association_section_print(const program_association_section_t *pas, char* str, size_t str_len) {
	if (pas == NULL || str == NULL || str_len < 2 || tslib_loglevel < TSLIB_LOG_LEVEL_INFO)
		return 0;
	int bytes = 0;

	LOG_INFO("Program Association Section");
	bytes += SKIT_LOG_UINT		(str + bytes, 0, pas->table_id, str_len);
	bytes += SKIT_LOG_UINT		(str + bytes, 0, pas->section_syntax_indicator, str_len - bytes);
	bytes += SKIT_LOG_UINT		(str + bytes, 0, pas->section_length, str_len - bytes);
	bytes += SKIT_LOG_UINT_HEX	(str + bytes, 0, pas->transport_stream_id , str_len - bytes);
	bytes += SKIT_LOG_UINT		(str + bytes, 0, pas->version_number, str_len - bytes);
	bytes += SKIT_LOG_UINT		(str + bytes, 0, pas->current_next_indicator, str_len - bytes);
	bytes += SKIT_LOG_UINT		(str + bytes, 0, pas->section_number, str_len - bytes);
	bytes += SKIT_LOG_UINT		(str + bytes, 0, pas->last_section_number, str_len - bytes);

	for (int i = 0; i < pas->_num_programs; i++) {
		bytes += SKIT_LOG_UINT		(str + bytes, 1, pas->programs[i].program_number, str_len - bytes);
		bytes += SKIT_LOG_UINT_HEX	(str + bytes, 1, pas->programs[i].program_map_PID, str_len - bytes);
	}
	bytes += SKIT_LOG_UINT_HEX	(str + bytes, 0, pas->CRC_32 , str_len - bytes);
	return bytes;
}

elementary_stream_info_t *es_info_new() {
	elementary_stream_info_t *es = calloc(1, sizeof(elementary_stream_info_t));
	es->descriptors = vqarray_new();
	return es;
}

void es_info_free(elementary_stream_info_t *es) {
	if (es == NULL)
		return;

	if (es->descriptors != NULL) {
		vqarray_foreach(es->descriptors, descriptor_free);
		vqarray_free(es->descriptors);
	}
	free(es);
}

int es_info_read(elementary_stream_info_t *es, bs_t *b) {
	int es_info_start = bs_pos(b);
	es->stream_type = bs_read_u8(b);
	bs_skip_u(b, 3);
	es->elementary_PID = bs_read_u(b, 13);
	bs_skip_u(b, 4);
	es->ES_info_length = bs_read_u(b, 12);

	read_descriptor_loop(es->descriptors, b, es->ES_info_length);
	if (es->ES_info_length > MAX_ES_INFO_LEN) {
		LOG_ERROR_ARGS("ES info length is 0x%02X, larger than maximum allowed 0x%02X",
			es->ES_info_length, MAX_ES_INFO_LEN);
		SAFE_REPORT_TS_ERR(-60);
		return 0;
	}

	return bs_pos(b) - es_info_start;
}

//int es_info_write(elementary_stream_info_t *es, uint8_t *buf, size_t buf_size);
int es_info_print(elementary_stream_info_t *es, int level, char* str, size_t str_len) {
	if (es == NULL || str == NULL || str_len < 2 || tslib_loglevel < TSLIB_LOG_LEVEL_INFO)
		return 0;
	int bytes = 0;

	bytes += SKIT_LOG_UINT_VERBOSE(str + bytes, level, es->stream_type, stream_desc(es->stream_type), str_len - bytes);
	bytes += SKIT_LOG_UINT_HEX(str + bytes, level, es->elementary_PID, str_len - bytes);
	bytes += SKIT_LOG_UINT(str + bytes, level, es->ES_info_length, str_len - bytes);

	bytes += print_descriptor_loop(es->descriptors, level + 1, str + bytes, str_len - bytes);
	return bytes;
}

int write_es_info_loop(vqarray_t *es_list, bs_t *b) {
	return 0; // TODO actually implement write_es_info_loop
}

int print_es_info_loop(vqarray_t *es_list, int level, char* str, size_t str_len) {
	int bytes = 0;
	for (int i = 0; i < vqarray_length(es_list); i++) {
		elementary_stream_info_t *es = vqarray_get(es_list, i);
		if (es != NULL)
			bytes += es_info_print(es, level, str + bytes, str_len - bytes);
	}
	return bytes;
}

program_map_section_t *program_map_section_new() {
	program_map_section_t *pms = calloc(1, sizeof(program_map_section_t));
	pms->descriptors = vqarray_new();
	pms->es_info = vqarray_new();
	return pms;
}

void program_map_section_free(program_map_section_t *pms) {
	if (pms == NULL)
		return;

	if (pms->descriptors != NULL) {
		vqarray_foreach(pms->descriptors, descriptor_free);
		vqarray_free(pms->descriptors);
	}

	if (pms->es_info) {
		vqarray_foreach(pms->es_info, (vqarray_functor_t) es_info_free);
		vqarray_free(pms->es_info);
	}

	free(pms);
}

int program_map_section_read(program_map_section_t *pms, uint8_t *buf, size_t buf_size) {
	if (pms == NULL || buf == NULL) {
		SAFE_REPORT_TS_ERR(-1);
		return 0;
	}

	bs_t *b = bs_new(buf, buf_size);

	pms->table_id = bs_read_u8(b);
	if (pms->table_id != TS_program_map_section) {
		LOG_ERROR_ARGS("Table ID in PMT is 0x%02X instead of expected 0x%02X", pms->table_id, TS_program_map_section);
		SAFE_REPORT_TS_ERR(-40);
		return 0;
	}

	pms->section_syntax_indicator = bs_read_u1(b);
	if (!pms->section_syntax_indicator) {
		LOG_ERROR("section_syntax_indicator not set in PMT");
		SAFE_REPORT_TS_ERR(-41);
		return 0;
	}

	bs_skip_u(b, 3);
	pms->section_length = bs_read_u(b, 12);
	if (pms->section_length > MAX_SECTION_LEN) {
		LOG_ERROR_ARGS("PMT section length is 0x%02X, larger than maximum allowed 0x%02X",
			pms->section_length, MAX_SECTION_LEN);
		SAFE_REPORT_TS_ERR(-42);
		return 0;
	}

	int section_start = bs_pos(b);

	// bytes 0,1
	pms->program_number = bs_read_u16(b);

	// byte 2;
	bs_skip_u(b, 2);
	pms->version_number = bs_read_u(b, 5);
	pms->current_next_indicator = bs_read_u1(b);
	if (!pms->current_next_indicator)
		LOG_WARN("This PMT is not yet applicable/n");

	// bytes 3,4
	pms->section_number = bs_read_u8(b);
	pms->last_section_number = bs_read_u8(b);
	if (pms->section_number != 0 || pms->last_section_number != 0) {
		LOG_ERROR("Multi-section PMT is not allowed/n");
		SAFE_REPORT_TS_ERR(-43);
		return 0;
	}

	bs_skip_u(b, 3);
	pms->PCR_PID = bs_read_u(b, 13);
	if (pms->PCR_PID < GENERAL_PURPOSE_PID_MIN || pms->PCR_PID > GENERAL_PURPOSE_PID_MAX) {
		LOG_ERROR_ARGS("PCR PID has invalid value 0x%02X", pms->PCR_PID);
		SAFE_REPORT_TS_ERR(-44);
		return 0;
	}
	bs_skip_u(b, 4);

	pms->program_info_length = bs_read_u(b, 12);
	if (pms->program_info_length > MAX_PROGRAM_INFO_LEN) {
		LOG_ERROR_ARGS("PMT program info length is 0x%02X, larger than maximum allowed 0x%02X",
			pms->program_info_length, MAX_PROGRAM_INFO_LEN);
		SAFE_REPORT_TS_ERR(-45);
		return 0;
	}

	read_descriptor_loop(pms->descriptors, b, pms->program_info_length);

	while (pms->section_length - (bs_pos(b) - section_start) > 4) { // account for CRC
		elementary_stream_info_t *es = es_info_new();
		es_info_read(es, b);
		vqarray_add(pms->es_info, es);
	}

	pms->CRC_32 = bs_read_u32(b);

	// check CRC
	crc_t pas_crc = crc_init();
	pas_crc = crc_update(pas_crc, buf, bs_pos(b) - 4);
	pas_crc = crc_finalize(pas_crc);
	if (pas_crc != pms->CRC_32) {
		LOG_ERROR_ARGS("PMT CRC_32 specified as 0x%08X, but calculated as 0x%08X", pms->CRC_32, pas_crc);
		SAFE_REPORT_TS_ERR(-46);
		return 0;
	} else {
		LOG_DEBUG("PMT CRC_32 checked successfully");
	}

	int bytes_read = bs_pos(b);
	bs_free(b);

	return bytes_read;
}
// int program_map_section_write(program_map_section_t *pms, uint8_t *buf, size_t buf_size);
int program_map_section_print(program_map_section_t *pms, char* str, size_t str_len) {
	if (pms == NULL || str == NULL || str_len < 2 || tslib_loglevel < TSLIB_LOG_LEVEL_INFO)
		return 0;
	int bytes = 0;

	LOG_INFO("Program Map Section");
	bytes += SKIT_LOG_UINT		(str + bytes, 1, pms->table_id, str_len - bytes);
	bytes += SKIT_LOG_UINT		(str + bytes, 1, pms->section_syntax_indicator, str_len - bytes);

	bytes += SKIT_LOG_UINT		(str + bytes, 1, pms->section_length, str_len - bytes);
	bytes += SKIT_LOG_UINT		(str + bytes, 1, pms->program_number, str_len - bytes);

	bytes += SKIT_LOG_UINT		(str + bytes, 1, pms->version_number, str_len - bytes);
	bytes += SKIT_LOG_UINT		(str + bytes, 1, pms->current_next_indicator, str_len - bytes);

	bytes += SKIT_LOG_UINT		(str + bytes, 1, pms->section_number , str_len - bytes);
	bytes += SKIT_LOG_UINT		(str + bytes, 1, pms->last_section_number , str_len - bytes);

	bytes += SKIT_LOG_UINT_HEX	(str + bytes, 1, pms->PCR_PID , str_len - bytes);

	bytes += SKIT_LOG_UINT		(str + bytes, 1, pms->program_info_length , str_len - bytes);

	bytes += print_descriptor_loop(pms->descriptors, 2, str + bytes, str_len - bytes);

	bytes += print_es_info_loop(pms->es_info, 2, str + bytes, str_len - bytes);

	bytes += SKIT_LOG_UINT_HEX	(str + bytes, 1, pms->CRC_32, str_len - bytes);

	return bytes;
}
