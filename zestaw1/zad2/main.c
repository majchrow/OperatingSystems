#include <sys/times.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/times.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "library.h"

#define AVERAGE_COUNT 100

struct test_time {
    long double real_time;
    long double user_time;
    long double system_time;
};

void start_clock();

void end_clock(struct test_time *tt);

void set_dir_test(struct wrapped *arr, char *dir);

void set_search_file_test(struct wrapped *arr, char *file);

void set_temporary_file_test(struct wrapped *arr, char *file);

void find_test(struct wrapped *arr);

void add_block_test(struct wrapped *arr);

void remove_block_test(struct wrapped *arr, int index);

struct wrapped *create_arr_test(int number_of_blocks);

void summary(struct test_time **tts);

void delete_tts(struct test_time **tts);


static clock_t st_time;
static clock_t en_time;
static struct tms st_cpu;
static struct tms en_cpu;


void start_clock() {
    st_time = times(&st_cpu);
}

void end_clock(struct test_time *tt) {
    en_time = times(&en_cpu);
    tt->real_time = (long double) (en_time - st_time) / sysconf(_SC_CLK_TCK);
    tt->user_time = (long double) (en_cpu.tms_utime - st_cpu.tms_utime) / sysconf(_SC_CLK_TCK);
    tt->system_time = (long double) (en_cpu.tms_stime - st_cpu.tms_stime) / sysconf(_SC_CLK_TCK);
}

void set_dir_test(struct wrapped *arr, char *dir) {
    set_dir(arr, dir);
}

void set_search_file_test(struct wrapped *arr, char *file) {
    set_search_file(arr, file);
}

void set_temporary_file_test(struct wrapped *arr, char *file) {
    set_temporary_file(arr, file);
}

void find_test(struct wrapped *arr) {
    execute_search(arr);
}

void remove_block_test(struct wrapped *arr, int index) {
    delete_array(index);
}

void add_block_test(struct wrapped *arr) {
    add_block(arr);
}

struct wrapped *create_arr_test(int number_of_blocks) {
    return create(number_of_blocks);
}

void summary(struct test_time **tts) {
    long double avg_real = 0, avg_user = 0, avg_system = 0;
    int time_index;
    for(time_index = 0; time_index < AVERAGE_COUNT; ++time_index){
     avg_real+=tts[time_index]->real_time;
     avg_user+=tts[time_index]->user_time;
     avg_system+=tts[time_index]->system_time;
    }
    avg_real/=AVERAGE_COUNT;
    avg_system/=AVERAGE_COUNT;
    avg_user/=AVERAGE_COUNT;
    printf("Test done with the following results: \nReal Time: %Lf, User Time %Lf, System Time %Lf\n", avg_real, avg_user, avg_system);
    delete_tts(tts);
}

void delete_tts(struct test_time **tts) {
    int time_index;
    for (time_index = 0; time_index < AVERAGE_COUNT; ++time_index){
        free(tts[time_index]);
    }
    free(tts);
}


int main(int argc, char *argv[]) {
    assert(argc > 1);

    const struct option long_options[] = { // Long options specified
            {"help",       0, NULL, 'h'},
            {"verbose",    0, NULL, 'v'},
            {"create",     1, NULL, 'c'},
            {"dir",        1, NULL, 'd'},
            {"searchf",    1, NULL, 's'},
            {"temporaryf", 1, NULL, 't'},
            {"find",       0, NULL, 'f'},
            {"add",        0, NULL, 'a'},
            {"remove",     1, NULL, 'r'},
            {NULL,         0, NULL, 0},
    };

    const char *const short_options = "d:s:t:r:c:hvfa"; // Short options specified

    struct wrapped *arr;
    struct test_time **tts;
    tts = calloc(AVERAGE_COUNT, sizeof(struct test_time *));

    int i, test_number, next_option, num;
    printf("Executing test:");
    for(i = 0; i<argc; ++i) {
        printf(" %s ", argv[i]);
    }

    printf(" %d times and averaging the results\n",AVERAGE_COUNT);

    for (test_number = 0;
         test_number < AVERAGE_COUNT; ++test_number) { // Doing the same test AVERAGE_COUNT times to average the test times
        start_clock();
        do {
            next_option = getopt_long(argc, argv, short_options, long_options, NULL);
            switch (next_option) {
                case 'h': // help menu
                    printf("%s", optarg);
                    break;
                case 'v': // verbose mode
                    printf("%s", optarg);
                    break;
                case 'c': // create arr
                    num = atoi(optarg);
                    arr = create_arr_test(num);
                    break;
                case 'd': // set dir
                    set_dir(arr, optarg);
                    break;
                case 's': // set search file
                    set_search_file(arr, optarg);
                    break;
                case 't': // set temporary file
                    set_temporary_file(arr, optarg);
                    break;
                case 'f': // execute find
                    find_test(arr);
                    break;
                case 'a': // add block to memory
                    add_block_test(arr);
                    break;
                case 'r': // remove block at index from memory
                    num = atoi(optarg);
                    remove_block_test(arr, num);
                    break;
                case '?': // user specified invalid option
                    break;
                case -1: // done with options
                    break;
                default: // something else: unexpected
                    abort();
            }
        } while (next_option != -1);
        tts[test_number] = calloc(1, sizeof(struct test_time));
        end_clock(tts[test_number]);
    }

    summary(tts);
    delete_array(arr);
    return 0;
}