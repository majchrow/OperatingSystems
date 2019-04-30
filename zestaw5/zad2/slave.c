#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>

#define MAX_PATH 4096

int main(int argc, char* argv[]) {
    srand(time(NULL));
    char usage[MAX_PATH];
    sprintf(usage, "SLAVE: usage: %s path_to_fifo integer\n", argv[0]);

    if(argc != 3){
        fprintf(stderr, "SLAVE: Wrong input, expected 2 arguments\n%s", usage);
        _exit(EXIT_FAILURE);
    }

    int repeats;
    if (sscanf(argv[2], "%d", &repeats) != 1) {
        fprintf(stderr, "SLAVE: Wrong input, third parameter should be integer\n");
        _exit(EXIT_FAILURE);
    }

    struct stat file_info;
    if (stat(argv[1], &file_info) != 0){
        fprintf(stderr, "SLAVE: Fifo %s doesn't exists, stoping the program\n", argv[1]);
        _exit(EXIT_FAILURE);
    }

    int fd = open(argv[1], O_WRONLY);

    char buffer[MAX_PATH];
    char date[MAX_PATH];

    for(int i = 0; i < repeats; ++i){
        FILE* to_date = popen("date", "r");
        fgets(date, sizeof(date), to_date);
        pclose(to_date);
        sprintf(buffer, "PID: %d, DATE: %s", getpid(), date);
        write(fd, buffer, sizeof(buffer));
        sleep(rand()%3 + 2);
    }

    close(fd);
    return 0;
}