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

#include "vqarray.h"

const int vqarray_start_length = 16;
const int vqarray_block_length = 1024;

void _vqarray_expand_to_length(vqarray_t* v, int length);

/**
   Create new array.  The array is initially empty.
   @return  the new array
 */
vqarray_t* vqarray_new()
{
    vqarray_t* v = (vqarray_t*)malloc(sizeof(vqarray_t));
    v->array = NULL;
    v->memlength = 0;
    v->length = 0;
    v->start = 0;
    _vqarray_expand_to_length(v, vqarray_start_length);
    return v;
}

/**
*  Initialize a new pre-allocated array.  The array is initially empty.
*  @param[in,out] v vqarray to initialize
*  @param[i] length initial number of elements ( -1 to use the default ) 
   @return  the new array
 */
vqarray_t* vqarray_init(vqarray_t* v, int length )
{
    v->array = NULL;
    v->memlength = 0;
    v->length = 0;
    v->start = 0;
    if ( length < 1 ) length = vqarray_start_length;
    _vqarray_expand_to_length(v, length);
    return v;
}

/**
   Free array.  The array must not be used after this.
   @param v the array
 */
void vqarray_free(vqarray_t* v)
{
    v->start = 0;
    v->length = 0;
    v->memlength = 0;
    free(v->array);
    v->array = NULL;
    free(v);
}

/**
*  Free buffer.
*  The array must not be used after this (it does not have a buffer)
*  The function must be only used as a vqarray_free substitution
*  in case of a statically allocated vqarray
*  @param v the array
*/
void vqarray_free_buf(vqarray_t* v)
{
    if ( v->array == NULL ) return;

    v->start = 0;
    v->length = 0;
    v->memlength = 0;
    free(v->array);

    v->array = NULL;
}

