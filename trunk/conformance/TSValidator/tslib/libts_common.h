/*
 Copyright (c) 2012, Alex Giladi <alex.giladi@gmail.com>
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

#ifndef _TSLIB_TSCOMMON_H_
#define _TSLIB_TSCOMMON_H_        

#include <stdint.h>

#include "bs.h"
#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline uint64_t bs_read_90khz_timestamp(bs_t *b) {
	uint64_t v = (uint64_t) bs_read_u(b, 3) << 30;
	bs_skip_u1(b);
	v |= (uint64_t) bs_read_u(b, 15) << 15;
	bs_skip_u1(b);
	v |= (uint64_t) bs_read_u(b, 15);
	bs_skip_u1(b);

	return v;
}

static inline void bs_write_90khz_timestamp(bs_t *b, uint64_t v) {
	bs_write_u(b, 3, v >> 30);
	bs_write_u1(b, 1);
	bs_write_u(b, 15, v >> 15);
	bs_write_u1(b, 1);
	bs_write_u(b, 15, v);
	bs_write_u1(b, 1);
}

#define bs_write_reserved(b,n) bs_write_ones(b, (n))

static inline void bs_write_ones(bs_t *b, int n) {
	bs_write_u(b, n, 0x7FFFFFFF);
}

static inline void bs_write_marker_bit(bs_t *b) {
	bs_write_u1(b, 1);
}

// Dirty hack: our own errno, error reporting via a global var. Initialized to zero on start.
extern volatile int tslib_errno;
// This macro is NOT thread-safe
#define SAFE_REPORT_TS_ERR(errCode)		if (tslib_errno == 0) tslib_errno = (errCode)

#ifdef __cplusplus
}
#endif

#endif // _TSLIB_TSCOMMON_H_
