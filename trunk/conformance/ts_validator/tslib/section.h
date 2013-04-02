/* 
 * Copyright (C) 2012-   Alex Giladi
 * Copyright (c) 2013- Alex Giladi <alex.giladi@gmail.com> and Vlad Zbarsky <zbarsky@cornell.edu>
 *
 * Written by Alex Giladi <alex.giladi@gmail.com> and Vlad Zbarsky <zbarsky@cornell.edu>
 * 
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


#ifndef _TSLIB_SECTION_H_
#define _TSLIB_SECTION_H_        

/*
#include <stdint.h>

#include "bs.h"
#include "common.h"
*/

typedef enum {
	program_association_section = 0,
	conditional_access_section,			// CA section
	TS_program_map_section,
	TS_description_section,
	ISO_IEC_14496_scene_description_section,
	ISO_IEC_14496_object_descriptor_section,
	Metadata_section,
	IPMP_Control_Information_section,	// (defined in ISO/IEC 13818-11)
//0x08-0x3F ISO/IEC 13818-1 reserved
//0x40-0xFE User private
	Forbidden = 0xFF
} table_id_t;

#define	MAX_SECTION_LEN			0x03FD

#define MAX_PROGRAM_INFO_LEN	0x03FF
#define MAX_ES_INFO_LEN			0x03FF

// TODO generic section reader

#endif // _TSLIB_SECTION_H_

