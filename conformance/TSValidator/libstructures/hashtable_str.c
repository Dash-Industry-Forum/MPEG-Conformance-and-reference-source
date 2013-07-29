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

#include <stdint.h>
#include <string.h>

#include "hashtable_str.h"
#ifdef HAVE_BSTRING
#include "bstrlib.h"
#endif

// string hash function by Prof. Daniel J. Bernstein, comp.lang.c, Oct 1990
// also seen in perl and bdb
uint32_t hashfn_uint8_djb(uint8_t* s, int len)
{
    uint32_t hash = 5381;
    int i    = 0;
    
    for(i = 0; i < len; i++)
    {
        hash = ((hash << 5) + hash) + s[i];
    }
    
    return (hash & 0x7FFFFFFF);
}

// 32bit integer hash function by Robert Jenkins
uint32_t hashfn_uint32_rj(uint32_t key)
{
    key += (key << 12);
    key ^= (key >> 22);
    key += (key << 4);
    key ^= (key >> 9);
    key += (key << 10);
    key ^= (key >> 2);
    key += (key << 7);
    key ^= (key >> 12);
    return key;
}

// 32bit integer hash function by Thomas Wang (http://www.concentric.net/~Ttwang/tech/inthash.htm)
int32_t hashfn_int32_tw(int32_t key)
{
    key += ~(key << 15);
    key ^=  (key >> 10);
    key +=  (key << 3);
    key ^=  (key >> 6);
    key += ~(key << 11);
    key ^=  (key >> 16);
    return key;
}

uint32_t hashtable_hashfn_char(void* k)
{
    char* s = (char*) k;
    int len  = strlen((char*)s);
    return hashfn_uint8_djb((uint8_t*)s, len);
}

#ifdef HAVE_BSTRING
uint32_t hashtable_hashfn_bstring(void* k)
{
    bstring b = (bstring) k;
    char* s = (char*)bdata(b);
    int len  = blength(b);
    return hashfn_uint8_djb((uint8_t*)s, len);
}
#endif

uint32_t hashtable_hashfn_int32(void* k)
{
    int32_t i = *((int32_t*)k);
    return hashfn_int32_tw(i);
}

uint32_t hashtable_hashfn_uint32(void* k)
{
    uint32_t i = *((uint32_t*)k);
    return hashfn_uint32_rj(i);
}

uint32_t hashtable_hashfn_int64(void* k)
{
    return hashfn_uint8_djb( k, sizeof(int64_t) );
}

uint32_t hashtable_hashfn_uint64(void* k)
{
    return hashfn_uint8_djb( k, sizeof(uint64_t) );
}



int hashtable_eqfn_char(void* k1, void* k2)
{
    char* s1 = (char*) k1;
    char* s2 = (char*) k2;
    if (s1 == s2){ return 1; }
    else if (s1 == NULL || s2 == NULL) { return 0; }
    else { return (strcmp(s1, s2) == 0 ? 1 : 0); }
}

#ifdef HAVE_BSTRING
int hashtable_eqfn_bstring(void* k1, void* k2)
{
    bstring b1 = (bstring) k1;
    bstring b2 = (bstring) k2;
    if (b1 == b2){ return 1; }
    else if (b1 == NULL || b2 == NULL) { return 0; }
    else { return biseq(b1, b2); }
}
#endif

int hashtable_eqfn_int32(void* k1, void* k2)
{
    int32_t i1 = *((int32_t*)k1);
    int32_t i2 = *((int32_t*)k2);
    return (i1 == i2);
}

int hashtable_eqfn_uint32(void* k1, void* k2)
{
    uint32_t i1 = *((uint32_t*)k1);
    uint32_t i2 = *((uint32_t*)k2);
    return (i1 == i2);
}
int hashtable_eqfn_int64(void* k1, void* k2)
{
    int64_t i1 = *((int64_t*)k1);
    int64_t i2 = *((int64_t*)k2);
    return (i1 == i2);
}

int hashtable_eqfn_uint64(void* k1, void* k2)
{
    uint64_t i1 = *((uint64_t*)k1);
    uint64_t i2 = *((uint64_t*)k2);
    return (i1 == i2);
}


