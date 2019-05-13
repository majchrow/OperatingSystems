#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define MAX_PATH 4096
#define MAX_LINES 100
#define MAX_ARGS 10
#define MAX_PIPES 15
#define MIN_PIPES 5

char *parsed[MAX_PIPES][MAX_ARGS];

int parse_row(char *pre_string) {

    char *tmp_arr[MAX_PIPES];
    char *tmp;
    int pipes = 0;
    if((tmp = strtok(pre_string, "|"))){
        tmp_arr[pipes] = tmp;
    }

    for(pipes = 1, tmp = strtok(NULL, "|"); pipes < MAX_PIPES && tmp; ++pipes, tmp = strtok(NULL, "|")){
        tmp_arr[pipes] = tmp;
    }

    if (pipes < MIN_PIPES) {
        fprintf(stderr, "Expected at least %d pipes in line: \n%s", MIN_PIPES, pre_string);
        _exit(EXIT_FAILURE);
    }

    for(int i = 0; i < pipes; ++i){
        tmp = strtok(tmp_arr[i], " ");
        int j = -1;
        parsed[i][++j] =  tmp;
        while((tmp = strtok(NULL, " "))){
            parsed[i][++j] = tmp;
        }
        parsed[i][++j] = NULL;
    }

    return pipes;
}

int process_line(char *line) {
    int pipes_number = parse_row(line);
    int pipes[2][2];
    int iterator;
    for(iterator = 0; iterator < pipes_number; ++iterator) {
        if(iterator > 1) {
            close(pipes[iterator%2][0]);
            close(pipes[iterator%2][1]);
        }
        pipe(pipes[iterator%2]);
        int pid = fork();
        if(pid == 0) { // child
            if(iterator != pipes_number - 1) {
                close(pipes[iterator%2][0]);
                dup2(pipes[iterator%2][1], 1);
                close(pipes[iterator%2][1]);
            }
            if(iterator != 0) {
                close(pipes[1 - iterator%2][1]);
                dup2(pipes[1 - iterator%2][0], 0);
                close(pipes[1 - iterator%2][0]);
            }
            execvp(parsed[iterator][0], parsed[iterator]);
        }
    }

    close(pipes[iterator%2][0]);
    close(pipes[iterator%2][1]);

    for(int i = 0; i < pipes_number; ++i){
        wait(NULL);
    }

    return 0;
}

int interpreter(char *path) {
    FILE *fp;
    if (!(fp = fopen(path, "r"))) {
        fprintf(stderr, "Failed to open file %s \n", path);
        _exit(EXIT_FAILURE);
    }
    char line[MAX_PATH];
    int line_counter = 0;

    while (line_counter < MAX_LINES && fgets(line, sizeof(line), fp)) {
        char* tmp = strtok(line, "\n"); // remove \n if there exists one
        printf("Starting to process to process line: %s\n", tmp);
        if (process_line(tmp)) {
            fprintf(stderr, "Failed processing line: %s\n", tmp);
            _exit(EXIT_FAILURE);
        }
        line_counter++;
    }
    fclose(fp);
    return 0;
}

int main(int argc, char *argv[]) {
    char usage[MAX_PATH];
    sprintf(usage, "Usage: %s path_to_input_file\n", argv[0]);

    if (argc != 2) {
        fprintf(stderr, "Wrong input, expected 2 argument\n%s", usage);
        _exit(EXIT_FAILURE);
    }

    if (interpreter(argv[1])) {
        fprintf(stderr, "Interpreting file %s failed\n", argv[1]);
        _exit(EXIT_FAILURE);
    }

    return 0;
}
