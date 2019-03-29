#define _XOPEN_SOURCE 500
#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <assert.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_PATH 4096

void summarize(char* path, pid_t pid){
    printf("\nPath: %s is being explored by process with PID %d\n", path, (int)pid);
    execl("/bin/ls", "ls", "-l", path, NULL);
}

int dir_search(char* current_path){
	
    DIR *dir = opendir(current_path);
    if(!dir){
        fprintf(stderr, "Couldn't open directory %s\n", current_path);
        return 1;
    }
    struct dirent* cur_dir;
    char tmp_path[MAX_PATH];
    struct stat file_info;
    for(cur_dir = readdir(dir); cur_dir != NULL; cur_dir = readdir(dir)){

        if (strcmp(cur_dir->d_name, ".") == 0 || strcmp(cur_dir->d_name, "..") == 0) { // ommiting . and .. directories
            continue;
        }

        sprintf(tmp_path, "%s/%s", current_path, cur_dir->d_name);
        lstat(tmp_path, &file_info);

        if(S_ISDIR(file_info.st_mode)){
            pid_t tmp_pid = fork();
            if(tmp_pid < 0) {
                fprintf(stderr, "Forking failed \n");
            }else if(tmp_pid == 0){
		dir_search(tmp_path);
                summarize(tmp_path, getpid());
            }else{wait(NULL);}
	}
    }
    closedir(dir);
    return 0;
}


int main(int argc, char *argv[]) {
        if(argc != 2){
            fprintf(stderr, "Wrong input, excpected 1 argument \n");
            return 1;
        }

	printf("Executing program:  %s %s \n", argv[0], argv[1]);

        if(dir_search(argv[1])){
            return 1;
        }
    return 0;
}
