#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <assert.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/time.h>

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

    fseek(fp, 0L , SEEK_END);
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

    pid_t pid;

    sprintf(tmp_file, "./archiwum/%s", filename);

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

int monitor(char *file, int watch_time, char *mode, int cpu_time_constraint, int vm_size_constraint) {
    FILE *fp = fopen(file, "r");

    if (!fp) {
        fprintf(stderr, "Couldn't open file %s", file);
        exit(EXIT_FAILURE);
    }

    int file_seconds, i, status, iterator = 0;
    char filename[MAX_PATH];
    pid_t pid[MAX_PATH];
    pid_t tmp;

    struct rusage previous_rusage;
    struct rusage current_rusage;
    struct timeval ru_stime;
    struct timeval ru_utime;


    getrusage(RUSAGE_CHILDREN, &previous_rusage);


    while (fscanf(fp, "%s %d", filename, &file_seconds) == 2) {

        if (strcmp(mode, "disc") == 0) {
            tmp = fork();
            if (tmp == 0) {

                struct rlimit cpu_time_limit;
                cpu_time_limit.rlim_cur = (rlim_t) cpu_time_constraint;
                cpu_time_limit.rlim_max = (rlim_t) cpu_time_constraint;

                struct rlimit vm_size_limit;
                vm_size_limit.rlim_cur = (rlim_t) vm_size_constraint;
                vm_size_limit.rlim_max = (rlim_t) vm_size_constraint;

                setrlimit(RLIMIT_AS, &vm_size_limit);
                setrlimit(RLIMIT_CPU, &cpu_time_limit);

                disc(filename, file_seconds, watch_time);
            } else {
                pid[iterator] = tmp;
            }
        } else { // mode = memory
            tmp = fork();
            if (tmp == 0) {

                struct rlimit cpu_time_limit;
                cpu_time_limit.rlim_cur = (rlim_t) cpu_time_constraint;
                cpu_time_limit.rlim_max = (rlim_t) cpu_time_constraint;

                struct rlimit vm_size_limit;
                vm_size_limit.rlim_cur = (rlim_t) vm_size_constraint;
                vm_size_limit.rlim_max = (rlim_t) vm_size_constraint;

                setrlimit(RLIMIT_AS, &vm_size_limit);
                setrlimit(RLIMIT_CPU, &cpu_time_limit);

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

        getrusage(RUSAGE_CHILDREN, &current_rusage);
        timersub(&current_rusage.ru_stime, &previous_rusage.ru_stime, &ru_stime);
        timersub(&current_rusage.ru_utime, &previous_rusage.ru_utime, &ru_utime);

        printf("User time: %f\n", ru_stime.tv_sec + ru_stime.tv_usec / 1000000.0);
        printf("System time: %f\n", ru_utime.tv_sec + ru_utime.tv_usec / 1000000.0);

        previous_rusage = current_rusage;
    }

    fclose(fp);
    return 0;
}


int main(int argc, char *argv[]) {

    if (argc != 6) {
        fprintf(stderr, "5 parameter was expected\n");
        exit(EXIT_FAILURE);
    }

    if (strcmp(argv[3], "memory") != 0 && strcmp(argv[3], "disc") != 0) {
        fprintf(stderr, "Wrong mode was given: %s\n", argv[3]);
        exit(EXIT_FAILURE);
    }

    int terminate_seconds, cpu_time_constraint, vm_size_constraint;

    if (sscanf(argv[2], "%d", &terminate_seconds) != 1) {
        printf("Wrong input, second parameter should be integer\n");
        exit(EXIT_FAILURE);
    }

    if (sscanf(argv[4], "%d", &cpu_time_constraint) != 1) {
        printf("Wrong input, fourth parameter should be integer\n");
        exit(EXIT_FAILURE);
    }

    if (sscanf(argv[5], "%d", &vm_size_constraint) != 1) {
        printf("Wrong input, fifth parameter should be integer\n");
        exit(EXIT_FAILURE);
    }
    if (monitor(argv[1], terminate_seconds, argv[3], cpu_time_constraint, vm_size_constraint)) {
        fprintf(stderr, "Program %s %s %s %s %s %s failed\n", argv[0], argv[1], argv[2], argv[3], argv[4], argv[5]);
        exit(EXIT_FAILURE);
    }

    return 0;
}