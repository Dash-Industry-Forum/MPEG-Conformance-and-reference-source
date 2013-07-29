#include "varray.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <sys/time.h>

#include "test_macros.h"

int verbose = 0;

// expects a NULL-terminated string array
int _varray_cmp_to_string_array(varray_t* v, const char** strs)
{
    int i = 0;
    int result = 0;
    while (strs[i] != NULL)
    {
        char* e = (char*)varray_get(v, i);
        if (e == NULL) { result = -2; continue; }
        if (result == 0) { result = strcmp(e, strs[i]); }
        i++;
    }
    if (varray_length(v) != i) { result = -3; }
    return result;
}

void _varray_dump(varray_t* v)
{
    int i;
    for (i = 0; i < varray_length(v); i++)
    {
        printf("'%s', ", (char*)varray_get(v, i));
    }
    printf("\n");
}

START_TEST (test_insert_remove)
{
    varray_t* v = varray_new();
    varray_insert(v, 0, (char*)"b");
    const char* strs1[] = { "b", NULL };
    fail_unless( _varray_cmp_to_string_array(v, strs1) == 0, "insert into empty failed" );
    varray_insert(v, 0, (char*)"a");
    const char* strs2[] = { "a", "b", NULL };
    fail_unless( _varray_cmp_to_string_array(v, strs2) == 0, "insert into start failed" );
    varray_insert(v, 2, (char*)"d");
    const char* strs3[] = { "a", "b", "d", NULL };
    fail_unless( _varray_cmp_to_string_array(v, strs3) == 0, "insert into end failed" );
    varray_insert(v, 2, (char*)"c");
    const char* strs4[] = { "a", "b", "c", "d", NULL };
    fail_unless( _varray_cmp_to_string_array(v, strs4) == 0, "insert into middle failed" );
    varray_remove(v, 2);
    const char* strs5[] = { "a", "b", "d", NULL };
    fail_unless( _varray_cmp_to_string_array(v, strs5) == 0, "remove from middle failed" );
    varray_remove(v, varray_length(v)-1);
    const char* strs6[] = { "a", "b", NULL };
    fail_unless( _varray_cmp_to_string_array(v, strs6) == 0, "remove from end failed" );
    varray_free(v);
}
END_TEST

int64_t* _new_int64(int64_t i)
{
    int64_t* p = (int64_t*)malloc(sizeof(int64_t));
    p[0] = i;
    return p;
}

void _free_int64(varray_elem_t* e)
{
    free(e);
}

int _cmp_int64(varray_elem_t* e1, varray_elem_t* e2)
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
    varray_t* v = varray_new();

    varray_add(v, _new_int64(6));
    varray_add(v, _new_int64(9));
    varray_add(v, _new_int64(19));
    varray_add(v, _new_int64(29));
    varray_add(v, _new_int64(39));
    varray_add(v, _new_int64(49));
    varray_add(v, _new_int64(59));
    varray_add(v, _new_int64(69));
    varray_add(v, _new_int64(79));
    varray_add(v, _new_int64(89));
    varray_add(v, _new_int64(99));
    varray_add(v, _new_int64(109));
    varray_add(v, _new_int64(119));

    //Search off the low end
    int64_t* t = _new_int64(0);
    int ndx = varray_binary_search(v, t, _cmp_int64);
    fail_unless(ndx == -1, "search before beginning failed");

    //Search for first item
    t[0] = 6;
    ndx = varray_binary_search(v, t, _cmp_int64);
    fail_unless(ndx == 0, "search for beginning item failed");
    
    t[0] = 19;
    ndx = varray_binary_search(v, t, _cmp_int64);
    fail_unless(ndx == 2, "search 2 failed");
    
    t[0] = 34;
    ndx = varray_binary_search(v, t, _cmp_int64);
    fail_unless(ndx == 3, "search 3 failed");

    t[0] = 50;
    ndx = varray_binary_search(v, t, _cmp_int64);
    fail_unless(ndx == 5, "search 4 failed");

    t[0] = 119;
    ndx = varray_binary_search(v, t, _cmp_int64);
    fail_unless(ndx == 12, "search for last item in list failed");

    t[0] = 120;
    ndx = varray_binary_search(v, t, _cmp_int64);
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
        varray_t* v = varray_new();
        int size = _random64() % max_size;
        for(j = 0; j < size; j++)
        {
            varray_add(v, _new_int64(_random64()));
        }

        t1 = gettimeusec();
        varray_sort(v, _cmp_int64);
        t2 = gettimeusec();
        tt += t2 - t1;

        for(j = 0; j < varray_length(v) - 1; j++)
        {
            fail_unless(_cmp_int64( varray_get(v, j), varray_get(v, j+1) ) <= 0, "array not sorted correctly");
        }
        
        for(j = varray_length(v) - 1; j >= 0; j--)
        {
            free( varray_get(v, j) );
            varray_remove(v, j);
        }
        varray_free(v);
    }

    if (verbose) { printf("sort loop: %f /sec (%d repeats, %d array size)\n", (1000000*(double)num_repeats)/tt, num_repeats, max_size/2); }
}
END_TEST

int main(int argc, char** argv)
{
    //plan_tests(1);
    int _testnum = 1;

    ok( test_insert_remove() , "insert and remove");
    ok( test_binary_search() , "binary search");
    ok( test_sort() ,          "sort");
    // TODO more tests

    return 0;
}
