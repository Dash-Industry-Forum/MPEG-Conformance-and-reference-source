#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>   //for PRIx64 macros
#include <time.h>
#include <sys/time.h>
#include <signal.h>

#include "hashtable.h"
#include "hashtable_str.h"

void _handle_term_signals(int sig);
static int _app_running = 1;

int main(int argc, char** argv)
{
    int64_t iterations = 0;
    while(_app_running)
    {
        hashtable_t* h = hashtable_new(hashtable_hashfn_char, hashtable_eqfn_char);
        int i;
        int n;
        int hash_size = 10000;
        int num_repeats = 100000;
        int len = 16;
        char** keys = (char**)malloc(hash_size * sizeof(char*));
        char** values = (char**)malloc(hash_size * sizeof(char*));

        for (i = 0; i < hash_size; i++)
        {
            keys[i] = (char*)malloc(len * sizeof(uint8_t));
            values[i] = (char*)malloc(len * sizeof(uint8_t));
            snprintf(keys[i], len, "key=%d", rand());
            snprintf(values[i], len, "value=%d", rand());

            hashtable_insert(h, keys[i], values[i]);
        }

        for (n = 0; n < num_repeats; n++)
        {
            i = rand() % hash_size;
        }
        for (n = 0; n < num_repeats; n++)
        {
            i = rand() % hash_size;
            char* v = (char*)hashtable_search(h, keys[i]);
        }
        for (n = 0; n < num_repeats; n++)
        {
            i = rand() % hash_size;
            char* v = (char*)hashtable_remove(h, keys[i]);
            free(v);

            keys[i] = (char*)malloc(len * sizeof(uint8_t));
            values[i] = (char*)malloc(len * sizeof(uint8_t));
            snprintf(keys[i], len, "key=%d", rand());
            snprintf(values[i], len, "value=%d", rand());

            hashtable_insert(h, keys[i], values[i]);
        }
        //1==free values
        hashtable_free(h, 1);
        free(keys);
        free(values);
        iterations++;
    }
    fprintf(stdout, "%"PRId64" iterations\n", iterations);
    return 0;
}

void _handle_term_signals(int sig)
{
    if ( (sig == SIGINT) || (sig == SIGTERM) )
    {
        _app_running = 0;
    }
    signal(sig, _handle_term_signals);
}
