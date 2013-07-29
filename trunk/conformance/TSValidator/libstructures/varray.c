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

#include <string.h>
#include <stdlib.h>

#include "varray.h"

const int varray_start_length = 16;
const int varray_block_length = 1024;

void _varray_expand_to_length(varray_t* v, int length);

#define _max(a, b) ( (a) > (b) ? (a) : (b) )

/**
   Create new array.  The array is initially empty.
   @return  the new array
 */
varray_t* varray_new()
{
    varray_t* v = (varray_t*)malloc(sizeof(varray_t));
    v->array = NULL;
    v->memlength = 0;
    v->length = 0;
    _varray_expand_to_length(v, varray_start_length);
    return v;
}

/**
   Free array.  The array must not be used after this.
   @param v the array
 */
void varray_free(varray_t* v)
{
    v->length = 0;
    v->memlength = 0;
    free(v->array);
    v->array = NULL;
    free(v);
}

/**
   Empty array.  The elements are not deallocated.
   @param v the array
 */
void varray_clear(varray_t* v) { v->length = 0; }

/**
   Returns length of array.
   @param v the array
 */
inline int varray_length(varray_t* v) { return v->length; }

/**
   Get the element at the given index in the array.
   If the index is out of bounds returns NULL.
   Note that an array may also contain a NULL element, so this is not an unambiguous indication of an out-of-bounds.
   @param v the array
   @param i the index
 */
inline varray_elem_t* varray_get(varray_t* v, int i)
{
    if (i < 0 || i >= v->length) { return NULL; }
    return v->array[i];
}

/**
   Set the element at the given index in the array.
   If the index is out of bounds, the array is grown to that size.  The values of any intermediate elements created by that will be NULL.
   @param v the array
   @param i the index
   @param e the element
*/
inline void varray_set(varray_t* v, int i, varray_elem_t* e)
{
    if (i < 0) { return; }
    _varray_expand_to_length(v, i+1);
    v->array[i] = e;
    v->length = _max(v->length, i+1);
}

/**
   Insert the element at the given index in the array.
   Any subsequent elements are moved up.
   If the index is out of bounds, the array is grown to that size.  The values of any intermediate elements created by that will be NULL.
   @param v the array
   @param i the index
   @param e the element
*/
void varray_insert(varray_t* v, int i, varray_elem_t* e)
{
    if (i < 0) { return; }
    _varray_expand_to_length(v, _max(v->length+1, i+1));
    if (i < v->length) { memmove(&(v->array[i+1]), &(v->array[i]), (v->length-i) * sizeof(varray_elem_t*)); }
    v->array[i] = e;
    v->length = _max(v->length+1, i+1);
}

/**
   Remove the element at the given index in the array.
   Any subsequent elements are moved down.
   If the index is out of bounds, does nothing.
   The element itself is not deallocated.
   @param v the array
   @param i the index
*/
void varray_remove(varray_t* v, int i)
{
    if (i < 0 || i >= v->length) { return; }
    if (i+1 < v->length) { memmove(&(v->array[i]), &(v->array[i+1]), (v->length-(i+1)) * sizeof(varray_elem_t*)); }
    if (v->length > 0) { v->length --; }
}

/* *** All operations below this are implemented in terms of get, set, insert and remove *** */

/**
   Add the element to the end of the array.
   @param v the array
   @param e the element
*/
void varray_add(varray_t* v, varray_elem_t* e) { varray_set(v, varray_length(v), e); }

/**
   Add the element to the end of the array.
   @param v the array
   @param e the element
*/
void varray_push(varray_t* v, varray_elem_t* e) { varray_insert(v, varray_length(v), e); }

/**
   Remove and return the element at the end of the array.
   If the array is empty, returns NULL.
   @param v the array
   @return the element
*/
varray_elem_t* varray_pop(varray_t* v) { varray_elem_t* e = varray_get(v, varray_length(v)-1); varray_remove(v, varray_length(v)-1); return e; }

/**
   Add the element to the beginning of the array.
   @param v the array
   @param e the element
*/
void varray_unshift(varray_t* v, varray_elem_t* e) { varray_insert(v, 0, e); }

/**
   Remove and return the element at the beginning of the array.
   If the array is empty, returns NULL.
   @param v the array
   @return the element
*/
varray_elem_t* varray_shift(varray_t* v) { varray_elem_t* e = varray_get(v, 0); varray_remove(v, 0); return e; }

/**
   Get a copy of the array.
   The copy is a newly allocated array.
   @param v the array
   @return the copy
*/
varray_t* varray_copy(varray_t* v) { return varray_get_range(v, 0, varray_length(v)); }

