#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_PATH 4096

int main(int argc, char* argv[]) {
    char usage[MAX_PATH];
    sprintf(usage, "MASTER: usage: %s path_to_fifo \n", argv[0]);

    if(argc != 2){
        fprintf(stderr, "MASTER: Wrong input, expected 1 argument\n%s", usage);
        _exit(EXIT_FAILURE);
    }

    struct stat file_info;

    if (stat(argv[1], &file_info) == 0){
        printf("MASTER: Fifo %s already exists, stoping the program\n", argv[1]);
        _exit(EXIT_FAILURE);
    }

    if(mkfifo(argv[1], 0666) == -1 ){
        fprintf(stderr, "MASTER: Couldn't create fifo %s \n", argv[1]);
        _exit(EXIT_FAILURE);
    }

    char line[MAX_PATH];

    int file; 
     
    while (1){
        if((file = open(argv[1], O_RDONLY)) == -1) {
        printf("MASTER: Failed to read from fifo, stoping the program %s \n", argv[1]);
        _exit(EXIT_FAILURE);
        }
        if((read(file, line, sizeof(line))) > 0){
             printf("%s", line);
        }
        sleep(2);
        close(file);
    }
    return 0;
}
