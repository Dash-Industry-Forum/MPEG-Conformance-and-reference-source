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

#include "vqarray.h"
#include "varray.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <sys/time.h>

#include "test_macros.h"

int verbose = 0;

// expects a NULL-terminated string array
int _vqarray_cmp_to_string_array(vqarray_t* v, const char** strs)
{
    int i = 0;
    int result = 0;
    while (strs[i] != NULL)
    {
        char* e = (char*)vqarray_get(v, i);
        if (e == NULL) { result = -2; continue; }
        if (result == 0) { result = strcmp(e, strs[i]); }
        i++;
    }
    if (vqarray_length(v) != i) { result = -3; }
    return result;
}

void _vqarray_dump(vqarray_t* v)
{
    int i;
    for (i = 0; i < vqarray_length(v); i++)
    {
        printf("'%s', ", (char*)vqarray_get(v, i));
    }
    printf("\n");
}

char* _vqarray_to_str(vqarray_t* v)
{
    int i;
    char* s = malloc(vqarray_length(v) + 1);
    for (i = 0; i < vqarray_length(v); i++)
    {
        char* c = (char*)vqarray_get(v, i);
        s[i] = c[0];
    }
    s[ vqarray_length(v) ] = '\0';
    return s;
}

char* _varray_to_str(varray_t* v)
{
    int i;
    char* s = malloc(varray_length(v) + 1);
    for (i = 0; i < varray_length(v); i++)
    {
        char* c = (char*)varray_get(v, i);
        s[i] = c[0];
    }
    s[ varray_length(v) ] = '\0';
    return s;
}

START_TEST (test_insert_remove)
{
    vqarray_t* v = vqarray_new();
    vqarray_insert(v, 0, (char*)"b");
    const char* strs1[] = { "b", NULL };
    fail_unless( _vqarray_cmp_to_string_array(v, strs1) == 0, "insert into empty failed" );
    vqarray_insert(v, 0, (char*)"a");
    const char* strs2[] = { "a", "b", NULL };
    fail_unless( _vqarray_cmp_to_string_array(v, strs2) == 0, "insert into start failed" );
    vqarray_insert(v, 2, (char*)"d");
    const char* strs3[] = { "a", "b", "d", NULL };
    fail_unless( _vqarray_cmp_to_string_array(v, strs3) == 0, "insert into end failed" );
    vqarray_insert(v, 2, (char*)"c");
    const char* strs4[] = { "a", "b", "c", "d", NULL };
    fail_unless( _vqarray_cmp_to_string_array(v, strs4) == 0, "insert into middle failed" );
    vqarray_add(v, (char*)"e");
    vqarray_add(v, (char*)"f");
    vqarray_add(v, (char*)"g");
    vqarray_add(v, (char*)"h");
    const char* strs5[] = { "a", "b", "c", "d", "e", "f", "g", "h", NULL };
    fail_unless( _vqarray_cmp_to_string_array(v, strs5) == 0, "insert more failed" );
    vqarray_remove(v, 1);
    const char* strs6[] = { "a", "c", "d", "e", "f", "g", "h", NULL };
    fail_unless( _vqarray_cmp_to_string_array(v, strs6) == 0, "remove from start failed" );
    vqarray_remove(v, vqarray_length(v)-2);
    const char* strs7[] = { "a", "c", "d", "e", "f", "h", NULL };
    fail_unless( _vqarray_cmp_to_string_array(v, strs7) == 0, "remove from end failed" );
    vqarray_free(v);
}
END_TEST

int64_t* _new_int64(int64_t i)
{
    int64_t* p = (int64_t*)malloc(sizeof(int64_t));
    p[0] = i;
    return p;
}

void _free_int64(vqarray_elem_t* e)
{
    free(e);
}

int _cmp_int64(vqarray_elem_t* e1, vqarray_elem_t* e2)
{
    int64_t* t1 = (int64_t*) e1;
    int64_t* t2 = (int64_t*) e2;
    if ( t1[0] < t2[0] ) { return -1; }
    else if ( t1[0] > t2[0] ) { return 1; }
    else { return 0; }
}

uint64_t _random64()
{
    uint64_t r1 = rand();
    uint64_t r2 = rand();
    return (r1 << 32) | r2;
}

uint64_t gettimeusec()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    uint64_t t = tv.tv_sec*1000000 + tv.tv_usec;
    return t;
}

