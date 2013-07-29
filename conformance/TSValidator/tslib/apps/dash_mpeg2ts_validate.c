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

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <strings.h>

#include "libts_common.h"
#include "ts.h"
#include "psi.h"
#include "mpeg2ts_demux.h"
#include "avc_es_handler.h"
#include "log.h"

#define BAIL_ON_TS_ERR		if ((tslib_errno) != 0) goto end

static struct option long_options[] = {
    { "verbose",		no_argument,		NULL,	'v' },
    { "init-segment",	no_argument,		NULL,	'i' },
    { "starts-with-sap",required_argument,	NULL,	's' },
    { "extract",		no_argument,		NULL,	'e' },
    { "help",			no_argument,		NULL,	'h' },
    { 0,				0,					0,		0 	}
    // TODO: have optional argument, so that we can stop somewhere
};

static char options[] =
	"\t-v, --verbose\t\tprint more info (up to two levels of extra verbosity)\n"
	"\t-i, --init-segment\tinput MPEG-2 TS segment is an initialization segment (default: no)\n"
	"\t-s, --starts-with-sap <SAP type>\tinput MPEG-2 TS segment starts with SAP of given type (default: none)\n"
	"\t-e, --extract\t\textract each elementary stream into a separate file (default: no)\n"
	"\t-h, --help\t\tprint this message and exit\n";

static void usage(char* name) {
	fprintf(stderr, "\n%s\n", name);
	fprintf(stderr, "Validates MPEG-2 transport streams for DASH compliance\n");
	fprintf(stderr, "\nUsage: \n%s [options] <input bitstream>\n\nOptions:\n%s\n", name, options);
}

int saw_program_pcr = 0; // real dirty hack