/**
   Get a copy of a range of elements from the array.
   The copy is a newly allocated array.
   If any part of the range is out-of-bounds, the range is shrunk until it is entirely within bounds.
   @param v the array
   @param start the start index
   @param len the number of elements
   @return the copy
*/
varray_t* varray_get_range(varray_t* v, int start, int len)
{
    varray_t* v2 = varray_new();
    if (start < 0) { len += start; start = 0; }
    if (len < 0) { len = 0; }
    if (start + len > varray_length(v)) { len = varray_length(v) - start; }
    int i;
    for (i = 0; i < len; i++)
    {
        varray_add(v2, varray_get(v, start+i));
    }
    return v2;
}

/**
   Replace a range of elements of this array with elements from the beginning of another array.
   If the range is out-of-bounds for this array, the array is expanded as needed.
   If the range is out-of-bounds for the array from which elements are copied, the range is shrunk.
   @param v the array
   @param start the start index
   @param len the number of elements
   @param v2 the other array
 */
void varray_set_range(varray_t* v, int start, int len, varray_t* v2)
{
    varray_remove_range(v, start, len);
    varray_insert_range(v, start, len, v2);
}

/**
   Insert a range of elements into this array using the elements from the beginning of another array.
   If the range is out-of-bounds for this array, the array is expanded as needed.
   If the range is out-of-bounds for the array from which elements are copied, the range is shrunk.
   @param v the array
   @param start the start index
   @param len the number of elements
   @param v2 the othe array
 */
void varray_insert_range(varray_t* v, int start, int len, varray_t* v2)
{
    int i;
    if (start < 0) { len += start; start = 0; }
    if (len < 0) { len = 0; }
    if (len > varray_length(v2)) { len = varray_length(v2); }
    for (i = 0; i < len; i++)
    {
        varray_insert(v, start+i, varray_get(v2, i));
    }
}

/**
   Remove a range of elements from this array.
   If any part of the range is out-of-bounds, the range is shrunk until it is entirely within bounds.
   @param v the array
   @param start the start index
   @param len the number of elements
 */
void varray_remove_range(varray_t* v, int start, int len)
{
    int i;
    if (start < 0) { len += start; start = 0; }
    if (len < 0) { len = 0; }
    for (i = 0; i < len; i++)
    {
        varray_remove(v, start);
    }
}

/**
   Add all elements from another array to the end of this one.
   @param v the array
   @param v2 the other array
*/
void varray_add_all(varray_t* v, varray_t* v2) { varray_insert_range(v, v->length, v2->length, v2); }

/**
   Create a new array from a plain C array of pointers.
   @param a the plain array of pointers
   @param len the length of the array
   @return the new array
 */
varray_t* varray_new_from_array(varray_elem_t* a, int len)
{
    varray_t* v = varray_new();
    varray_add_array(v, a, len);
    return v;
}

/**
   Create a plain C array of pointers from an array.
   The length of the returned array can be obtained with varray_length(v).
   @param v the array
   @return the plain array of pointers
 */
varray_elem_t** varray_to_array(varray_t* v)
{
    varray_elem_t** a = (varray_elem_t**)malloc(varray_length(v) * sizeof(varray_elem_t*));
    int i;
    for (i = 0; i < varray_length(v); i++)
    {
        a[i] = varray_get(v, i);
    }
    return a;
}

void varray_add_array(varray_t* v, varray_elem_t** a, int len)
{
    int i;
    for (i = 0; i < len; i++)
    {
        varray_add(v, a[i]);
    }
}

int varray_index_of(varray_t* v, varray_elem_t* e, int (*cmp_func) (varray_elem_t* e1, varray_elem_t* e2))
{
    int i;
    for (i = 0; i < varray_length(v); i++)
    {
        if (cmp_func(varray_get(v, i), e)) { return i; }
    }
    return -1;
}

int varray_last_index_of(varray_t* v, varray_elem_t* e, int (*cmp_func) (varray_elem_t* e1, varray_elem_t* e2))
{
    int i;
    for (i = varray_length(v)-1; i >= 0; i--)
    {
        if (cmp_func(varray_get(v, i), e)) { return i; }
    }
    return -1;
}

/**
 Binary search a sorted array
 @param[in] v         array being searched (must be sorted already)
 @param[in] es        element to search for (key)
 @param[in] cmp_func  function to compare elements with
 @return              index of greatest element less than or equal to key, or -1 if key is less than first element or if array contains no elements
 */
int varray_binary_search(varray_t* v, varray_elem_t* e, int (*cmp_func) (varray_elem_t* e1, varray_elem_t* e2) )
{
    int hi = varray_length(v) - 1;
    if(hi < 0) { return -1; }; // varray_length(v) == 0
    int lo = 0;
    
    //properly handle key less than first element of the list
    if( cmp_func( e, varray_get(v, lo)  ) < 0 ) { return -1; } 
    
    //properly handle at or beyond end of array...if at or beyond end, return end
    if( cmp_func( e, varray_get(v, hi)  ) >= 0 ) { return hi; }

    //Handle everything in between..        
    int res;
    int ndx;
    while(1)
    {
        ndx = (hi - lo)/2 + lo;
        res = cmp_func( varray_get(v, ndx), e );

        if( res == 0 )
        {
            return ndx;
        }
        else if( res < 0 ) { lo = ndx; }
        else { hi = ndx; }

        if( (hi - lo) <= 1) { return lo; }
    }
}