START_TEST(test_binary_search)
{
    vqarray_t* v = vqarray_new();

    vqarray_add(v, _new_int64(6));
    vqarray_add(v, _new_int64(9));
    vqarray_add(v, _new_int64(19));
    vqarray_add(v, _new_int64(29));
    vqarray_add(v, _new_int64(39));
    vqarray_add(v, _new_int64(49));
    vqarray_add(v, _new_int64(59));
    vqarray_add(v, _new_int64(69));
    vqarray_add(v, _new_int64(79));
    vqarray_add(v, _new_int64(89));
    vqarray_add(v, _new_int64(99));
    vqarray_add(v, _new_int64(109));
    vqarray_add(v, _new_int64(119));

    int64_t* t = _new_int64(0);
    int ndx = vqarray_binary_search(v, t, _cmp_int64);
    fail_unless(ndx == -1, "search before beginning failed");

    t[0] = 6;
    ndx = vqarray_binary_search(v, t, _cmp_int64);
    fail_unless(ndx == 0, "search for first item failed");

    t[0] = 19;
    ndx = vqarray_binary_search(v, t, _cmp_int64);
    fail_unless(ndx == 2, "search 2 failed");

    t[0] = 34;
    ndx = vqarray_binary_search(v, t, _cmp_int64);
    fail_unless(ndx == 3, "search 3 failed");

    t[0] = 50;
    ndx = vqarray_binary_search(v, t, _cmp_int64);
    fail_unless(ndx == 5, "search 4 failed");

    t[0] = 119;
    ndx = vqarray_binary_search(v, t, _cmp_int64);
    fail_unless(ndx == 12, "search for end failed");

    t[0] = 120;
    ndx = vqarray_binary_search(v, t, _cmp_int64);
    fail_unless(ndx == 12, "search past end failed");
}
END_TEST

START_TEST(test_sort)
{
    srand(12345);
    int i, j;
    uint64_t t1, t2, tt;
    tt = 0;
    int num_repeats = 10;
    int max_size = 20000;
    for(i = 0; i < num_repeats; i++)
    {
        vqarray_t* v = vqarray_new();
        int size = _random64() % max_size;
        for(j = 0; j < size; j++)
        {
            vqarray_add(v, _new_int64(_random64()));
        }

        t1 = gettimeusec();
        vqarray_sort(v, _cmp_int64);
        t2 = gettimeusec();
        tt += t2 - t1;

        for(j = 0; j < vqarray_length(v) - 1; j++)
        {
            fail_unless(_cmp_int64( vqarray_get(v, j), vqarray_get(v, j+1) ) <= 0, "array not sorted correctly");
        }
        
        for(j = vqarray_length(v) - 1; j >= 0; j--)
        {
            free( vqarray_get(v, j) );
            vqarray_remove(v, j);
        }
        vqarray_free(v);
    }

    if (verbose) { printf("sort loop: %f /sec (%d repeats, %d array size)\n", (1000000*(double)num_repeats)/tt, num_repeats, max_size/2); }
}
END_TEST

START_TEST(test_random)
{
    char* letters1 = "abcdefghijklmnopqrstuvwxyz";
    char* letters = malloc(26);
    memcpy(letters, letters1, 26);

    varray_t* v = varray_new();
    vqarray_t* vq = vqarray_new();

    // compare with varray after the same random ops are performed on both
    int num_repeats = 20000;
    int max_size = 8000;
    
    int i;
    for(i = 0; i < num_repeats; i++)
    {
        int op = _random64();
        if (vqarray_length(vq) > max_size) { op = 1; }
        int idx;
        if (op % 3 == 0) { idx = _random64() % vqarray_length(vq); }
        if (op % 3 == 1) { idx = 0; }
        if (op % 3 == 2) { idx = vqarray_length(vq); }
        if (op % 2 == 0)
        {
            int lidx = _random64() % 10;
            vqarray_insert(vq, idx, &(letters[lidx]));
            varray_insert(v, idx, &(letters[lidx]));
        }
        else if (op % 2 == 1)
        {
            vqarray_remove(vq, idx);
            varray_remove(v, idx);
        }
        char* sq = _vqarray_to_str(vq);
        char* s = _varray_to_str(v);

        fail_unless(strcmp(s, sq) == 0, "varray and vqarray produced different results");
        free(sq);
        free(s);
    }
}
END_TEST


int main(int argc, char** argv)
{
    //plan_tests(1);
    int _testnum = 1;

    ok( test_insert_remove() , "insert and remove");
    ok( test_binary_search() , "binary search");
    ok( test_sort() ,          "sort");
    ok( test_random() ,        "random");
    // TODO more tests

    return 0;
}
