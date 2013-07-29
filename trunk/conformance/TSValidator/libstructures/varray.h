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

#ifndef VARRAY_INCLUDE
#define VARRAY_INCLUDE

typedef void varray_elem_t;

typedef struct
{
    varray_elem_t** array;
    int length;
    int memlength;
} varray_t;

typedef struct
{
    varray_t* v;
    int i;
} varray_iterator_t;


varray_t* varray_new();
void varray_free(varray_t* v);
void varray_clear(varray_t* v);
inline int varray_length(varray_t* v);
inline varray_elem_t* varray_get(varray_t* v, int i);
inline void varray_set(varray_t* v, int i, varray_elem_t* e);
void varray_insert(varray_t* v, int i, varray_elem_t* e);
void varray_remove(varray_t* v, int i);
void varray_add(varray_t* v, varray_elem_t* e);
void varray_push(varray_t* v, varray_elem_t* e);
varray_elem_t* varray_pop(varray_t* v);
void varray_unshift(varray_t* v, varray_elem_t* e);
varray_elem_t* varray_shift(varray_t* v);
varray_t* varray_copy(varray_t* v);
varray_t* varray_get_range(varray_t* v, int start, int len);
void varray_set_range(varray_t* v, int start, int len, varray_t* v2);
void varray_insert_range(varray_t* v, int start, int len, varray_t* v2);
void varray_remove_range(varray_t* v, int start, int len);
void varray_add_all(varray_t* v, varray_t* v2);
varray_t* varray_new_from_array(varray_elem_t* a, int len);
varray_elem_t** varray_to_array(varray_t* v);
void varray_add_array(varray_t* v, varray_elem_t** a, int len);
int varray_index_of(varray_t* v, varray_elem_t* e, int (*cmp_func) (varray_elem_t* e1, varray_elem_t* e2));
int varray_last_index_of(varray_t* v, varray_elem_t* e, int (*cmp_func) (varray_elem_t* e1, varray_elem_t* e2));
void varray_sort(varray_t* v, int (*cmp_func) (varray_elem_t* e1, varray_elem_t* e2) );
int varray_binary_search(varray_t* v, varray_elem_t* e, int (*cmp_func) (varray_elem_t* e1, varray_elem_t* e2) );
varray_t* varray_grep(varray_t* v, int (*grep_func) (varray_elem_t* e) );
varray_t* varray_map(varray_t* v, varray_elem_t* (*map_func) (varray_elem_t* e) );
varray_iterator_t* varray_iterator_new(varray_t* v);
void varray_iterator_free(varray_iterator_t* iter);
varray_elem_t* varray_iterator_next(varray_iterator_t* iter);
varray_elem_t* varray_iterator_previous(varray_iterator_t* iter);
int varray_iterator_has_next(varray_iterator_t* iter);
int varray_iterator_has_previous(varray_iterator_t* iter);
void _varray_expand_to_length(varray_t* v, int length);

#endif
