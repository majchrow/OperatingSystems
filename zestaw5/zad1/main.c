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
    char post_string[strlen(pre_string) + 1];

    int i;
    post_string[0] = pre_string[0];
    int j = 1;
    for (i = 1; i < strlen(pre_string); ++i) {
        if (!(pre_string[i] == ' ' && (pre_string[i - 1] == '|' || pre_string[i + 1] == '|')) &&
            pre_string[i] != '\n') {
            post_string[j++] = pre_string[i];
        }
    }

    // post_string is ready, all the unnecessary white spaces was deleted, as well as \n sign
    int row_counter = 0;
    int args_counter;
    char *between;
    char *args;
    char *to_process[MAX_PIPES];
    between = strtok(post_string, "|");

    while (between != NULL) {
        to_process[row_counter] = between;
        between = strtok(NULL, "|");
        ++row_counter;
    }

    for (int row = 0; row < row_counter; ++row) {
        args = strtok(to_process[row], " ");
        args_counter = 0;
        while (args != NULL) {
            parsed[row][args_counter] = args;
            args = strtok(NULL, " ");
            ++args_counter;
        }
        parsed[row][args_counter] = NULL;
    }
    parsed[row_counter][0] = NULL;
    return row_counter;
}


int process_line(char *line) {
    int pipes = parse_row(line);
    if (pipes < MIN_PIPES) {
        fprintf(stderr, "Expected at least %d pipes in line: \n%s", MIN_PIPES, line);
        _exit(EXIT_FAILURE);
    }

    int current_descriptor[2];
    int previous_descriptor[2];
    if (pipe(current_descriptor) == -1) {
        fprintf(stderr, "Failed to create pipes \n");
        _exit(EXIT_FAILURE);
    }

    int iterator, status;
    for (iterator = 0; iterator < pipes ; iterator++) {
        pid_t pid = fork();
        if (pid < 0) {
            fprintf(stderr, "Failed to create fork \n");
            _exit(EXIT_FAILURE);
        }
        if (pid == 0) { // child

            if (iterator > 0) {
                dup2(previous_descriptor[0], STDOUT_FILENO);
                close(previous_descriptor[1]);
                close(previous_descriptor[0]);
            }
            if (iterator < pipes) {
                dup2(current_descriptor[1], STDIN_FILENO);
                close(current_descriptor[0]);
                close(current_descriptor[1]);
            }
            execvp(parsed[iterator][0], parsed[iterator]);
            _exit(127);
        } else { // parent
            waitpid(pid, &status, 0);
            if (iterator > 0) {
                close(previous_descriptor[0]);
                close(previous_descriptor[1]);
            }
            if (iterator < pipes) {
                previous_descriptor[0] = current_descriptor[0];
                previous_descriptor[1] = current_descriptor[1];
            }
        }
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
        printf("Starting to process to process line:\n%s", line);
        if (process_line(line)) {
            fprintf(stderr, "Failed processing line: \n%s", line);
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