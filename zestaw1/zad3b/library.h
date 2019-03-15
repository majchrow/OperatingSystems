#ifndef ZAD1_LIBRARY_H
#define ZAD1_LIBRARY_H

struct wrapped{
    int number_of_blocks;
    char **blocks;
    char *current_dir;
    char *search_file;
    char *temporary_file;
    int indicator;

};

struct wrapped *create(int number_of_blocks);

void set_dir(struct wrapped *arr, char* dir_name);

void set_search_file(struct wrapped *arr, char *file_name);

void set_temporary_file(struct wrapped *arr, char *file_name);

void execute_search(struct wrapped *arr);

int add_block(struct wrapped *arr);

void clear_block(struct wrapped *arr, int index);

void delete_array(struct wrapped *arr);

#endif
