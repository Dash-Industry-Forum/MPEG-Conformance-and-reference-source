/* 
 * Copyright (C) 2012-     Alex Giladi
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

#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>

#include "log.h"

int tslib_loglevel = TSLIB_LOG_LEVEL_DEFAULT;

#define INDENT_LEVEL	4
#define PREFIX_BUF_LEN	0x80

int skit_log_struct(int num_indents, char *name, uint64_t value, int type, char *str) {
	if (name == NULL)
		return 0;

	// get rid of prefixes
	char *last_dot = strrchr(name, '.');
	char *last_arrow = strrchr(name, '>');
	char *real_name = NULL;

	if (last_dot == NULL)
		real_name = (last_arrow != NULL) ? last_arrow + 1 : name;
	else {
		if (last_arrow != NULL)
			real_name = (last_dot > last_arrow) ? last_dot : last_arrow;
		else
			real_name = last_dot;
		real_name++;
	}

	int nbytes = 0, indent_len = INDENT_LEVEL * num_indents;
	if (indent_len >= PREFIX_BUF_LEN) {
		LOG_ERROR_ARGS("Too many indents - %d", num_indents);
		return 0;
	}
	char prefix[PREFIX_BUF_LEN];
	memset(prefix, ' ', indent_len);
	prefix[indent_len] = 0;

	switch (type) {
	case SKIT_LOG_TYPE_UINT:
		nbytes += fprintf(stdout, "INFO: %s%s=%"PRId64"", prefix, real_name, (uint64_t) value);
		break;
	case SKIT_LOG_TYPE_UINT_DBG:
		nbytes += fprintf(stdout, "DEBUG: %s%s=%"PRId64"", prefix, real_name, (uint64_t) value);
		break;
	case SKIT_LOG_TYPE_UINT_HEX:
		nbytes += fprintf(stdout, "INFO: %s%s=0x%"PRIX64"", prefix, real_name, (uint64_t) value);
		break;
	case SKIT_LOG_TYPE_UINT_HEX_DBG:
		nbytes += fprintf(stdout, "DEBUG: %s%s=0x%"PRIX64"", prefix, real_name, (uint64_t) value);
		break;
	case SKIT_LOG_TYPE_STR:
		nbytes += fprintf(stdout, "INFO: %s%s=%s", prefix, real_name, (char*) value);
		break;
	case SKIT_LOG_TYPE_STR_DBG:
		nbytes += fprintf(stdout, "INFO: %s%s=%s", prefix, real_name, (char*) value);
		break;
	default:
		break;
	}

	if (str)
		nbytes += fprintf(stdout, " (%s)\n", str);
	else
		nbytes += fprintf(stdout, "\n");

	// TODO logging tasks:
	//  - additional targets (file, string)
	//  - additional log formats -- e.g. XML or/and JSON ???
	//  - separate handling strings....
	return nbytes;
}