/**
   Insert the element at the given index in the array.
   Any subsequent elements are moved up.
   If the index is out of bounds, the array is grown to that size.  The values of any intermediate elements created by that will be NULL.
   @param v the array
   @param i the index
   @param e the element
*/
void vqarray_insert(vqarray_t* v, int i, vqarray_elem_t* e)
{
    if (i < 0) { return; }
    _vqarray_expand_to_length(v, _max(v->length+1, i+1));
    if (v->start > 0 && i < v->length/2)
    {
        // move beginning backwards
        if (i > 0) { memmove(&(v->array[v->start - 1]), &(v->array[v->start]), i * sizeof(vqarray_elem_t*)); }
        v->start --;
    }
    else
    {
        // move end forwards
        if (i < v->length) { memmove(&(v->array[i+1 + v->start]), &(v->array[i + v->start]), (v->length-i) * sizeof(vqarray_elem_t*)); }
    }
    
    v->array[i + v->start] = e;
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
void vqarray_remove(vqarray_t* v, int i)
{
    if (i < 0 || i >= v->length) { return; }
    if (i < v->length/2)
    {
        // move beginning forwards
        if (i > 0) { memmove(&(v->array[v->start + 1]), &(v->array[v->start]), i * sizeof(vqarray_elem_t*)); }
        v->start ++;
    }
    else
    {
        // move end backwards
        if (i < v->length) { memmove(&(v->array[i + v->start]), &(v->array[i+1 + v->start]), (v->length-(i+1)) * sizeof(vqarray_elem_t*)); }
    }
    v->length --; //if (v->length > 0) { v->length --; } // not possible, i == v->length in this case
}

/**
   Get a copy of a range of elements from the array.
   The copy is a newly allocated array.
   If any part of the range is out-of-bounds, the range is shrunk until it is entirely within bounds.
   @param v the array
   @param start the start index
   @param len the number of elements
   @return the copy
*/
vqarray_t* vqarray_get_range(vqarray_t* v, int start, int len)
{
    vqarray_t* v2 = vqarray_new();
    if (start < 0) { len += start; start = 0; }
    if (len < 0) { len = 0; }
    if (start + len > vqarray_length(v)) { len = vqarray_length(v) - start; }
    int i;
    for (i = 0; i < len; i++)
    {
        vqarray_add(v2, vqarray_get(v, start+i));
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
void vqarray_set_range(vqarray_t* v, int start, int len, vqarray_t* v2)
{
    vqarray_remove_range(v, start, len);
    vqarray_insert_range(v, start, len, v2);
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
void vqarray_insert_range(vqarray_t* v, int start, int len, vqarray_t* v2)
{
    int i;
    if (start < 0) { len += start; start = 0; }
    if (len < 0) { len = 0; }
    if (len > vqarray_length(v2)) { len = vqarray_length(v2); }
    for (i = 0; i < len; i++)
    {
        vqarray_insert(v, start+i, vqarray_get(v2, i));
    }
}

/**
   Remove a range of elements from this array.
   If any part of the range is out-of-bounds, the range is shrunk until it is entirely within bounds.
   @param v the array
   @param start the start index
   @param len the number of elements
 */
void vqarray_remove_range(vqarray_t* v, int start, int len)
{
    int i;
    if (start < 0) { len += start; start = 0; }
    if (len < 0) { len = 0; }
    for (i = 0; i < len; i++)
    {
        vqarray_remove(v, start);
    }
}

/**
   Add all elements from another array to the end of this one.
   @param v the array
   @param v2 the other array
*/
void vqarray_add_all(vqarray_t* v, vqarray_t* v2) { vqarray_insert_range(v, v->length, v2->length, v2); }

/**
   Create a new array from a plain C array of pointers.
   @param a the plain array of pointers
   @param len the length of the array
   @return the new array
 */
vqarray_t* vqarray_new_from_array(vqarray_elem_t* a, int len)
{
    vqarray_t* v = vqarray_new();
    vqarray_add_array(v, a, len);
    return v;
}

vqarray_t* vqarray_new_reserve(int len)
{
    vqarray_t* v = (vqarray_t*)malloc(sizeof(vqarray_t));
    v->array = NULL;
    v->memlength = 0;
    v->length = 0;
    v->start = 0;
    _vqarray_expand_to_length(v, len);
    return v;
}

/**
   Create a plain C array of pointers from an array.
   The length of the returned array can be obtained with vqarray_length(v).
   @param v the array
   @return the plain array of pointers
 */
vqarray_elem_t** vqarray_to_array(vqarray_t* v)
{
    vqarray_elem_t** a = (vqarray_elem_t**)malloc(vqarray_length(v) * sizeof(vqarray_elem_t*));
    int i;
    for (i = 0; i < vqarray_length(v); i++)
    {
        a[i] = vqarray_get(v, i);
    }
    return a;
}

void vqarray_add_array(vqarray_t* v, vqarray_elem_t** a, int len)
{
    int i;
    for (i = 0; i < len; i++)
    {
        vqarray_add(v, a[i]);
    }
}

int vqarray_index_of(vqarray_t* v, vqarray_elem_t* e, int (*cmp_func) (vqarray_elem_t* e1, vqarray_elem_t* e2))
{
    int i;
    for (i = 0; i < vqarray_length(v); i++)
    {
        if (cmp_func(vqarray_get(v, i), e)) { return i; }
    }
    return -1;
}

int vqarray_last_index_of(vqarray_t* v, vqarray_elem_t* e, int (*cmp_func) (vqarray_elem_t* e1, vqarray_elem_t* e2))
{
    int i;
    for (i = vqarray_length(v)-1; i >= 0; i--)
    {
        if (cmp_func(vqarray_get(v, i), e)) { return i; }
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
int vqarray_binary_search(vqarray_t* v, vqarray_elem_t* e, int (*cmp_func) (vqarray_elem_t* e1, vqarray_elem_t* e2) )
{
    int hi = vqarray_length(v) - 1;
    if(hi < 0) { return -1; }; // vqarray_length(v) == 0
    int lo = 0;
    
    //properly handle key less than first element of the list
    if( cmp_func( e, vqarray_get(v, lo)  ) < 0 ) { return -1; } 
    
    //properly handle at or beyond end of array...if at or beyond end, return end
    if( cmp_func( e, vqarray_get(v, hi)  ) >= 0 ) { return hi; }

    //Handle everything in between..        
    int res;
    int ndx;
    while(1)
    {
        ndx = (hi - lo)/2 + lo;
        res = cmp_func( vqarray_get(v, ndx), e );

        if( res == 0 )
        {
            return ndx;
        }
        else if( res < 0 ) { lo = ndx; }
        else { hi = ndx; }

        if( (hi - lo) <= 1) { return lo; }
    }
}

int vqarray_binary_search_by_key(vqarray_t* v, void* key, int (*cmp_func) (void* key, vqarray_elem_t* e2) )
{
    int hi = vqarray_length(v) - 1;
    if(hi < 0) { return -1; }; // vqarray_length(v) == 0
    int lo = 0;
    
    //properly handle key less than first element of the list
    if( cmp_func( key, vqarray_get(v, lo)  ) < 0 ) { return -1; } 
    
    //properly handle at or beyond end of array...if at or beyond end, return end
    if( cmp_func( key, vqarray_get(v, hi)  ) >= 0 ) { return hi; }

    //Handle everything in between..        
    int res;
    int ndx;
    while(1)
    {
        ndx = (hi - lo)/2 + lo;
        res = -1*cmp_func( key, vqarray_get(v, ndx) );

        if( res == 0 )
        {
            return ndx;
        }
        else if( res < 0 ) { lo = ndx; }
        else { hi = ndx; }

        if( (hi - lo) <= 1) { return lo; }
    }
}

void vqarray_sort(vqarray_t* v, int (*cmp_func) (vqarray_elem_t* e1, vqarray_elem_t* e2) )
{
    // FIXME qsort call looks correct but is buggy, it affects memory outside the array
    //qsort(v->array, v->length, sizeof(vqarray_elem_t*), (int(*)(const void *, const void *))cmp_func); 

    /*
    // slow bubble-sort O(N^2)

    int is_sorted;
    int i;
    do
    {
        is_sorted = 1;
        for( i = 0; i < vqarray_length(v) - 1; i++ )
        {
            if( cmp_func( vqarray_get(v, i), vqarray_get(v, i+1) ) > 0 )
            {
                vqarray_elem_t* tmp = vqarray_get(v, i);
                vqarray_set(v, i, vqarray_get(v, i+1));
                vqarray_set(v, i+1, tmp);
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
    vqarray_elem_t* tmp;
 
    vqarray_elem_t** array = &(v->array[v->start]);
    while(1)
    {
        if (i > 0)
        {
            i--;
            tmp = array[i];
        }
        else
        {
            n--;
            if (n == 0) { break; }
            tmp = array[n];
            array[n] = array[0];
        }
 
        parent = i;
        child = i*2 + 1;
 
        while (child < n)
        {
            if (child + 1 < n  &&  
                cmp_func( array[child + 1], array[child] ) > 0)
            {
                child++;
            }
            if (cmp_func( array[child], tmp ) > 0)
            {
                array[parent] = array[child];
                parent = child;
                child = parent*2 + 1;
            }
            else
            {
                break;
            }
        }

        array[parent] = tmp;
    }

}

vqarray_t* vqarray_grep(vqarray_t* v, int (*grep_func) (vqarray_elem_t* e) )
{
    vqarray_t* v2 = vqarray_new();
    int i;
    for (i = 0; i < vqarray_length(v); i++)
    {
        vqarray_elem_t* e = vqarray_get(v, i);
        if (grep_func(e))
        {
            vqarray_add(v2, e);
        }
    }
    return v2;
}

vqarray_t* vqarray_map(vqarray_t* v, vqarray_elem_t* (*map_func) (vqarray_elem_t* e) )
{
    vqarray_t* v2 = vqarray_new();
    int i;
    for (i = 0; i < vqarray_length(v); i++)
    {
        vqarray_elem_t* e = vqarray_get(v, i);
        vqarray_elem_t* e2 = map_func(e);
        if (e2 != NULL)
        {
            vqarray_add(v2, e2);
        }
    }
    return v2;
}

vqarray_iterator_t* vqarray_iterator_new(vqarray_t* v)
{
    vqarray_iterator_t* iter = (vqarray_iterator_t*)malloc(sizeof(vqarray_iterator_t));
    iter->v = v;
    iter->i = -1;
    return iter;
}

void vqarray_iterator_free(vqarray_iterator_t* iter)
{
    free(iter);
}

vqarray_elem_t* vqarray_iterator_next(vqarray_iterator_t* iter)
{
    if (iter->i < vqarray_length(iter->v))
    {
        iter->i++;
    }
    return vqarray_get(iter->v, iter->i);
}

vqarray_elem_t* vqarray_iterator_previous(vqarray_iterator_t* iter)
{
    if (iter->i >= 0)
    {
        iter->i--;
    }
    return vqarray_get(iter->v, iter->i);
}

int vqarray_iterator_has_next(vqarray_iterator_t* iter)
{
    if (iter->i >= -1 && iter->i < vqarray_length(iter->v) - 1) { return 1; }
    else { return 0; }
}

int vqarray_iterator_has_previous(vqarray_iterator_t* iter)
{
    if (iter->i > 0 && iter->i <= vqarray_length(iter->v)) { return 1; }
    else { return 0; }
}


void _vqarray_expand_to_length(vqarray_t* v, int length)
{
    int memlength;
    if (v->memlength >= length + v->start) { return; }

    if (v->start > vqarray_block_length)
    {
        memmove(&(v->array[vqarray_block_length]), &(v->array[v->start]), v->length*sizeof(vqarray_elem_t*));
        v->start = vqarray_block_length;
    }

    memlength = vqarray_block_length * (2 + ((length + v->start + 1) / vqarray_block_length));

    v->array = (vqarray_elem_t**)realloc(v->array, memlength*sizeof(vqarray_elem_t*));

    if (v->start < vqarray_block_length)
    {
        memmove(&(v->array[vqarray_block_length]), &(v->array[v->start]), v->length*sizeof(vqarray_elem_t*));
        v->start = vqarray_block_length;
    }

    v->memlength = memlength;
}

void vqarray_foreach(vqarray_t* v, void (*func)( void *arg0))
{
    int i;
    for (i = 0; i < vqarray_length(v); i++) 
    {
        vqarray_elem_t* e = vqarray_get(v, i);
        func((void *)e);
    }
}

void vqarray_foreach2(vqarray_t* v, void (*func)( void *arg0, void *arg1), void* arg)
{
    int i;
    for (i = 0; i < vqarray_length(v); i++) 
    {
        vqarray_elem_t* e = vqarray_get(v, i);
        func((void *)e, arg);
    }
}

