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

#ifndef BINHEAP_INCLUDE
#define BINHEAP_INCLUDE

#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef void binheap_elem_t;
typedef int (*binheap_cmp_func_t) (binheap_elem_t* e1, binheap_elem_t* e2);

typedef struct
{
    binheap_elem_t** array;
    int length;
    int memlength;
    binheap_cmp_func_t cmp_func;
} binheap_t;

binheap_t* binheap_new(binheap_cmp_func_t cmp_func);
void binheap_free(binheap_t* bh);
void binheap_insert(binheap_t* bh, binheap_elem_t* e);

binheap_elem_t* binheap_remove_first(binheap_t* bh);
void binheap_foreach(binheap_t* bh, void (*func) (binheap_elem_t* e));
void binheap_foreach2(binheap_t* bh, void (*func) (binheap_elem_t* e, void* arg), void* arg);
binheap_elem_t** binheap_get_all(binheap_t* bh, int *len);

void _binheap_dump(binheap_t* bh);

static inline binheap_elem_t* binheap_get_first(binheap_t* bh)
{
    if (bh->length == 0) { return NULL; }
    return bh->array[0];
}

static inline int binheap_size(binheap_t* bh)
{
    return bh->length;
}

static inline binheap_elem_t** binheap_get_all_ref(binheap_t* bh, int *len)
{
    *len = bh->length;
    return bh->array;
}

#endif
