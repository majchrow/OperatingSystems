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

#define MAX_PATH 4096

const char format[] = "%Y-%m-%d %H:%M:%S";
time_t global_data;
char global_operator;
char global_path[MAX_PATH];

int summarize(char* path, struct stat file_info){
    printf("path: %s \n", path);
    printf("file info: ");
    if(S_ISREG(file_info.st_mode)) {
        printf("regular_file ");
    }
    if(S_ISDIR(file_info.st_mode)){
        printf("directory ");
    }
    if(S_ISCHR(file_info.st_mode)){
        printf("character_device ");
    }
    if(S_ISBLK(file_info.st_mode)){
        printf("block_device ");
    }
    if(S_ISFIFO(file_info.st_mode)){
        printf("FIFO(named_pipe) ");
    }
    if(S_ISLNK(file_info.st_mode)){
        printf("symbolic_link ");
    }
    if(S_ISSOCK(file_info.st_mode)){
        printf("socket ");
    }
    printf("\n");
    printf("size %ld bytes\n",file_info.st_size);
    char time_buffer[MAX_PATH];
    strftime(time_buffer, MAX_PATH, format, localtime(&file_info.st_atime));
    printf("date of last access %s\n",time_buffer);
    strftime(time_buffer, MAX_PATH, format, localtime(&file_info.st_mtime));
    printf("date of last modification %s\n\n",time_buffer);
    return 1;
}

int fn_display(const char *fpath, const struct stat *sb,
               int typeflag, struct FTW *ftwbuf){
    char tmp_path[MAX_PATH];
    strcpy(tmp_path, fpath);
    if(!strcmp(fpath, global_path)){
        return 0;
    }

    switch(global_operator){
        case '=':
            if(difftime(global_data, sb->st_mtime) == 0)
                summarize(tmp_path, *sb);
            break;
        case '<':
            if(difftime(global_data, sb->st_mtime) > 0)
                summarize(tmp_path, *sb);
            break;
        case '>':
            if(difftime(global_data, sb->st_mtime) < 0)
                summarize(tmp_path, *sb);
            break;
        default:
            printf("Parsing operator fails %c differs from = < and > \n", global_operator);
            exit(0);
    }

    return 0;
}

int nftw_wrapper(char* current_path, char operator, time_t data) {
    global_operator = operator;
    global_data = data;
    char tmp_path[MAX_PATH];
    realpath(current_path,tmp_path);
    strcpy(global_path,tmp_path);
    if(!nftw(tmp_path, fn_display, 16, FTW_PHYS))
        return 0;
    return 1;
}


int dir_search(char* current_path, char operator, time_t data){

    DIR *dir = opendir(current_path);
    if(!dir){
        fprintf(stderr, "Couldn't open directory %s\n", current_path);
        return 0;
    }
    struct dirent* cur_dir;
    char tmp_path[MAX_PATH];
    struct stat file_info;
    char* absolutePath = realpath(current_path,NULL);

    for(cur_dir = readdir(dir); cur_dir != NULL; cur_dir = readdir(dir)){

        sprintf(tmp_path, "%s/%s", absolutePath, cur_dir->d_name);
        lstat(tmp_path, &file_info);

        if (strcmp(cur_dir->d_name, ".") == 0 || strcmp(cur_dir->d_name, "..") == 0) { // ommiting . and .. directories
            continue;
        }

        switch(operator){
            case '=':
                if(difftime(data, file_info.st_mtime) == 0)
                    if(!summarize(tmp_path, file_info))
                        return 0;
                break;
            case '<':
                if(difftime(data, file_info.st_mtime) > 0)
                    if(!summarize(tmp_path, file_info))
                        return 0;
                break;
            case '>':
                if(difftime(data, file_info.st_mtime) < 0)
                    if(!summarize(tmp_path, file_info))
                        return 0;
                break;
            default:
                printf("Parsing operator fails %c differs from = < and > \n", operator);
                return 0;
        }

        if(S_ISDIR(file_info.st_mode)){
            dir_search(tmp_path, operator, data);
        }
    }
    closedir(dir);
    return 1;
}


int main(int argc, char *argv[]) {
        if(argc != 5){
            fprintf(stderr, "Wrong input, excpected 4 arguments \n");
            return 1;
        }
	printf("Executing test:  %s %s %s %s %s \n\n", argv[0], argv[1], argv[2], argv[3], argv[4]);
    	const char *time_input = argv[4];
        struct tm tm;
        strptime(time_input, format, &tm);
        time_t date = mktime(&tm);

        char operator = argv[3][0];

        if(!strcmp(argv[1],"-dir")){
            if(!dir_search(argv[2],operator, date)){
                fprintf(stderr, "Program for input %s %s %s %s %s failed \n", argv[0], argv[1], argv[2], argv[3], argv[4]);
                return 1;
            }
        }

        else if(!strcmp(argv[1],"-nftw")){
            if(nftw_wrapper(argv[2], operator, date)){
                fprintf(stderr, "Program for input %s %s %s %s %s failed \n", argv[0], argv[1], argv[2], argv[3], argv[4]);
                return 1;
            }
        }else {
            fprintf(stderr, "Wrong input %s %s %s %s %s \n", argv[0], argv[1], argv[2], argv[3], argv[4]);
            return 1;
        }
    return 0;
}
