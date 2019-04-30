#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_PATH 4096

const char format[] = "%Y-%m-%d_%H-%M-%S";

struct memory_block {
    char *memory;
    long size;
    time_t date;
};

int number_of_copies = 0, iterator, status;
pid_t pid[MAX_PATH];
FILE* fp;
struct memory_block *block;
int on_duty;

void end(){
    int i;
    for (i = 0; i < iterator; ++i) {
        kill(pid[i], SIGTERM);
        waitpid(pid[i], &status, 0);
        printf("Process %d created %d number of copies\n", pid[i], (status / 256));
    }
    fclose(fp);
    exit(0);
}

void end_handler(int signum){
    int i;
    for (i = 0; i < iterator; ++i) {
        kill(pid[i], SIGINT);
    }
    fclose(fp);
    exit(0);
}

void toggle_handler(int signum){
    if(signum == SIGUSR1){ // start handler
        on_duty = 1;
    }else if(signum == SIGUSR2){ // stop handler
        on_duty = 0;
    }
}

void exit_handler(int signum){
    free(fp);
    free(block->memory);
    free(block);
    if(signum == SIGINT) printf("Process %d created %d number of copies\n", getpid(), number_of_copies);
    exit(number_of_copies);
}


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

void prepare_signals(){
    signal(SIGUSR1, toggle_handler); // start
    signal(SIGUSR2, toggle_handler); // stop
    signal(SIGTERM, exit_handler);
    signal(SIGINT, exit_handler);
}


void memory(char *filename, int seconds) {
    fp = fopen(filename, "r+");

    if (!fp) {
        fprintf(stderr, "Couldn't open file %s", filename);
        exit(EXIT_FAILURE);
    }

    number_of_copies = 0;

    block = calloc(1, sizeof(struct memory_block));
    struct stat attr;

    char tmp_file[MAX_PATH], tmp_time[MAX_PATH];

    if (stat(filename, &attr) != 0) {
        printf("Cannot find file.\n");
        exit(EXIT_FAILURE);
    }
    block->date = attr.st_mtime;

    copy_to_memory(filename, block);

    on_duty = 1;

    prepare_signals();

    while (1) {
        if(on_duty){
            sleep(seconds);
            stat(filename, &attr);
            if (difftime(attr.st_mtime, block->date) > 0) {

                strftime(tmp_time, 100, format, localtime(&block->date));

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
        }else{
            sleep(seconds);
        }


    }

}

int exist(int pid, pid_t* pid_arr, int number){
    int i;
    if(pid < 1 || !pid_arr || number < 1) return 0;
    for(i = 0; i < number; ++i){
        if(pid_arr[i] == pid) return 1;
    }
    return 0;
}

int monitor(char *file) {
    fp = fopen(file, "r");

    if (!fp) {
        fprintf(stderr, "Couldn't open file %s", file);
        exit(EXIT_FAILURE);
    }


    int file_seconds, i, pid_number;
    char filename[MAX_PATH], command[MAX_PATH], *file_names[MAX_PATH];
    pid_t tmp;

    while (fscanf(fp, "%s %d", filename, &file_seconds) == 2) {
        tmp = fork();
        if (tmp == 0) {
            memory(filename, file_seconds);
        } else {
            pid[iterator] = tmp;
            file_names[iterator] = calloc(1, sizeof(filename));
            strcpy(file_names[iterator], filename);
        }
        iterator++;
    }

    for(i =0; i< iterator; ++i){
        printf("PID %d IS WATCHING FILE %s \n", (int)pid[i], file_names[i]);
    }

    struct sigaction act;
    act.sa_handler = end_handler;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    sigaction(SIGINT, &act, NULL);

    while(scanf("%s", command)){
        if(!strcmp(command, "LIST")){
            for(i =0; i< iterator; ++i){
                printf("PID %d IS WATCHING FILE %s \n", (int)pid[i], file_names[i]);
            }
        } else if(!strcmp(command, "END")){
                end();

        }else if(!strcmp(command, "STOP")){
            scanf("%s", command);
            if(!strcmp(command, "ALL")){
                for(i =0; i< iterator; ++i){
                 kill(pid[i], SIGUSR2);
                }

            }else{
                pid_number = atoi(command);
                if(exist(pid_number, pid, iterator) > 0){
                    kill(pid_number, SIGUSR2);
                }
                else{
                    fprintf(stderr, "Wrong command, please give correct input \n");
                }
            }
        }else if(!strcmp(command, "START")){
            scanf("%s", command);
            if (!strcmp(command, "ALL")) {
                for(i =0; i< iterator; ++i){
                    kill(pid[i], SIGUSR1);
                }
            } else {
                pid_number = atoi(command);
                if (exist(pid_number, pid, iterator) > 0) {
                    kill(pid_number, SIGUSR1);
                } else {
                    fprintf(stderr, "Wrong command, please give correct input \n");
                }
            }
        }else{
            fprintf(stderr, "Wrong command, please give correct input \n");
        }
       // sigqueue(info->si_pid, signum, value);

    }
    return 0;
}

int main(int argc, char *argv[]) {

    if (argc != 2) {
        fprintf(stderr, "1 parameter is expected\n");
        exit(EXIT_FAILURE);
    }

    if (monitor(argv[1])) {
        fprintf(stderr, "Program %s %s  failed\n", argv[0], argv[1]);
        exit(EXIT_FAILURE);
    }

    return 0;
}