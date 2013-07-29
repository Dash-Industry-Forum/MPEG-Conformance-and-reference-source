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

#include "binheap.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h> // for printf
#include <assert.h>

const int binheap_start_length = 16;
const int binheap_block_length = 1024;

void _binheap_sift_up(binheap_t* bh, int idx);
void _binheap_sift_down(binheap_t* bh, int idx);
void _binheap_expand_to_length(binheap_t* bh, int length);

binheap_t* binheap_new(binheap_cmp_func_t cmp_func)
{
    binheap_t* bh = (binheap_t*)malloc(sizeof(binheap_t));
    bh->cmp_func = cmp_func;
    bh->array = NULL;
    bh->memlength = 0;
    bh->length = 0;
    _binheap_expand_to_length(bh, binheap_start_length);
    return bh;
}

void binheap_free(binheap_t* bh)
{
    free(bh->array);
    bh->memlength = 0;
    bh->length = 0;
    bh->cmp_func = NULL;
    free(bh);
}

void binheap_insert(binheap_t* bh, binheap_elem_t* e)
{
    _binheap_expand_to_length(bh, bh->length+1);
    bh->length ++;
    bh->array[ bh->length-1 ] = e;
    _binheap_sift_up(bh, bh->length-1);
}

binheap_elem_t* binheap_remove_first(binheap_t* bh)
{
    if (bh->length == 0) { return NULL; }
    binheap_elem_t* first_elem = bh->array[0];
    bh->array[0] = bh->array[ bh->length-1 ];
    bh->length --;
    if (bh->length > 0)
    {
        _binheap_sift_down(bh, 0);
    }
    return first_elem;
}

// TODO call this map or apply or foreach or...?
void binheap_foreach(binheap_t* bh, void (*func) (binheap_elem_t* e))
{
   int i;
   if (bh->length == 0) return; 

   for ( i = 0; i < bh->length; i++ )
   {
       func( bh->array[i] );
   }
}

void binheap_foreach2(binheap_t* bh, void (*func) (binheap_elem_t* e, void* arg), void* arg)
{
   int i;
   if (bh->length == 0) return;

   for ( i = 0; i < bh->length; i++ )
   {
       func( bh->array[i], arg );
   }
}

binheap_elem_t** binheap_get_all(binheap_t* bh, int *len)
{
    binheap_elem_t** elems = (binheap_elem_t**)malloc( bh->length * sizeof(binheap_elem_t*));
    memcpy(elems, bh->array, bh->length * sizeof(binheap_elem_t*));
    *len = bh->length;
    return elems;
}

inline int _binheap_parent_idx(binheap_t* bh, int idx)
{
    if (idx <= 0) { return -1; }
    return (idx-1)/2;
}

inline int _binheap_lchild_idx(binheap_t* bh, int idx)
{
    int lchild_idx = (2*idx + 1);
    if (lchild_idx >= bh->length) { return -1; }
    return lchild_idx;
}

inline int _binheap_rchild_idx(binheap_t* bh, int idx)
{
    int rchild_idx = (2*idx + 2);
    if (rchild_idx >= bh->length) { return -1; }
    return rchild_idx;
}

inline void _binheap_swap(binheap_t*bh, int idx1, int idx2)
{
    binheap_elem_t* tmp = bh->array[idx1];
    bh->array[idx1] = bh->array[idx2];
    bh->array[idx2] = tmp;
}

void _binheap_sift_up(binheap_t* bh, int idx)
{
    while(1)
    {
        int parent_idx = _binheap_parent_idx(bh, idx);
        if (parent_idx == -1)
        {
            break;
        }
        if (bh->cmp_func(bh->array[parent_idx], bh->array[idx]) <= 0)
        {
            break;
        }

        _binheap_swap(bh, parent_idx, idx);
        idx = parent_idx;
    }
}

void _binheap_sift_down(binheap_t* bh, int idx)
{
    while(1)
    {
        int lchild_idx = _binheap_lchild_idx(bh, idx);
        int rchild_idx = _binheap_rchild_idx(bh, idx);

        int child_idx;
        if ( lchild_idx != -1 && rchild_idx != -1 )
        {
            if (bh->cmp_func(bh->array[lchild_idx], bh->array[rchild_idx]) <= 0) { child_idx = lchild_idx; }
            else { child_idx = rchild_idx; }
        }
        else if ( rchild_idx != -1 )
        {
            child_idx = rchild_idx; // this should not be possible since lchild would also exist
        }
        else if ( lchild_idx != -1 )
        {
            child_idx = lchild_idx;
        }
        else
        {
            break;
        }

        if (bh->cmp_func(bh->array[idx], bh->array[child_idx]) <= 0)
        {
            break;
        }

        _binheap_swap(bh, idx, child_idx);
        idx = child_idx;
    }
}


// TODO from varray.c - any way to merge the two?
void _binheap_expand_to_length(binheap_t* bh, int length)
{
    if (bh->memlength >= length) { return; }

    int memlength;
    if (length <= binheap_start_length)
    {
        memlength = binheap_start_length;
    }
    else if (length < binheap_block_length && length < bh->memlength * 2)
    {
        memlength = bh->memlength * 2;
    }
    else
    {
        memlength = binheap_block_length * (1 + ((length + 1) / binheap_block_length));
    }

    bh->array = (binheap_elem_t**)realloc(bh->array, memlength*sizeof(binheap_elem_t*));

    bh->memlength = memlength;
}


void _binheap_invariants(binheap_t* bh)
{
    assert( bh != NULL );
    assert( bh->length >= 0);
    assert( bh->memlength >= bh->length );
    int idx;
    for (idx = 0; idx < bh->length; idx++)
    {
        int lchild_idx = _binheap_lchild_idx(bh, idx);
        int rchild_idx = _binheap_rchild_idx(bh, idx);
        if (lchild_idx != -1)
        {
            assert( lchild_idx >= 0 && lchild_idx < bh->length );
            assert( bh->cmp_func( bh->array[idx], bh->array[lchild_idx] ) <= 0 );
        }
        if (rchild_idx != -1)
        {
            assert( rchild_idx >= 0 && rchild_idx < bh->length );
            assert( bh->cmp_func( bh->array[idx], bh->array[rchild_idx] ) <= 0 );
        }
    }
}

void _binheap_dump(binheap_t* bh)
{
    printf("digraph G {\n");
    int idx;
    for (idx = 0; idx < bh->length; idx++)
    {
        int lchild_idx = _binheap_lchild_idx(bh, idx);
        int rchild_idx = _binheap_rchild_idx(bh, idx);
        if (lchild_idx != -1)
        {
            printf("    %s -> %s;\n", (char*)(bh->array[idx]),  (char*)(bh->array[lchild_idx]) );
        }
        if (rchild_idx != -1)
        {
            printf("    %s -> %s;\n", (char*)(bh->array[idx]),  (char*)(bh->array[rchild_idx]) );
        }
    }
    printf("}\n");
}
