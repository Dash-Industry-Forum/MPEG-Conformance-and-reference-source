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

#ifndef VQARRAY_INCLUDE
#define VQARRAY_INCLUDE

#include <string.h>
#include <stdlib.h>

#ifndef _max
#define _max(a, b) ( (a) > (b) ? (a) : (b) )
#endif

typedef void vqarray_elem_t;

typedef struct
{
    vqarray_elem_t** array;
    int start;
    int length;
    int memlength;
} vqarray_t;

typedef struct
{
    vqarray_t* v;
    int i;
} vqarray_iterator_t;

typedef void (*vqarray_functor_t)(void*);
typedef void (*vqarray_functor2_t)(void*, void*);

vqarray_t* vqarray_new();
vqarray_t* vqarray_init(vqarray_t* v, int length );
void vqarray_free_buf(vqarray_t* v);
void vqarray_free(vqarray_t* v);

void vqarray_insert(vqarray_t* v, int i, vqarray_elem_t* e);
void vqarray_remove(vqarray_t* v, int i);

vqarray_t* vqarray_get_range(vqarray_t* v, int start, int len);
void vqarray_set_range(vqarray_t* v, int start, int len, vqarray_t* v2);
void vqarray_insert_range(vqarray_t* v, int start, int len, vqarray_t* v2);
void vqarray_remove_range(vqarray_t* v, int start, int len);
void vqarray_add_all(vqarray_t* v, vqarray_t* v2);
vqarray_t* vqarray_new_from_array(vqarray_elem_t* a, int len);
vqarray_t* vqarray_new_reserve(int len);
vqarray_elem_t** vqarray_to_array(vqarray_t* v);
void vqarray_add_array(vqarray_t* v, vqarray_elem_t** a, int len);
int vqarray_index_of(vqarray_t* v, vqarray_elem_t* e, int (*cmp_func) (vqarray_elem_t* e1, vqarray_elem_t* e2));
int vqarray_last_index_of(vqarray_t* v, vqarray_elem_t* e, int (*cmp_func) (vqarray_elem_t* e1, vqarray_elem_t* e2));
void vqarray_sort(vqarray_t* v, int (*cmp_func) (vqarray_elem_t* e1, vqarray_elem_t* e2) );
int vqarray_binary_search(vqarray_t* v, vqarray_elem_t* e, int (*cmp_func) (vqarray_elem_t* e1, vqarray_elem_t* e2) );
int vqarray_binary_search_by_key(vqarray_t* v, void* key, int (*cmp_func) (void* key, vqarray_elem_t* e2) );
vqarray_t* vqarray_grep(vqarray_t* v, int (*grep_func) (vqarray_elem_t* e) );
vqarray_t* vqarray_map(vqarray_t* v, vqarray_elem_t* (*map_func) (vqarray_elem_t* e) );
vqarray_iterator_t* vqarray_iterator_new(vqarray_t* v);
void vqarray_iterator_free(vqarray_iterator_t* iter);
vqarray_elem_t* vqarray_iterator_next(vqarray_iterator_t* iter);
vqarray_elem_t* vqarray_iterator_previous(vqarray_iterator_t* iter);
int vqarray_iterator_has_next(vqarray_iterator_t* iter);
int vqarray_iterator_has_previous(vqarray_iterator_t* iter);
void _vqarray_expand_to_length(vqarray_t* v, int length);

// apply functors to vqarray elements
void vqarray_foreach(vqarray_t* v, vqarray_functor_t func );
void vqarray_foreach2(vqarray_t* v, vqarray_functor2_t func, void* arg);

static void vqarray_clear(vqarray_t* v);
static int vqarray_length(const vqarray_t* v);
static vqarray_elem_t* vqarray_get(const vqarray_t* v, int i);
static void vqarray_set(vqarray_t* v, int i, vqarray_elem_t* e);
static void vqarray_add(vqarray_t* v, vqarray_elem_t* e);
static void vqarray_push(vqarray_t* v, vqarray_elem_t* e);
static vqarray_elem_t* vqarray_pop(vqarray_t* v);
static void vqarray_unshift(vqarray_t* v, vqarray_elem_t* e); 
static vqarray_elem_t* vqarray_shift(vqarray_t* v);
static vqarray_t* vqarray_copy(vqarray_t* v);
static int vqarray_insert_sorted(vqarray_t* v, vqarray_elem_t* e, int (*cmp_func) (vqarray_elem_t* e1, vqarray_elem_t* e2));

