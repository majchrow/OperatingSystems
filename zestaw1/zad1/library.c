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
    for (i = 0; i < number_of_blocks; ++i) arr->blocks[i] = NULL;
    arr->indicator = -1;
    arr->current_dir = NULL;
    arr->search_file = NULL;
    arr->temporary_file = calloc(8, sizeof(char));
    strcpy(arr->temporary_file, "tmp.txt");
    return arr;
}

void set_dir(struct wrapped *arr, char *dir_name) { // Setting current directory to search
    assert(arr != NULL);
    if (dir_name == NULL) {
        fprintf(stderr, "Couldn't set current directory to search\n");
        return;
    }
    free(arr->current_dir);
    arr->current_dir = calloc(strlen(dir_name), sizeof(char));
    strcpy(arr->current_dir, dir_name);
}

void set_search_file(struct wrapped *arr, char *file_name) { // Setting current file to search
    assert(arr != NULL);
    if (file_name == NULL) {
        fprintf(stderr, "Couldn't set current file to search\n");
        return;
    }
    free(arr->search_file);
    arr->search_file = calloc(strlen(file_name), sizeof(char));
    strcpy(arr->search_file, file_name);
}

void set_temporary_file(struct wrapped *arr, char *file_name) { // Setting temporary output file name
    assert(arr != NULL);
    if (file_name == NULL) {
        fprintf(stderr, "Couldn't set temporary file\n");
        return;
    }
    free(arr->temporary_file);
    arr->temporary_file = calloc(strlen(file_name), sizeof(char));
    strcpy(arr->temporary_file, file_name);
}

void execute_search(
        struct wrapped *arr) { // Executing find search with current directory, file to search and output temporary file
    assert(arr != NULL);
    if (arr->search_file == NULL || arr->current_dir == NULL || arr->temporary_file == NULL) {
        fprintf(stderr, "Set directory and files before executing search\n");
        return;
    }
    int len = snprintf(NULL, 0, "find %s -name %s > %s 2>/dev/null", arr->current_dir, arr->search_file,
                       arr->temporary_file);
    char *cmd = calloc(len, sizeof(char));
    sprintf(cmd, "find %s -name %s > %s 2>/dev/null", arr->current_dir, arr->search_file, arr->temporary_file);
    system(cmd);
    free(cmd);
}

int add_block(struct wrapped *arr) { // Allocating current temporary file content into to arr->blocks at first free index
    assert(arr != NULL);
    assert(arr->temporary_file != NULL);
    int tmp_indicator;
    for (tmp_indicator = 0; tmp_indicator < arr->number_of_blocks; ++tmp_indicator) { // searching for first free block
        if (arr->blocks[tmp_indicator] == NULL) {
            break;
        }
    }
    if (tmp_indicator > arr->number_of_blocks) {
        fprintf(stderr, "All the blocks are occupied, you can't add more blocks\n");
        return -1;
    };

    FILE *fp = fopen(arr->temporary_file, "rb");
    if (!fp) {
        fprintf(stderr, "Couldn't read file %s\n", arr->temporary_file);
        return -1;
    }

    fseek(fp, 0L, SEEK_END);
    long lSize = ftell(fp);
    if (lSize == 0) {
        fprintf(stderr, "Can't allocate empty file to block %d\n", tmp_indicator);
        exit(1);
    }
    rewind(fp);
    arr->blocks[tmp_indicator] = calloc(1, lSize + 1);

    if (!arr->blocks[tmp_indicator]) {
        fclose(fp);
        fprintf(stderr, "Memory allocating at block %d failed\n", tmp_indicator);
        exit(1);
    }

    if (fread(arr->blocks[tmp_indicator], lSize, 1, fp) != 1) {
        fclose(fp);
        free(arr->blocks[tmp_indicator]);
        fprintf(stderr, "Coping file into to block %d failed\n", tmp_indicator);
        exit(1);
    }

    fclose(fp);
    arr->indicator = tmp_indicator;
    return tmp_indicator;
}

void clear_block(struct wrapped *arr, int index) { // Freeing memory from arr at block index index
    if (index < 0) {
        fprintf(stderr, "Clearing array on index %d failed\n", index);
        return;
    }

    if (arr == NULL || arr->blocks[index] == NULL) {
        fprintf(stderr, "Warrning, there is nothing to clear at index %d\n", index);
        return;
    }

    free(arr->blocks[index]);
    arr->blocks[index] = NULL;
}

void delete_array(struct wrapped *arr) { // Freeing memory from whole array
    if (arr == NULL) {
        fprintf(stderr, "Clearing array failed\n");
        return;
    }
    int index;
    for (index = 0; index < arr->number_of_blocks; ++index) {
        if (arr->blocks[index])
            clear_block(arr, index);
    }
    free(arr);
}

