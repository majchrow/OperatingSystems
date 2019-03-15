#include <sys/times.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/times.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <dlfcn.h>

#ifndef DLL

#include "library.h"

#endif


#define AVERAGE_COUNT 5

struct test_time {
    long double real_time;
    long double user_time;
    long double system_time;
};

void start_clock();

void end_clock(struct test_time *tt);

void summary(struct test_time **tts);

void delete_tts(struct test_time **tts);


static clock_t st_time;
static clock_t en_time;
static struct tms st_cpu;
static struct tms en_cpu;


void start_clock() { // Starting counting
    st_time = times(&st_cpu);
}

void end_clock(struct test_time *tt) { // End of one time count
    en_time = times(&en_cpu);
    tt->real_time = (long double) (en_time - st_time) / sysconf(_SC_CLK_TCK);
    tt->user_time = (long double) (en_cpu.tms_utime - st_cpu.tms_utime) / sysconf(_SC_CLK_TCK);
    tt->system_time = (long double) (en_cpu.tms_stime - st_cpu.tms_stime) / sysconf(_SC_CLK_TCK);
}

void summary(struct test_time **tts) { // Counting average of all times calculated
    long double avg_real = 0, avg_user = 0, avg_system = 0;
    int time_index;
    for (time_index = 0; time_index < AVERAGE_COUNT; ++time_index) {
        avg_real += tts[time_index]->real_time;
        avg_user += tts[time_index]->user_time;
        avg_system += tts[time_index]->system_time;
    }
    avg_real /= AVERAGE_COUNT;
    avg_system /= AVERAGE_COUNT;
    avg_user /= AVERAGE_COUNT;
    printf("Test done with the following results: \nReal Time: %Lf, User Time %Lf, System Time %Lf\n", avg_real,
           avg_user, avg_system);
}

void delete_tts(struct test_time **tts) {
    int time_index;
    for (time_index = 0; time_index < AVERAGE_COUNT; ++time_index) {
        free(tts[time_index]);
    }
    free(tts);
}



int main(int argc, char *argv[]) {
#ifdef DLL

void *handle = dlopen("./Libs/libsearchlib.so", RTLD_LAZY);
struct wrapped* (*create)(int) = dlsym(handle, "create");
void (*set_dir)(struct wrapped*, char*) = dlsym(handle, "set_dir");
void (*set_search_file)(struct wrapped*, char*) = dlsym(handle, "set_search_file");
void (*set_temporary_file)(struct wrapped*, char*) = dlsym(handle, "set_temporary_file");
int (*add_block)(struct wrapped*) = dlsym(handle, "add_block");
void (*clear_block)(struct  wrapped*, int) = dlsym(handle, "clear_block");
void (*delete_array)(struct wrapped*) = dlsym(handle, "delete_array");
void (*execute_search)(struct wrapped*) = dlsym(handle, "execute_search");

#endif

    assert(argc > 1);

    const struct option long_options[] = { // Long options specified for parser
            {"create",     1, NULL, 'c'},
            {"dir",        1, NULL, 'd'},
            {"searchf",    1, NULL, 's'},
            {"temporaryf", 1, NULL, 't'},
            {"find",       0, NULL, 'f'},
            {"add",        0, NULL, 'a'},
            {"remove",     1, NULL, 'r'},
            {NULL,         0, NULL, 0},
    };

    const char *const short_options = "d:s:t:r:c:hvfa"; // Short options specified for parser

    struct test_time **tts = NULL;
    tts = calloc(AVERAGE_COUNT, sizeof(struct test_time *));
    struct wrapped **arr;
    arr = calloc(AVERAGE_COUNT, sizeof(struct wrapped *));
    int i, test_number, next_option, num;
    printf("Executing test:"); // Printing information about the given test
    for (i = 0; i < argc; ++i) {
        printf(" %s ", argv[i]);
    }
    printf("\nMaking %d tests and taking average of that\n\n", AVERAGE_COUNT);


    for (test_number = 0; test_number <
                          AVERAGE_COUNT; ++test_number) { // Doing the same test AVERAGE_COUNT times to average the test times
        optind = 1;
        start_clock();
        do { // parse entire input
            next_option = getopt_long(argc, argv, short_options, long_options, NULL);
            switch (next_option) {
                case 'c': // create arr
                    assert(optarg);
                    num = atoi(optarg);
                    assert(num);
                    arr[test_number] = create(num);
                    break;
                case 'd': // set dir
                    assert(optarg);
                    set_dir(arr[test_number], optarg);
                    break;
                case 's': // set search file
                    assert(optarg);
                    set_search_file(arr[test_number], optarg);
                    break;
                case 't': // set temporary file
                    assert(optarg);
                    set_temporary_file(arr[test_number], optarg);
                    break;
                case 'f': // execute find
                    execute_search(arr[test_number]);
                    break;
                case 'a': // add block to memory
                    add_block(arr[test_number]);
                    break;
                case 'r': // remove block at index from memory
                    num = atoi(optarg);
                    clear_block(arr[test_number], num);
                    break;
                case '?': // user specified invalid option
                    assert(1);
                    break;
                case -1: // done with options
                    assert(1);
                    break;
                default: // something else: unexpected
                    assert(1);
                    abort();
            }
        } while (next_option != -1);
        tts[test_number] = calloc(1, sizeof(struct test_time));
        end_clock(tts[test_number]); // counting time per one full input parsing
    }

    for (test_number = 0; test_number < AVERAGE_COUNT; ++test_number) { // clearning array
        if (arr[test_number]) {
            delete_array(arr[test_number]);
        }
    }

    summary(tts); // printing average times
    delete_tts(tts); // clearing tts array

#ifdef DLL
    dlclose(handle);
#endif
    return 0;

}