// IMPLEMENTATION

/**
   Empty array.  The elements are not deallocated.
   @param v the array
 */
static inline void vqarray_clear(vqarray_t* v) { v->length = 0;}

/**
   Returns length of array.
   @param v the array
 */
static inline int vqarray_length(const vqarray_t* v) { return v->length;}

/**
   Get the element at the given index in the array.
   If the index is out of bounds returns NULL.
   Note that an array may also contain a NULL element, so this is not an unambiguous indication of an out-of-bounds.
   @param v the array
   @param i the index
 */
static inline vqarray_elem_t* vqarray_get(const vqarray_t* v, int i)
{
    if (i < 0 || i >= v->length)
    {
        return NULL;
    }
    return v->array[i + v->start];
}

/**
   Set the element at the given index in the array.
   If the index is out of bounds, the array is grown to that size.  The values of any intermediate elements created by that will be NULL.
   @param v the array
   @param i the index
   @param e the element
*/
static inline void vqarray_set(vqarray_t* v, int i, vqarray_elem_t* e)
{
    if (i < 0)
    {
        return;
    }
    _vqarray_expand_to_length(v, i+1);
    v->array[i + v->start] = e;
    v->length = _max(v->length, i+1);
}

/* *** All operations below this are implemented in terms of get, set, insert and remove *** */

/**
   Add the element to the end of the array.
   @param v the array
   @param e the element
*/
static inline void vqarray_add(vqarray_t* v, vqarray_elem_t* e) { vqarray_set(v, vqarray_length(v), e);}

/**
   Add the element to the end of the array.
   @param v the array
   @param e the element
*/
static inline void vqarray_push(vqarray_t* v, vqarray_elem_t* e) { vqarray_insert(v, vqarray_length(v), e);}

/**
   Remove and return the element at the end of the array.
   If the array is empty, returns NULL.
   @param v the array
   @return the element
*/
static inline vqarray_elem_t* vqarray_pop(vqarray_t* v) { vqarray_elem_t* e = vqarray_get(v, vqarray_length(v)-1); vqarray_remove(v, vqarray_length(v)-1); return e;}

/**
   Remove and return the element at the end of the array.
   If the array is empty, returns NULL.
   @param v the array
   @return the element
*/
static inline vqarray_elem_t* vqarray_top(vqarray_t* v) { return vqarray_get(v, vqarray_length(v)-1);}

/**
   Add the element to the beginning of the array.
   @param v the array
   @param e the element
*/
static inline void vqarray_unshift(vqarray_t* v, vqarray_elem_t* e) { vqarray_insert(v, 0, e);}

/**
   Remove and return the element at the beginning of the array.
   If the array is empty, returns NULL.
   @param v the array
   @return the element
*/
static inline vqarray_elem_t* vqarray_shift(vqarray_t* v) { vqarray_elem_t* e = vqarray_get(v, 0); vqarray_remove(v, 0); return e;}

/**
   Get a copy of the array.
   The copy is a newly allocated array.
   @param v the array
   @return the copy
*/
static inline vqarray_t* vqarray_copy(vqarray_t* v) { return vqarray_get_range(v, 0, vqarray_length(v));}

/**
   Add the element to a sorted array.
   @param[in] v the array
   @param[in] e the element
   @param[in] cmp_func  function to compare elements with
   @return index of e in the array
*/
static inline int vqarray_insert_sorted(
                                       vqarray_t* v, vqarray_elem_t* e, 
                                       int (*cmp_func) (vqarray_elem_t* e1, vqarray_elem_t* e2))
{
    int idx = vqarray_binary_search( v, e, cmp_func ) + 1;
    vqarray_insert( v, idx, e );
    return idx;    
}

#endif
