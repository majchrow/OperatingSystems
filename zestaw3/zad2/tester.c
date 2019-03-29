#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <assert.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_PATH 4096
#define LOOP_COUNTER 50

const char format[] = "%Y-%m-%d_%H-%M-%S";



char *generate_random_string(int size) {
    assert(size > 0);
    char *arr = (char *) calloc(size, sizeof(char));
    int iterator, tmp_int;
    char tmp_char;
    for (iterator = 0; iterator < size - 2; ++iterator) {
        tmp_int = rand() % 62; // 10 numbers 26 small letters 26 capital betters
        if (tmp_int < 10) { // number 0-9
            tmp_char = (char) (tmp_int + '0');
        } else if (tmp_int < 36) { // small letters a-z
            tmp_char = (char) (tmp_int - 10 + 'a');
        } else { // capital letters A-Z
            tmp_char = (char) (tmp_int - 36 + 'A');
        }

        arr[iterator] = tmp_char;
    }
    arr[iterator] = '\0'; // end of string
    arr[iterator + 1] = '\n'; // new line
    return arr;
}


int main(int argc, char ** argv) {
    if (argc != 5) {
        fprintf(stderr, "4 parameter was expected\n");
        exit(EXIT_FAILURE);
    }

    FILE *fp = fopen(argv[1], "a+");

    if(!fp) {
        fprintf(stderr, "Failed to open file %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }


    int pmin, pmax, bytes;

    if (sscanf(argv[2], "%d", &pmin) != 1) {
        printf("Wrong input, second parameter should be integer\n");
        exit(EXIT_FAILURE);
    }

    if (sscanf(argv[2], "%d", &pmin) != 1) {
        printf("Wrong input, second parameter should be integer\n");
        exit(EXIT_FAILURE);
    }

    if ( sscanf(argv[3], "%d", &pmax) != 1) {
        printf("Wrong input, third parameter should be integer\n");
        exit(EXIT_FAILURE);
    }
    if ( sscanf(argv[4], "%d", &bytes) != 1) {
        printf("Wrong input, third parameter should be integer\n");
        exit(EXIT_FAILURE);
    }

    char to_write[MAX_PATH + bytes], tmp_time[MAX_PATH];
    srand(time(NULL));
    int sleep_time, iterator;

    for(iterator = 0; iterator < LOOP_COUNTER; ++iterator) {
        sleep_time = rand()%(pmax - pmin + 1) + pmin;
        sleep(sleep_time);

        char* rand_string = generate_random_string(bytes);

        time_t cur_time = time(NULL);

        strftime(tmp_time, MAX_PATH, format, localtime(&cur_time));

        sprintf(to_write, "%d %d %s %s", (int)getpid(), sleep_time, tmp_time, rand_string);

        fseek(fp, 0, SEEK_END);

        fprintf(fp, "%s\n", to_write);

        free(rand_string);

    }

    fclose(fp);
    return 0;
}
