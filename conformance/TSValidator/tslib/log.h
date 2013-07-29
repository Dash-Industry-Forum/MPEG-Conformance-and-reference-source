/* 
 * Copyright (C) 2012-   Alex Giladi
 * 
 * Written by Alex Giladi <alex.giladi@gmail.com>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef _STREAMKIT_LOG_H_
#define _STREAMKIT_LOG_H_

#ifdef __cplusplus
extern "C" {
#endif

// we need those for inttypes.h in C++
#if defined __cplusplus && !defined __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS 1
#endif
#include <inttypes.h>

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#define SKIT_LOG_TYPE_UINT			0x01
#define SKIT_LOG_TYPE_UINT_HEX		0x02
#define SKIT_LOG_TYPE_STR			0x03

#define SKIT_LOG_TYPE_UINT_DBG		0x04
#define SKIT_LOG_TYPE_UINT_HEX_DBG	0x05
#define SKIT_LOG_TYPE_STR_DBG		0x06

#define SKIT_LOG_UINT32_DBG(str, prefix, arg, n)  fprintf(stdout, "DEBUG: %s%s=%"PRIu32"\n", prefix, #arg, (arg));
#define SKIT_LOG_UINT32_HEX_DBG(str, prefix, arg, n)  fprintf(stdout, "DEBUG: %s%s=%"PRIX32"\n", prefix, #arg, (arg));
#define SKIT_LOG_UINT64_DBG(str, prefix, arg, n)  fprintf(stdout, "DEBUG: %s%s=%"PRIu64"\n", prefix, #arg, (arg));

#define SKIT_LOG_UINT(str, level, arg, n) skit_log_struct((level), #arg,  (arg), SKIT_LOG_TYPE_UINT, NULL);
#define SKIT_LOG_UINT_DBG(str, level, arg, n) skit_log_struct((level), #arg,  (arg), SKIT_LOG_TYPE_UINT_DBG, NULL);
#define SKIT_LOG_UINT_HEX(str, level, arg, n) skit_log_struct((level), #arg,  (arg), SKIT_LOG_TYPE_UINT_HEX, NULL);
#define SKIT_LOG_UINT_HEX_DBG(str, level, arg, n) skit_log_struct((level), #arg,  (arg), SKIT_LOG_TYPE_UINT_HEX_DBG, NULL);
#define SKIT_LOG_UINT_VERBOSE(str, level, arg, explain, n) skit_log_struct((level), #arg,  (arg), SKIT_LOG_TYPE_UINT, explain);
#define SKIT_LOG_STR(str, level, arg, n) skit_log_struct((level), #arg, arg, SKIT_LOG_TYPE_STR, NULL);
#define SKIT_LOG_STR_DBG(str, level, arg, n) skit_log_struct((level), #arg, arg, SKIT_LOG_TYPE_STR_DBG, NULL);
int skit_log_struct(int level, char *name, uint64_t value, int type, char *str);

// More traditional debug logging
// tslib-global loglevel: error > warn (default) > info > debug
#define TSLIB_LOG_LEVEL_ERROR		1
#define TSLIB_LOG_LEVEL_WARN		((TSLIB_LOG_LEVEL_ERROR) + 1)
#define TSLIB_LOG_LEVEL_INFO		((TSLIB_LOG_LEVEL_ERROR) + 2)
#define TSLIB_LOG_LEVEL_DEBUG		((TSLIB_LOG_LEVEL_ERROR) + 3)

#define TSLIB_LOG_LEVEL_DEFAULT		TSLIB_LOG_LEVEL_WARN

extern int tslib_loglevel;

#define LOG_ERROR(msg)				{ if (tslib_loglevel >= TSLIB_LOG_LEVEL_ERROR) \
										fprintf(stderr, "ERROR: %s\t\t[%s() @ %s:%d]\n", msg, __FUNCTION__, __FILE__, __LINE__); }
#define LOG_ERROR_ARGS(format, ...)	{ if (tslib_loglevel >= TSLIB_LOG_LEVEL_ERROR) { \
										fprintf(stderr, "ERROR: "); \
										fprintf(stderr, format, __VA_ARGS__); \
										fprintf(stderr, "\t\t[%s() @ %s:%d]\n", __FUNCTION__, __FILE__, __LINE__); } \
									}

#define LOG_WARN(msg)				{	if (tslib_loglevel >= TSLIB_LOG_LEVEL_DEBUG) \
											fprintf(stdout, "WARNING: %s\t\t[%s() @ %s:%d]\n", msg, __FUNCTION__, __FILE__, __LINE__); \
										else if (tslib_loglevel >= TSLIB_LOG_LEVEL_WARN) \
											fprintf(stdout, "WARNING: %s\n", msg); \
									}
#define LOG_WARN_ARGS(format, ...)	{ if (tslib_loglevel >= TSLIB_LOG_LEVEL_WARN) { \
										fprintf(stdout, "WARNING: "); \
										fprintf(stdout, format, __VA_ARGS__); \
										if (tslib_loglevel >= TSLIB_LOG_LEVEL_DEBUG) \
											fprintf(stdout, "\t\t[%s() @ %s:%d]\n", __FUNCTION__, __FILE__, __LINE__); \
										else \
											fprintf(stdout, "\n"); \
									}}

#define LOG_INFO(msg)				{ if (tslib_loglevel >= TSLIB_LOG_LEVEL_INFO) \
										fprintf(stdout, "INFO: %s\n", msg); }
#define LOG_INFO_ARGS(format, ...)	{ if (tslib_loglevel >= TSLIB_LOG_LEVEL_INFO) { \
										fprintf(stdout, "INFO: "); \
										fprintf(stdout, format, __VA_ARGS__); \
										fprintf(stdout, "\n"); } \
									}

#define LOG_DEBUG(msg)				{ if (tslib_loglevel >= TSLIB_LOG_LEVEL_DEBUG) \
										fprintf(stdout, "DEBUG: %s\t\t[%s() @ %s:%d]\n", msg, __FUNCTION__, __FILE__, __LINE__); }
#define LOG_DEBUG_ARGS(format, ...)	{ if (tslib_loglevel >= TSLIB_LOG_LEVEL_DEBUG) { \
										fprintf(stdout, "DEBUG: "); \
										fprintf(stdout, format, __VA_ARGS__); \
										fprintf(stdout, "\t\t[%s() @ %s:%d]\n", __FUNCTION__, __FILE__, __LINE__); } \
									}

#ifdef __cplusplus
}
#endif

#endif // _STREAMKIT_LOG_H_
