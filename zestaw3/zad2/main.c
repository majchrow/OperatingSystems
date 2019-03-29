#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <assert.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_PATH 4096

const char format[] = "%Y-%m-%d_%H-%M-%S";

struct memory_block {
    char *memory;
    long size;
    time_t date;
};


int copy_to_memory(char *filename, struct memory_block *block) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Couldn't copy file\n");
        exit(EXIT_FAILURE);
    }

    fseek(fp, 0L, SEEK_END);
    long lSize = ftell(fp);
    if (lSize == 0) {
        fprintf(stderr, "Can't allocate empty file to block\n");
        exit(EXIT_FAILURE);
    }
    rewind(fp);
    block->memory = calloc(1, lSize + 1);
    block->size = lSize + 1;
    if (fread(block->memory, lSize, 1, fp) != 1) {
        free(block->memory);
        fprintf(stderr, "Coping file failed\n");
        fclose(fp);
        exit(EXIT_FAILURE);
    }
    fclose(fp);
    return 0;
}


void memory(char *filename, int seconds, int terminate_seconds) {
    FILE *fp = fopen(filename, "r+");

    if (!fp) {
        fprintf(stderr, "Couldn't open file %s", filename);
        exit(EXIT_FAILURE);
    }

    int number_of_copies = 0;
    int timeDone = 0;
    struct stat attr;
    struct memory_block *block = calloc(1, sizeof(struct memory_block));

    char tmp_file[MAX_PATH], tmp_time[MAX_PATH];

    if (stat(filename, &attr) != 0) {
        printf("Cannot find file.\n");
        exit(EXIT_FAILURE);
    }
    block->date = attr.st_mtime;

    copy_to_memory(filename, block);

    while (timeDone < terminate_seconds) {

        sleep(seconds);
        stat(filename, &attr);
        if (difftime(attr.st_mtime, block->date) > 0) {

            strftime(tmp_time, MAX_PATH, format, localtime(&block->date));

            sprintf(tmp_file, "./archiwum/%s_%s", filename, tmp_time);
            FILE *fp_write = fopen(tmp_file, "w+");

            if (!fp_write) {
                fprintf(stderr, "Couldn't open file %s", tmp_file);
                exit(EXIT_FAILURE);
            }

            if (fwrite(block->memory, sizeof(char), block->size, fp_write) != block->size) {
                fprintf(stderr, "Couldn't write to file %s", tmp_file);
                exit(EXIT_FAILURE);
            }
            fclose(fp_write);

            free(block->memory);

            block->date = attr.st_mtime;

            copy_to_memory(filename, block);

            ++number_of_copies;
        }

        timeDone += seconds;
    }

    fclose(fp);
    free(block->memory);
    free(block);
    exit(number_of_copies);
}


void disc(char *filename, int seconds, int terminate_seconds) {
    FILE *fp = fopen(filename, "r+");

    if (!fp) {
        fprintf(stderr, "Couldn't open file %s", filename);
        exit(EXIT_FAILURE);
    }


    int number_of_copies = 1;
    int timeDone = 0;
    struct stat attr;
    time_t actual_date;

    char tmp_file[MAX_PATH], tmp_time[MAX_PATH];

    if (stat(filename, &attr) != 0) {
        printf("Cannot find file.\n");
        exit(EXIT_FAILURE);
    }
    actual_date = attr.st_mtime;

    sprintf(tmp_file, "./archiwum/%s", filename);
    pid_t pid;
    pid = fork();
    if (pid == 0) {
        execlp("/bin/cp", "cp", filename, tmp_file, NULL);
    }


    while (timeDone < terminate_seconds) {

        sleep(seconds);
        stat(filename, &attr);
        if (difftime(attr.st_mtime, actual_date) > 0) {

            strftime(tmp_time, MAX_PATH, format, localtime(&actual_date));

            sprintf(tmp_file, "./archiwum/%s_%s", filename, tmp_time);

            pid = fork();

            if (pid == 0) {
                execlp("/bin/cp", "cp", filename, tmp_file, NULL);
            }

            actual_date = attr.st_mtime;


            ++number_of_copies;
        }

        timeDone += seconds;
    }

    fclose(fp);
    exit(number_of_copies);
}


int monitor(char *file, int watch_time, char *mode) {
    FILE *fp = fopen(file, "r");

    if (!fp) {
        fprintf(stderr, "Couldn't open file %s", file);
        exit(EXIT_FAILURE);
    }

    int file_seconds, i, status, iterator = 0;
    char filename[MAX_PATH];
    pid_t pid[MAX_PATH];
    pid_t tmp;

    while (fscanf(fp, "%s %d", filename, &file_seconds) == 2) {

        if (strcmp(mode, "disc") == 0) {
            tmp = fork();
            if (tmp == 0) {
                disc(filename, file_seconds, watch_time);
            } else {
                pid[iterator] = tmp;
            }
        } else { // mode = memory
            tmp = fork();
            if (tmp == 0) {
                memory(filename, file_seconds, watch_time);
            } else {
                pid[iterator] = tmp;
            }
        }
        iterator++;
    }

    sleep(watch_time);

    for (i = 0; i < iterator; ++i) {
        waitpid(pid[i], &status, 0);
        printf("Process %d created %d number of copies\n", pid[i], (status / 256));
    }

    fclose(fp);
    return 0;
}


int main(int argc, char *argv[]) {

    if (argc != 4) {
        fprintf(stderr, "3 parameter was expected\n");
        exit(EXIT_FAILURE);
    }

    if (strcmp(argv[3], "memory") != 0 && strcmp(argv[3], "disc") != 0) {
        fprintf(stderr, "Wrong mode was given: %s\n", argv[3]);
        exit(EXIT_FAILURE);
    }

    int terminate_seconds;

    if (sscanf(argv[2], "%d", &terminate_seconds) != 1) {
        printf("Wrong input, second parameter should be integer\n");
        exit(EXIT_FAILURE);
    }

    if (monitor(argv[1], terminate_seconds, argv[3])) {
        fprintf(stderr, "Program %s %s %s %s failed\n", argv[0], argv[1], argv[2], argv[3]);
        exit(EXIT_FAILURE);
    }

    return 0;
}
