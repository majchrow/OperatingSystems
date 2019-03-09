#include "library.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct wrapped *create(int number_of_blocks) { // Creating array with given number_of_blocks
    assert(number_of_blocks > 0);
    struct wrapped *arr = calloc(1, sizeof(struct wrapped));
    arr->number_of_blocks = number_of_blocks;
    arr->blocks = calloc(number_of_blocks, sizeof(char *));
    int i = 0;
    for(i = 0; i < number_of_blocks; ++i) arr->blocks[i] = NULL;
    arr->indicator = -1;
    arr->current_dir = NULL;
    arr->current_file = NULL;
    return arr;
}

void set_dir(struct wrapped *arr, char *dir_name) { // Setting current directory to search
    assert(arr != NULL);
    if (dir_name == NULL) {
        fprintf(stderr, "Couldn't set current directory to search");
        return;
    }
    free(arr->current_dir);
    arr->current_dir = calloc(strlen(dir_name),sizeof(char));
    arr->current_dir = dir_name;
}

void set_file(struct wrapped *arr, char *file_name) { // Setting current file to search
    assert(arr != NULL);
    if (file_name == NULL) {
        fprintf(stderr, "Couldn't set current file to search");
        return;
    }
    free(arr->current_file);
    arr->current_file = calloc(strlen(file_name),sizeof(char));
    arr->current_file = file_name;
}


void execute_search(struct wrapped *arr) { // Executing find search with current directory and file to search
    assert(arr != NULL);
    if (arr->current_file == NULL || arr->current_dir == NULL) {
        fprintf(stderr, "Set directory and file before executing search");
        return;
    }
    char* cmd = calloc(strlen("find ") + strlen(arr->current_dir) + strlen(" -name ") + strlen(arr->current_file) + strlen(" > tmp.txt") + strlen(" 2> /dev/null"), sizeof(char));
    strcpy(cmd, "find ");
    strcat(cmd, arr->current_dir);
    strcat(cmd, " -name ");
    strcat(cmd, arr->current_file);
    strcat(cmd, " > tmp.txt");
    strcat(cmd, " 2> /dev/null");
    system(cmd);
    free(cmd);
}

int add_block(struct wrapped *arr) { // Allocating tmp.txt content into to arr->blocks at first free index
    assert(arr != NULL);
    int tmp_indicator;
    for(tmp_indicator = 0;tmp_indicator < arr->number_of_blocks; ++tmp_indicator){ // searching for first free block
        if(arr->blocks[tmp_indicator] == NULL){
            break;
        }
    }
    if(tmp_indicator > arr->number_of_blocks){
        fprintf(stderr, "All the blocks are occupied, you can't add more blocks");
        return -1;
    };

    FILE *fp = fopen ("tmp.txt" ,"rb");
    if(!fp){
        fprintf(stderr, "Couldn't read file tmp.txt");
        return -1;
    }

    fseek(fp , 0L , SEEK_END);
    long lSize = ftell(fp);
    rewind(fp);
    arr->blocks[tmp_indicator] = calloc(1, lSize+1);

    if(!arr->blocks[tmp_indicator]){
        fclose(fp);
        fprintf(stderr, "Memory allocating at block %d failed", tmp_indicator);
        exit(1);
    }

    if(fread(arr->blocks[tmp_indicator], lSize, 1 , fp)!=1){
        fclose(fp);
        free(arr->blocks[tmp_indicator]);
        fprintf(stderr, "Coping file into to block %d failed", tmp_indicator);
        exit(1);
    }

    fclose(fp);
    arr->indicator = tmp_indicator;
    return tmp_indicator;

}

void clear_block(struct wrapped *arr, int index) { // Freeing memory from arr at block index index
    if (index < 0) {
        fprintf(stderr, "Clearing array on index %d failed", index);
        return;
    }

    if (arr == NULL || arr->blocks[index] == NULL) {
        fprintf(stderr, "Warrning, there is nothing to clear at index %d", index);
        return;
    }

    free(arr->blocks[index]);
    arr->blocks[index] = NULL;
}

void delete_array(struct wrapped *arr) { // Freeing memory from whole array
    if (arr == NULL) {
        fprintf(stderr, "Clearing array failed");
        return;
    }
    int index;
    for (index = 0; index < arr->number_of_blocks; --index) {
        if(arr->blocks[index] != NULL)
            clear_block(arr, index);
    }
    free(arr);
}

