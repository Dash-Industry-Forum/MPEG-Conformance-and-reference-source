#include "binheap.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>

#include "test_macros.h"

int verbose = 0;

int string_cmp_func(binheap_elem_t* e1, binheap_elem_t* e2)
{
    return strcmp((const char*)e1, (const char*)e2);
}

void unary_test(binheap_elem_t* e)
{
    fprintf( stderr, "%s\n",  (char* )e );
}

START_TEST (test_binheap_strings)
{
    binheap_t* bh = binheap_new(string_cmp_func);
    binheap_insert(bh, (binheap_elem_t*)"z");
    binheap_insert(bh, (binheap_elem_t*)"a");
    binheap_insert(bh, (binheap_elem_t*)"c");
    binheap_insert(bh, (binheap_elem_t*)"y");
    binheap_insert(bh, (binheap_elem_t*)"b");
    binheap_insert(bh, (binheap_elem_t*)"x");

    binheap_foreach( bh, unary_test);

    if (verbose) { _binheap_dump(bh); }

    fail_unless( strcmp((char*) binheap_remove_first(bh), "a") == 0, "got wrong element" );
    fail_unless( strcmp((char*) binheap_remove_first(bh), "b") == 0, "got wrong element" );
    fail_unless( strcmp((char*) binheap_remove_first(bh), "c") == 0, "got wrong element" );
    fail_unless( strcmp((char*) binheap_remove_first(bh), "x") == 0, "got wrong element" );
    fail_unless( strcmp((char*) binheap_remove_first(bh), "y") == 0, "got wrong element" );
    fail_unless( strcmp((char*) binheap_remove_first(bh), "z") == 0, "got wrong element" );
}
END_TEST

uint64_t gettimeusec()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    uint64_t t = tv.tv_sec*1000000 + tv.tv_usec;
    return t;
}

int uint64_cmp_func(binheap_elem_t* e1, binheap_elem_t* e2)
{
    uint64_t v1, v2;
    v1 = ((uint64_t*) e1)[0];
    v2 = ((uint64_t*) e2)[0];
    if ( v1 < v2 ) { return -1; }
    else if ( v1 > v2 ) { return 1; }
    else { return 0; }
}

START_TEST (test_binheap_uint64_benchmark)
{
    binheap_t* bh = binheap_new(uint64_cmp_func);
    int i;
    int heap_size = 100000;
    int num_repeats = 1000000;
    uint64_t t1, t2;
    uint64_t* p;

    for (i = 0; i < heap_size; i++)
    {
        p = (uint64_t*)malloc(sizeof(uint64_t));
        p[0] = rand();
        binheap_insert(bh, p);
    }

    t1 = gettimeusec();
    for (i = 0; i < num_repeats; i++)
    {
        p = (uint64_t*)malloc(sizeof(uint64_t));
        p[0] = rand();
        free(p);
    }
    t2 = gettimeusec();
    if (verbose) { printf("empty loop: %f /sec (%d repeats)\n", (1000000*(double)num_repeats)/(t2 - t1), num_repeats); }

    t1 = gettimeusec();
    for (i = 0; i < num_repeats; i++)
    {
        p = (uint64_t*)malloc(sizeof(uint64_t));
        p[0] = rand();
        binheap_insert(bh, p);
        p = (uint64_t*) binheap_remove_first(bh);
        free(p);
    }
    t2 = gettimeusec();
    if (verbose) { printf("insert/remove loop: %f /sec (%d repeats, %d heap size)\n", (1000000*(double)num_repeats)/(t2 - t1), num_repeats, heap_size); }
    if (verbose) { printf("heap size after: %d\n", binheap_size(bh)); }
}
END_TEST

int main(int argc, char** argv)
{
    //plan_tests(1);
    int _testnum = 1;

    ok( test_binheap_strings() , "strings");
    ok( test_binheap_uint64_benchmark() , "benchmark");

    return 0;
}