int main(int argc, char *argv[]) {
	int opt_init_seg = 0, opt_starts_with_sap = 0, opt_extract_es = 0, c, long_options_index, bytes = 0, sap_type = -1;
	extern char *optarg;
	extern int optind;

	ts_packet_t *ts = NULL;
	program_association_section_t *pas = NULL;
 	mpeg2ts_stream_t *m2s = NULL;
	mpeg2ts_program_t *m2p = NULL;

//	printf("Log level before parsing cmdline args is %d\n", tslib_loglevel);

	if (argc < 2) {
		usage(argv[0]);
		SAFE_REPORT_TS_ERR(1);
		return 1;
	}

	SAFE_REPORT_TS_ERR(0);
	while ((c = getopt_long(argc, argv, "eisvh", long_options, &long_options_index)) != -1) {
		switch (c) {
		case 'i':
			opt_init_seg = 1;
			break;
		case 's':
			opt_starts_with_sap = 1;
			sap_type = atoi(optarg);
			break;
		case 'e':
			opt_extract_es = 1;
			break;
		case 'v':
			if (tslib_loglevel < TSLIB_LOG_LEVEL_DEBUG)
				tslib_loglevel++;
			break;
		case 'h':
		default:
			usage(argv[0]);
			SAFE_REPORT_TS_ERR(1);
			return 1;
		}
	}

//	printf("Log level after parsing cmdline args is %d\n", tslib_loglevel);

	if (opt_init_seg && opt_starts_with_sap) {
		LOG_ERROR("Segment can either be initialization, or it can start with SAP, but not both");
		usage(argv[0]);
		SAFE_REPORT_TS_ERR(2);
		return 1;
	}

	char *fname = argv[optind];
	if (fname == NULL || fname[0] == 0) {
		usage(argv[0]);
		SAFE_REPORT_TS_ERR(1);
		return 1;
	}

	FILE *infile = NULL;
	if (NULL == (infile = fopen(fname, "rb"))) {
		LOG_ERROR_ARGS("Cannot open file %s - %s", fname, strerror(errno));
		SAFE_REPORT_TS_ERR(2);
		goto end;
	}

	size_t infile_offset = 0;
	char text_buf[0x10000], es_fname[1024];;
	uint8_t ts_buf[TS_SIZE];
//	uint8_t prev_continuity_counter; // TODO impl continuity_counter checking

	// look for PAT and parse it
	while (!pas) {
		bytes = fread(ts_buf, 1, sizeof(ts_buf), infile);
		if (bytes != TS_SIZE) {
			LOG_ERROR_ARGS("Error reading PAT from file %s - %s", fname, strerror(errno));
			SAFE_REPORT_TS_ERR(3);
			goto end;
		}
		LOG_DEBUG_ARGS("[======== Reading TS packet at file offset 0x%08zX ========]", infile_offset);
		infile_offset += sizeof(ts_buf);

		ts = ts_new();
		ts_read(ts, ts_buf, sizeof(ts_buf));
		ts_print(ts, text_buf, sizeof(text_buf));
		BAIL_ON_TS_ERR;
		// FIXME check continuity counter (pg35), incl. up to 2 dupes

		if (ts->header.PID == PAT_PID) { // search for PAT
			if (tslib_loglevel >= TSLIB_LOG_LEVEL_DEBUG) {
				for (int i = 0; i < 32; i += 4)
					LOG_DEBUG_ARGS("%02X %02X %02X %02X", ts_buf[i], ts_buf[i + 1], ts_buf[i + 2], ts_buf[i + 3]);
			}

			pas = program_association_section_new();
			program_association_section_read(pas, ts->payload.bytes + 1, ts->payload.len - 1);
			program_association_section_print(pas, text_buf, sizeof(text_buf));
			BAIL_ON_TS_ERR;
		}

		ts_free(ts);
		ts = NULL;
	}

	// DASH allows only a single program
	if (pas->_num_programs != 1) {
		LOG_ERROR("MPTS found. Only SPTS supported in DASH.");
		SAFE_REPORT_TS_ERR(6);
		goto end;
	}

	// verify the program number is not 0 (else it'd point to a NIT PID instead of PMT PID)
	program_info_t *prog_info = &(pas->programs[0]);
	if (prog_info->program_number == 0) {
		LOG_ERROR("NIT (Network Information Table) not yet supported");
		SAFE_REPORT_TS_ERR(7);
		goto end;
	}

	// verify the PMT PID is in the valid range (per Table 2-3)
	if (prog_info->program_map_PID < GENERAL_PURPOSE_PID_MIN || prog_info->program_map_PID > GENERAL_PURPOSE_PID_MAX) {
		LOG_ERROR_ARGS("PMT PID has invalid value 0x%02X", prog_info->program_map_PID);
		SAFE_REPORT_TS_ERR(8);
		goto end;
	}

	// TODO we assume a single PAT for now
	// TODO we assume PAT to come in a single packet & section for now
	// TODO verify no program_number has more than one PMT PID assignment

	// init MPEG-2 stream and 1st program
	if (NULL == (m2s = mpeg2ts_stream_new())) {
		LOG_ERROR("Error creating MPEG-2 STREAM object");
		SAFE_REPORT_TS_ERR(9);
		goto end;
	}
	m2s->pat = pas;
	if (NULL == (m2p = mpeg2ts_program_new(prog_info->program_number, prog_info->program_map_PID))) {
		LOG_ERROR("Error creating MPEG-2 program object");
		SAFE_REPORT_TS_ERR(10);
		goto end;
	}
	vqarray_add(m2s->programs, m2p);

	// now search for PMT and parse it
	while (!m2p->pmt) {
		bytes = fread(ts_buf, 1, sizeof(ts_buf), infile);
		if (bytes != TS_SIZE) {
			LOG_ERROR_ARGS("Error reading PMT from file %s - %s", fname, strerror(errno));
			SAFE_REPORT_TS_ERR(11);
			goto end;
		}
		LOG_DEBUG_ARGS("[======== Reading TS packet at file offset 0x%08zX ========]", infile_offset);
		infile_offset += sizeof(ts_buf);

		ts = ts_new();
		ts_read(ts, ts_buf, sizeof(ts_buf));
		ts_print(ts, text_buf, sizeof(text_buf));
		BAIL_ON_TS_ERR;
		// FIXME check continuity counter (pg35), incl. up to 2 dupes

		if (ts->header.PID == m2p->PID) { // again, assuming single program in the stream
			if (0 != mpeg2ts_stream_read_ts_packet(m2s, ts)) { // let the lib function parse PMT
				LOG_ERROR("Error parsing PMT");
				SAFE_REPORT_TS_ERR(12);
				goto end;
			}
		} else { // non-PMT packet
			// TODO do something abt early ES packets
		}

		BAIL_ON_TS_ERR;
		ts_free(ts);
		ts = NULL;
	}
	program_map_section_print(m2p->pmt, text_buf, sizeof(text_buf));

	// do some validation of PMT
	if (m2p->pmt->PCR_PID == 0x1FFF) {
		LOG_ERROR("No PCR is associated with program definition");
		SAFE_REPORT_TS_ERR(18);
		goto end;
	}

	// now register handlers for elementary streams in this program
	segment_type seg_type = GENERAL_SEGMENT;
	for (int es_idx=0; es_idx < vqarray_length(m2p->pmt->es_info); es_idx++) {
		elementary_stream_info_t *es = vqarray_get(m2p->pmt->es_info, es_idx);
		if (es == NULL)
			continue;

		demux_pid_handler_t *es_handler = NULL;
		switch (es->stream_type) { // create the demux handler for this particular elementary stream
		case STREAM_TYPE_AVC: // only AVC supported for now
				if (opt_starts_with_sap && (sap_type == 1 || sap_type == 2))
					seg_type = SAP12_SEGMENT;
				else if (opt_init_seg)
					seg_type = INITIALIZATION_SEGMENT;
				else
					seg_type = GENERAL_SEGMENT;

				FILE *es_outfile = NULL;
				if (opt_extract_es) {
					memset(es_fname, 0, ARRAYSIZE(es_fname));
					snprintf(es_fname, ARRAYSIZE(es_fname), "%s.AVC_elementary_stream.PID_0x%04X.264", fname,
						es->elementary_PID);
					es_fname[ARRAYSIZE(es_fname) - 1] = 0;

					es_outfile = fopen(es_fname, "wb");
					if (!es_outfile) {
						LOG_ERROR_ARGS("Error opening elementary stream file \"%s\" for extraction - %s", es_fname,
							strerror(errno));
						SAFE_REPORT_TS_ERR(-1);
						goto end;
					}
					LOG_DEBUG_ARGS("Extracting AVC elementary stream (PID 0x%04X) into file \"%s\"",
						es->elementary_PID, es_fname);
				}

				es_handler = avc_es_handler_new(m2s, m2p, es->elementary_PID, seg_type, es_outfile);
				if (!opt_init_seg)
					break; // if initial segment, we need to check it doesn't contain media streams
		case STREAM_TYPE_MPEG1_VIDEO:
		case STREAM_TYPE_MPEG2_VIDEO:
		case STREAM_TYPE_MPEG1_AUDIO:
		case STREAM_TYPE_MPEG2_AUDIO:
		case STREAM_TYPE_MPEG2_AAC:
		case STREAM_TYPE_MPEG4_VIDEO:
		case STREAM_TYPE_MPEG4_AAC:
		case STREAM_TYPE_MPEG4_AAC_RAW:
			if (opt_init_seg) {
				LOG_ERROR("Media data not allowed in initialization segments");
				SAFE_REPORT_TS_ERR(17);
				goto end;
			}
		default: // FIXME register default handlers for all common elementary stream types
			LOG_WARN_ARGS("No default handler for stream type %d (%s) yet", es->stream_type, stream_desc(es->stream_type));
			break;
		}
		BAIL_ON_TS_ERR;

		if (es_handler != NULL && 0 != mpeg2ts_program_register_pid_processor(m2p, es->elementary_PID, es_handler)) {
			LOG_ERROR_ARGS("Error registering default handler for elementary stream PID=0x%08X type=0x%02X (%s)",
				es->elementary_PID, es->stream_type, stream_desc(es->stream_type));
			SAFE_REPORT_TS_ERR(13);
			goto end;
		}
		BAIL_ON_TS_ERR;
	}

	// now process the rest of the file
	do {
		bytes = fread(ts_buf, 1, sizeof(ts_buf), infile);
		switch (bytes) {
		case TS_SIZE:	// read a complete 188-byte TS packet
			LOG_DEBUG_ARGS("[======== Reading TS packet at file offset 0x%08zX ========]", infile_offset);
			infile_offset += sizeof(ts_buf);

			ts = ts_new();
			ts_read(ts, ts_buf, sizeof(ts_buf));
			ts_print(ts, text_buf, sizeof(text_buf));
			BAIL_ON_TS_ERR;
			// FIXME check continuity counter (pg35), incl. up to two dupes

			switch (ts->header.PID) {
			case PAT_PID:
				{
					program_association_section_t *new_pas = program_association_section_new();
					if (new_pas) {
						if (program_association_section_read(new_pas, ts->payload.bytes + 1, ts->payload.len - 1) > 0 &&
								m2s->pat->version_number != pas->version_number) // if reading fails, ver# will be left as 0
							LOG_WARN("Updating PAT section not supported yet");
						program_association_section_free(new_pas);
					}
				}
				break;
			case CAT_PID:
				LOG_WARN("CAT section not supported yet");
				break;
			case TSDT_PID:
				LOG_WARN("TSDT section not supported yet");
				break;
			case IPMP_CIT_PID:
				LOG_WARN("IPMP CIT section not supported yet");
				break;
			case NULL_PID:
				break;
			default: // assuming either a reserved PID, or a PMT, or a PES packet
				if (ts->header.PID == m2p->pmt->PCR_PID) {
					// FIXME actually parse the PCR
					if (!saw_program_pcr && (ts->header.adaptation_field_control & TS_ADAPTATION_FIELD)
						&& ts->adaptation_field.PCR_flag) {
						saw_program_pcr = 1;
					}
				}
				if (ts->header.PID < GENERAL_PURPOSE_PID_MIN || ts->header.PID > GENERAL_PURPOSE_PID_MAX)
					break; // skipping reserved PIDs for now

				if (0 != mpeg2ts_stream_read_ts_packet(m2s, ts)) { // finally invoke mpeg2ts_stream_read_ts_packet()
					LOG_ERROR("Error processing MPEG-2 TS packet via library function");
					SAFE_REPORT_TS_ERR(14);
					goto end;
				}

				break;
			}
			BAIL_ON_TS_ERR;

			ts_free(ts);
			ts = NULL;
			break;
		case 0:			// read 0 bytes: either end of file, or an error reading
			if (feof(infile)) {
				SAFE_REPORT_TS_ERR(0); // does nothing, merely an illustrative statement
			} else { // ferror(infile)
				LOG_ERROR_ARGS("Error reading TS packet from file %s at offset 0x%08zX - %s",
					fname, infile_offset, strerror(errno));
				SAFE_REPORT_TS_ERR(15);
			}
			break;
		default:		// read some bytes, but less than 188; it's a truncated TS packet
			LOG_ERROR_ARGS("Read truncated TS packet (expected %d bytes, got %d) from file %s at offset 0x%08zX - %s",
				TS_SIZE, bytes, fname, infile_offset, strerror(errno));
			SAFE_REPORT_TS_ERR(16);
			break;
		}

		BAIL_ON_TS_ERR;
	} while (bytes == TS_SIZE); // loop while we're reading complete TS packets

	// final checks

	if (!saw_program_pcr) {
		LOG_ERROR("No PCR information in the segment");
		SAFE_REPORT_TS_ERR(17);
	}

	SAFE_REPORT_TS_ERR(0); // likewise, does nothing
end:
	if (m2s) 						// trigger deletion of MPEG-2 TS stream here,
		mpeg2ts_stream_free(m2s);	// this will catch incomplete bounded PES packets at the end of elementary streams
	else if (pas)
		program_association_section_free(pas);
	if (ts)
		ts_free(ts);
	if (infile) {
		fclose(infile);
		infile = NULL;
	}

	char *file_desc;
	switch (seg_type) {
	case INITIALIZATION_SEGMENT:
		file_desc = "initialization segment";
		break;
	case SAP12_SEGMENT:
		file_desc = "media segment of SAP type 1/2";
		break;
	case GENERAL_SEGMENT:
	default:
		file_desc = "media segment";
		break;
	}

	if (tslib_errno == 0)
		printf("\n\n[SUCCESS]\tFile \"%s\" is a valid DASH MPEG-2 TS %s\n\n", fname, file_desc);
	else {
		printf("\n\n[FAILURE]\tFile \"%s\" is not a valid DASH MPEG-2 TS %s (message %d). ", fname, file_desc, tslib_errno);
		printf("\n\t\t%sheck the log for warnings and errors.\n\n",
			(tslib_loglevel < TSLIB_LOG_LEVEL_DEBUG ? "Re-run validation with higher verbosity parameter, and c" : "C"));
	}

	return tslib_errno;
}
