/* 
 * libstructures - a library for generic data structures in C
 * Copyright (C) 2005-2008 Avail Media, Inc.
 * 
 * Written by Alex Izvorski <aizvorski@gmail.com>
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

#ifndef HASHTABLE_STR_INCLUDE
#define HASHTABLE_STR_INCLUDE

#include <stdint.h>

uint32_t hashtable_hashfn_char(void* k);
uint32_t hashtable_hashfn_int32(void* k);
uint32_t hashtable_hashfn_uint32(void* k);
uint32_t hashtable_hashfn_int64(void* k);
uint32_t hashtable_hashfn_uint64(void* k);

int hashtable_eqfn_char(void* k1,void* k2);
int hashtable_eqfn_int32(void* k1,void* k2);
int hashtable_eqfn_int32(void* k1,void* k2);
int hashtable_eqfn_uint32(void* k1, void* k2);
int hashtable_eqfn_uint64(void* k1, void* k2);

#ifdef HAVE_BSTRING
int hashtable_eqfn_bstring(void* k1,void* k2);
uint32_t hashtable_hashfn_bstring(void* k);
#endif

uint32_t hashfn_uint8_djb(uint8_t* s, int len);
uint32_t hashfn_uint32_rj(uint32_t key);
int32_t hashfn_int32_tw(int32_t key);


#endif
