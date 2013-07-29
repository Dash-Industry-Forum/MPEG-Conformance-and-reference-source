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
#include "mpeg2ts_demux.h"

pid_info_t *pid_info_new() {
	pid_info_t *pi = calloc(1, sizeof(pid_info_t));
	return pi;
}

void pid_info_free(pid_info_t *pi) {
	if (pi == NULL)
		return;

	if (pi->demux_handler != NULL) {
		if ((pi->demux_handler->arg != NULL) && (pi->demux_handler->arg_destructor != NULL)) {
			pi->demux_handler->arg_destructor(pi->demux_handler->arg);
		}
		free(pi->demux_handler);
		pi->demux_handler = NULL;
	}

	// note: es_info is a reference, don't free it!
	//       the real thing is in pmt

	free(pi);
}

mpeg2ts_program_t *mpeg2ts_program_new(int program_number, int PID) {
	mpeg2ts_program_t *m2p = calloc(1, sizeof(mpeg2ts_program_t));
	m2p->pids = vqarray_new();
	m2p->PID = PID;
	m2p->program_number = program_number;
	return m2p;
}

void mpeg2ts_program_free(mpeg2ts_program_t *m2p) {
	if (m2p == NULL)
		return;

	if (m2p->pids != NULL) {
		vqarray_foreach(m2p->pids, pid_info_free);
		vqarray_free(m2p->pids);
	}

	if (m2p->pmt != NULL) {
		program_map_section_free(m2p->pmt);
	}

	free(m2p);
}

int mpeg2ts_program_register_pid_processor(mpeg2ts_program_t *m2p, uint32_t PID, demux_pid_handler_t *handler) {
	if (m2p == NULL || m2p->pmt == NULL || handler == NULL)
		return 0;

	// if some handler already registered for this PID before, unregister it
	mpeg2ts_program_unregister_pid_processor(m2p, PID);

	pid_info_t *pid = pid_info_new();
	elementary_stream_info_t *esi = NULL;
	for (int i=0; i < vqarray_length(m2p->pmt->es_info); i++) {
		elementary_stream_info_t *tmp = vqarray_get(m2p->pmt->es_info, i);
		if (tmp != NULL && tmp->elementary_PID == PID) {
			esi = tmp;
			break;
		}
	}
	if (esi == NULL) {
		LOG_ERROR_ARGS("Elementary stream with PID 0x%02X not found in PMT of program %d", PID, m2p->program_number);
		SAFE_REPORT_TS_ERR(-71);
		return -1;
	}
	pid->es_info = esi;
	pid->demux_handler = handler;
	vqarray_add(m2p->pids, pid);

	return 0;
}

int mpeg2ts_program_unregister_pid_processor(mpeg2ts_program_t *m2p, uint32_t PID) {
	if (m2p == NULL || m2p->pmt == NULL)
		return 0;

	int i;
	pid_info_t *pi = NULL;
	for (i = 0; i < vqarray_length(m2p->pids); i++) { // TODO replace linear search w/ hashtable lookup in the future
		pid_info_t *tmp = NULL;
		if ((tmp = vqarray_get(m2p->pids, i)) != NULL && tmp->es_info->elementary_PID == PID) {
			pi = tmp;
			break;
		}
	}
	if (pi == NULL) // not found
		return 0;

	pid_info_free(pi);
	vqarray_remove(m2p->pids, i);

	return 0;
}

pid_info_t *mpeg2ts_program_get_pid_info(mpeg2ts_program_t *m2p, int PID) {
	if (m2p == NULL || m2p->pmt == NULL) // if we don't have any PSI, there's nothing we can do
		return NULL;
	pid_info_t *pi = NULL;

	for (int i = 0; i < vqarray_length(m2p->pids); i++) { // TODO replace linear search w/ hashtable lookup in the future
		pid_info_t *tmp = NULL;
		if ((tmp = vqarray_get(m2p->pids, i)) != NULL) {
			if (tmp->es_info->elementary_PID == PID) {
				pi = tmp;
				break;
			}
		}
	}

	return pi;
}

mpeg2ts_stream_t *mpeg2ts_stream_new() {
	mpeg2ts_stream_t *m2s = calloc(1, sizeof(mpeg2ts_stream_t));
	m2s->programs = vqarray_new();
	return m2s;
}

void mpeg2ts_stream_free(mpeg2ts_stream_t *m2s) {
	if (m2s == NULL)
		return;
	if (m2s->programs != NULL) {
		vqarray_foreach(m2s->programs, mpeg2ts_program_free);
		vqarray_free(m2s->programs);
	}
	if (m2s->pat != NULL) {
		program_association_section_free(m2s->pat);
	}
	free(m2s);
}

// TODO: when we'll be doing implied PCR calculations, we'll need to add estimation...

int mpeg2ts_stream_read_ts_packet(mpeg2ts_stream_t *m2s, ts_packet_t *ts) {
	int ret = 0;

	if (m2s == NULL || ts == NULL)
		return 0;
	if ((m2s->pat == NULL) && (ts->header.PID != PAT_PID))
		return 0; // we don't have any PSI, nothing we can do.

	pid_info_t *pi = NULL;
	for (int i = 0; i < vqarray_length(m2s->programs) && (pi == NULL); i++) {
		mpeg2ts_program_t *m2p = vqarray_get(m2s->programs, i);
		if (m2p == NULL)
			continue;

		if (m2p->PID == ts->header.PID) { // got a PMT
			// FIXME: this hack works in 90% of cases
			// however, it won't work when PAT is
			// in 2+ packets. we need generic section reader for this

			program_map_section_t *pms = program_map_section_new();
			program_map_section_read(pms, ts->payload.bytes + 1, ts->payload.len - 1);
			if (pms->program_number != m2p->program_number) {
				LOG_ERROR_ARGS("Invalid program number in PMT: expecting %d, encountered %d",
					m2p->program_number, pms->program_number);
				SAFE_REPORT_TS_ERR(-70);
				return -1;
			}

			if (m2p->pmt != NULL) {
				// FIXME: compare sections, drop the older one
				if ( pms->version_number != m2p->pmt->version_number )
					LOG_WARN("Changing PMT section versions not supported yet");
				program_map_section_free(pms);

				continue;
			} else {
				m2p->pmt = pms;
				for (int es_idx=0; es_idx < vqarray_length(m2p->pmt->es_info) ; es_idx++) {
					elementary_stream_info_t *es = vqarray_get(m2p->pmt->es_info, es_idx);
					if (es == NULL)
						continue;
					pi = pid_info_new();
					pi->es_info = es;
					// FIXME add default stream handlers via stream type
					vqarray_add(m2p->pids, pi);
					pi = NULL;
				}
			}
		} else { // same elementary PID may be owned by multiple programs
			pi = mpeg2ts_program_get_pid_info(m2p, ts->header.PID);
			if ((pi != NULL) && (pi->demux_handler != NULL) && (pi->demux_handler->process_ts_packet != NULL)) {
				ret = pi->demux_handler->process_ts_packet(ts, pi->es_info, pi->demux_handler->arg);
				if (ret != 0)
					return ret;
			}
		}
	}

	return ret;
}
