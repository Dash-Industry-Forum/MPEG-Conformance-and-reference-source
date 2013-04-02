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
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>

#include "hashtable.h"
#include "hashtable_str.h"

#include "test_macros.h"

int verbose = 0;

#define strdup(a, b) a = (char*)malloc(strlen(b)+1); strcpy(a, b);

void func(void* item, void* arg)
{
    *((char *)item) =  *((char *)arg);
    fprintf( stderr, "value = %s, arg = %c\n",  (char* )( (char * )item ), *( (char * )arg));
}

START_TEST (test_hashtable_simple)
{
    hashtable_t* h = hashtable_new(hashtable_hashfn_char, hashtable_eqfn_char);
    char* k;
    char* v;
    char  r = 'y';
    strdup(k, "key1"); strdup(v, "value1"); 
    hashtable_insert(h, k, v);
    strdup(k, "key2"); strdup(v, "value2"); 
    hashtable_insert(h, k, v);
    
    v = (char*)hashtable_search(h, "key1");
    fail_unless2( v != NULL && strcmp(v, "value1") == 0, "got wrong element", "%s", v );
    v = (char*)hashtable_search(h, "key2");
    fail_unless2( v != NULL && strcmp(v, "value2") == 0, "got wrong element", "%s", v );

    hashtable_foreach( h, func, &r );

    v = (char*)hashtable_search(h, "key1");
    fail_unless2( v != NULL && strcmp(v, "yalue1") == 0, "never replaced anything", "%s", v );
    v = (char*)hashtable_search(h, "key2");
    fail_unless2( v != NULL && strcmp(v, "yalue2") == 0, "never replaced anything", "%s", v );

}
END_TEST

START_TEST (test_hashtable_replace)
{
    hashtable_t* h = hashtable_new(hashtable_hashfn_char, hashtable_eqfn_char);
    char* k;
    char* v;
    strdup(k, "key1"); strdup(v, "value1"); 
    hashtable_insert(h, k, v);
    strdup(k, "key2"); strdup(v, "value2"); 
    hashtable_insert(h, k, v);

    v = (char*)hashtable_remove(h, "key2");
    fail_unless( v != NULL && strcmp(v, "value2") == 0, "got wrong element" );
    free(v);
    
    strdup(k, "key2"); strdup(v, "value2 changed");
    hashtable_insert(h, k, v);

    v = (char*)hashtable_search(h, "key2");
    fail_unless( v != NULL && strcmp(v, "value2 changed") == 0, "got wrong element after replace" );

    //hashtable_remove(h, "key2");
    //v = (char*)hashtable_search(h, "key2");
    //fail_unless( v == NULL, "got wrong element" );
}
END_TEST

int64_t gettimeusec()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int64_t t = (int64_t)tv.tv_sec*1000000 + (int64_t)tv.tv_usec;
    return t;
}

START_TEST (test_hashtable_benchmark)
{
    hashtable_t* h = hashtable_new(hashtable_hashfn_char, hashtable_eqfn_char);
    int i;
    int n;
    int hash_size = 10000;
    int num_repeats = 100000;
    int len = 16;
    char** keys;
    char** values;
    int64_t t1, t2;
    
    keys = (char**)malloc(hash_size * sizeof(char*));
    values = (char**)malloc(hash_size * sizeof(char*));

    for (i = 0; i < hash_size; i++)
    {
        keys[i] = (char*)malloc(len * sizeof(uint8_t));
        values[i] = (char*)malloc(len * sizeof(uint8_t));
        snprintf(keys[i], len, "key=%d", rand());
        snprintf(values[i], len, "value=%d", rand());

        hashtable_insert(h, keys[i], values[i]);
    }

    t1 = gettimeusec();
    for (n = 0; n < num_repeats; n++)
    {
        i = rand() % hash_size;
    }
    t2 = gettimeusec();
    if (verbose) { printf("empty loop: %f /sec (%d repeats)\n", (1000000*(double)num_repeats)/(t2 - t1), num_repeats); }

    t1 = gettimeusec();
    for (n = 0; n < num_repeats; n++)
    {
        i = rand() % hash_size;
        char* v = (char*)hashtable_search(h, keys[i]);
        fail_unless2( (strcmp(values[i], v) == 0), "lookup failed", "%s != %s", v, values[i] );
    }
    t2 = gettimeusec();
    if (verbose) { printf("lookup loop: %f /sec (%d repeats, %d hashtable size)\n", (1000000*(double)num_repeats)/(t2 - t1), num_repeats, hashtable_count(h)); }

    t1 = gettimeusec();
    for (n = 0; n < num_repeats; n++)
    {
        i = rand() % hash_size;
        char* v = (char*)hashtable_remove(h, keys[i]);
        fail_unless2( (strcmp(values[i], v) == 0), "lookup failed", "%s != %s", v, values[i] );
        free(v);
        
        keys[i] = (char*)malloc(len * sizeof(uint8_t));
        values[i] = (char*)malloc(len * sizeof(uint8_t));
        snprintf(keys[i], len, "key=%d", rand());
        snprintf(values[i], len, "value=%d", rand());

        hashtable_insert(h, keys[i], values[i]);
    }
    t2 = gettimeusec();
    if (verbose) { printf("modify loop: %f /sec (%d repeats, %d hashtable size)\n", (1000000*(double)num_repeats)/(t2 - t1), num_repeats, hashtable_count(h) ); }

}
END_TEST

int main(int argc, char** argv)
{
    //plan_tests(1);
    int _testnum = 1;
    
    ok( test_hashtable_simple() , "simple");
    ok( test_hashtable_replace() , "replace");
    ok( test_hashtable_benchmark() , "benchmark");

    return 0;
}