void varray_sort(varray_t* v, int (*cmp_func) (varray_elem_t* e1, varray_elem_t* e2) )
{
    // FIXME qsort call looks correct but is buggy, it affects memory outside the array
    //qsort(v->array, v->length, sizeof(varray_elem_t*), (int(*)(const void *, const void *))cmp_func); 

    /*
    // slow bubble-sort O(N^2)

    int is_sorted;
    int i;
    do
    {
        is_sorted = 1;
        for( i = 0; i < varray_length(v) - 1; i++ )
        {
            if( cmp_func( varray_get(v, i), varray_get(v, i+1) ) > 0 )
            {
                varray_elem_t* tmp = varray_get(v, i);
                varray_set(v, i, varray_get(v, i+1));
                varray_set(v, i+1, tmp);
                is_sorted = 0;
                break;
            }
        }
    }
    while( ! is_sorted );
    */

    // fast in-place heapsort O(N log(N)) 
    // general idea copied from Numerical Recipes in C

    int n = v->length;
    int i = n/2;
    int parent, child;
    varray_elem_t* tmp;
 
    while(1)
    {
        if (i > 0)
        {
            i--;
            tmp = v->array[i];
        }
        else
        {
            n--;
            if (n == 0) { break; }
            tmp = v->array[n];
            v->array[n] = v->array[0];
        }
 
        parent = i;
        child = i*2 + 1;
 
        while (child < n)
        {
            if (child + 1 < n  &&  
                cmp_func( v->array[child + 1], v->array[child] ) > 0)
            {
                child++;
            }
            if (cmp_func( v->array[child], tmp ) > 0)
            {
                v->array[parent] = v->array[child];
                parent = child;
                child = parent*2 + 1;
            }
            else
            {
                break;
            }
        }

        v->array[parent] = tmp;
    }

}

varray_t* varray_grep(varray_t* v, int (*grep_func) (varray_elem_t* e) )
{
    varray_t* v2 = varray_new();
    int i;
    for (i = 0; i < varray_length(v); i++)
    {
        varray_elem_t* e = varray_get(v, i);
        if (grep_func(e))
        {
            varray_add(v2, e);
        }
    }
    return v2;
}

varray_t* varray_map(varray_t* v, varray_elem_t* (*map_func) (varray_elem_t* e) )
{
    varray_t* v2 = varray_new();
    int i;
    for (i = 0; i < varray_length(v); i++)
    {
        varray_elem_t* e = varray_get(v, i);
        varray_elem_t* e2 = map_func(e);
        if (e2 != NULL)
        {
            varray_add(v2, e2);
        }
    }
    return v2;
}

varray_iterator_t* varray_iterator_new(varray_t* v)
{
    varray_iterator_t* iter = (varray_iterator_t*)malloc(sizeof(varray_iterator_t));
    iter->v = v;
    iter->i = -1;
    return iter;
}

void varray_iterator_free(varray_iterator_t* iter)
{
    free(iter);
}

varray_elem_t* varray_iterator_next(varray_iterator_t* iter)
{
    if (iter->i < varray_length(iter->v))
    {
        iter->i++;
    }
    return varray_get(iter->v, iter->i);
}

varray_elem_t* varray_iterator_previous(varray_iterator_t* iter)
{
    if (iter->i >= 0)
    {
        iter->i--;
    }
    return varray_get(iter->v, iter->i);
}

int varray_iterator_has_next(varray_iterator_t* iter)
{
    if (iter->i >= -1 && iter->i < varray_length(iter->v) - 1) { return 1; }
    else { return 0; }
}

int varray_iterator_has_previous(varray_iterator_t* iter)
{
    if (iter->i > 0 && iter->i <= varray_length(iter->v)) { return 1; }
    else { return 0; }
}


void _varray_expand_to_length(varray_t* v, int length)
{
    int memlength;
    if (v->memlength >= length) { return; }

    if (length <= varray_start_length)
    {
        memlength = varray_start_length;
    }
    else if (length < varray_block_length && length < v->memlength * 2)
    {
        memlength = v->memlength * 2;
    }
    else
    {
        memlength = varray_block_length * (1 + ((length + 1) / varray_block_length));
    }

    v->array = (varray_elem_t**)realloc(v->array, memlength*sizeof(varray_elem_t*));

    /*
    // this may be much faster than realloc in some circumstances (adding element far beyond the end of array) but much slower in others (depending on malloc memory block merging behavior)
    varray_elem_t* tmp = v->array;
    v->array = (varray_elem_t**)malloc(memlength*sizeof(varray_elem_t*));
    if (v->length > 0 && tmp != NULL)
    {
        memcpy(v->array, tmp, v->length*sizeof(varray_elem_t*));
    }
    if (tmp != NULL)
    {
        free(tmp);
    }
    */

    v->memlength = memlength;
}

