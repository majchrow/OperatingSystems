#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/times.h>


char *
generate_random_string(int size); // function to generate random string with size size, assumes that srand is setted
int generate_sys(char *file_name, int number_of_records,
                 int number_of_byte); // function to fill file with random strings using system functions
int generate_lib(char *file_name, int number_of_records,
                 int number_of_byte); // function to fill file with random strings using library functions
int comparator(char first_char,
               char second_char); // compare 2 strings treating them as unsigned char, and comparing ASCII code
int sort_file_sys(char *file_name, int number_of_records, int number_of_byte); // sorting file using system functions
int sort_file_lib(char *file_name, int number_of_records, int number_of_byte); // sorting file using library functions
int copy_file_sys(char *target_file, char *destination_file, int number_of_records,
                  int number_of_byte); // copying file using library functions
int copy_file_lib(char *target_file, char *destination_file, int number_of_records,
                  int number_of_byte); // copying file using library functions



static clock_t st_time;
static clock_t en_time;
static struct tms st_cpu;
static struct tms en_cpu;

void start_clock() { // Starting counting
    st_time = times(&st_cpu);
}

void end_clock() { // End of one time count
    en_time = times(&en_cpu);
   printf("Test done with the following results\nUser time: %Lf System time: %Lf\n\n",
    (long double) (en_cpu.tms_utime - st_cpu.tms_utime) / sysconf(_SC_CLK_TCK),
    (long double) (en_cpu.tms_stime - st_cpu.tms_stime) / sysconf(_SC_CLK_TCK));
}

char *generate_random_string(int size) {
    assert(size > 0);
    char *arr = (char *) calloc(size, sizeof(char));
    int iterator, tmp_int;
    char tmp_char;
    for (iterator = 0; iterator < size - 2; ++iterator) {
        tmp_int = rand() % 62; // 10 numbers 26 small letters 26 capital betters
        if (tmp_int < 10) { // number 0-9
            tmp_char = (char) (tmp_int + '0');
        } else if (tmp_int < 36) { // small letters a-z
            tmp_char = (char) (tmp_int - 10 + 'a');
        } else { // capital letters A-Z
            tmp_char = (char) (tmp_int - 36 + 'A');
        }

        arr[iterator] = tmp_char;
    }
    arr[iterator] = '\0'; // end of string
    arr[iterator + 1] = '\n'; // new line
    return arr;
}

int generate_sys(char *file_name, int number_of_records, int number_of_byte) {
    int real_byte_number = number_of_byte + 2; // adding \0 and \n characters
    int fd = open(file_name,  O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);

    if (fd == -1) {
        fprintf(stderr, "Couldn't read file %s\n", file_name);
        return 0;
    }

    int iterator;
    char *buffor;

    for (iterator = 0; iterator < number_of_records; ++iterator) {
        buffor = generate_random_string(real_byte_number);
        write(fd, buffor, real_byte_number);
        free(buffor);
    }
    close(fd);
    return 1;

}

int generate_lib(char *file_name, int number_of_records, int number_of_byte) {
    int real_byte_number = number_of_byte + 2; // adding \0 and \n characters
    FILE *fp = fopen(file_name, "w");

    if (!fp) {
        fprintf(stderr, "Couldn't read file %s\n", file_name);
        return 0;
    }
    int iterator;
    char *buffor;
    for (iterator = 0; iterator < number_of_records; ++iterator) {
        buffor = generate_random_string(real_byte_number);
        fwrite(buffor, sizeof(char), real_byte_number, fp);
        free(buffor);
    }
    fclose(fp);
    return 1;
}

int comparator(char first_char, char second_char) {
    return (unsigned char) first_char > (unsigned char) second_char;
}


int sort_file_sys(char *file_name, int number_of_records, int number_of_byte) {
    int real_byte_number = number_of_byte + 2; // adding \0 and \n characters
    int fd = open(file_name, O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        fprintf(stderr, "Couldn't read file %s\n", file_name);
        return 0;
    }
    long int offset = real_byte_number;
    int i, j;
    char *buffor1 = calloc(real_byte_number, sizeof(char));
    char *buffor2 = calloc(real_byte_number, sizeof(char));
    int pos;

    for (i = 0; i < number_of_records - 1; ++i) {
        lseek(fd, i * offset, SEEK_SET);
        if (read(fd, buffor1, real_byte_number) != real_byte_number) {
            return 0;
        }

        pos = i;
        for (j = i + 1; j < number_of_records; ++j) { // find maximum (stable comparator)
            lseek(fd, j * offset, SEEK_SET);
            if (read(fd, buffor2, real_byte_number * sizeof(char)) != real_byte_number) {
                return 0;
            }
            if (comparator(buffor1[0], buffor2[0])) {
                strcpy(buffor1, buffor2);
                pos = j;
            }
        }
        lseek(fd, i * offset, SEEK_SET);
        if (read(fd, buffor2, real_byte_number * sizeof(char)) != real_byte_number) {
            return 0;
        }

        if (pos != i) {
            lseek(fd, i * offset, SEEK_SET);
            if (write(fd, buffor1, real_byte_number * sizeof(char)) != real_byte_number) {
                return 0;
            }

            lseek(fd, pos * offset, SEEK_SET);

            if (write(fd, buffor2, real_byte_number * sizeof(char)) != real_byte_number) {
                return 0;
            }
        }
    }
    close(fd);
    free(buffor1);
    free(buffor2);
    return 1;
}

int sort_file_lib(char *file_name, int number_of_records, int number_of_byte) {
    int real_byte_number = number_of_byte + 2; // adding \0 and \n characters
    int fd = open(file_name, O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        fprintf(stderr, "Couldn't read file %s\n", file_name);
        return 0;
    }
    FILE *fp = fopen(file_name, "r+");
    if (!fp) {
        fprintf(stderr, "Couldn't read file %s\n", file_name);
        return 0;
    }
    long int offset = real_byte_number;
    int i, j;
    char *buffor1 = calloc(real_byte_number, sizeof(char));
    char *buffor2 = calloc(real_byte_number, sizeof(char));
    int pos;

    for (i = 0; i < number_of_records - 1; ++i) {
        fseek(fp, i * offset, 0);
        if (fread(buffor1, sizeof(char), real_byte_number, fp) != real_byte_number) {
            return 0;
        }
        pos = i;
        for (j = i + 1; j < number_of_records; ++j) { // find maximum (stable comparator)
            fseek(fp, j * offset, 0);
            if (fread(buffor2, sizeof(char), real_byte_number, fp) != real_byte_number) {
                return 0;
            }
            if (comparator(buffor1[0], buffor2[0])) {
                strcpy(buffor1, buffor2);
                pos = j;

            }
        }
        fseek(fp, i * offset, 0);
        if (fread(buffor2, sizeof(char), real_byte_number, fp) != real_byte_number) {
            return 0;
        }

        if (pos != i) {
            fseek(fp, i * offset, 0);
            if (fwrite(buffor1, sizeof(char), real_byte_number, fp) != real_byte_number) {
                return 0;
            }

            fseek(fp, pos * offset, 0);

            if (fwrite(buffor2, sizeof(char), real_byte_number, fp) != real_byte_number) {
                return 0;
            }
        }

    }
    fclose(fp);
    free(buffor1);
    free(buffor2);
    return 1;
}

int copy_file_sys(char *target_file, char *destination_file, int number_of_records, int number_of_byte) {
    int real_byte_number = number_of_byte + 2; // adding \0 and \n characters
    int fd_target = open(target_file, O_RDONLY, S_IRUSR | S_IWUSR);
    int fd_destination = open(destination_file, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd_target == -1) {
        fprintf(stderr, "Couldn't read file %s\n", target_file);
        return 0;
    }
    if (fd_destination == -1) {
        fprintf(stderr, "Couldn't write to file %s\n", destination_file);
        return 0;
    }
    int i;
    char *buffor = calloc(real_byte_number, sizeof(char));
    for (i = 0; i < number_of_records; ++i) { // find maximum (stable comparator)
        if (read(fd_target, buffor, real_byte_number * sizeof(char)) != real_byte_number) {
            return 0;
        }
        if (write(fd_destination, buffor, real_byte_number * sizeof(char)) != real_byte_number) {
            return 0;
        }
    }
    close(fd_target);
    close(fd_destination);
    free(buffor);
    return 1;
}

int copy_file_lib(char *target_file, char *destination_file, int number_of_records, int number_of_byte) {
    int real_byte_number = number_of_byte + 2; // adding \0 and \n characters
    FILE *fp_target = fopen(target_file, "r");
    FILE *fp_destination = fopen(destination_file, "w");
    if (!fp_target) {
        fprintf(stderr, "Couldn't read file %s\n", target_file);
        return 0;
    }

    if (!fp_destination) {
        fprintf(stderr, "Couldn't write to file %s\n", destination_file);
        return 0;
    }
    int i;
    char *buffor = calloc(real_byte_number, sizeof(char));
    for (i = 0; i < number_of_records; ++i) { // find maximum (stable comparator)

        if (fread(buffor, sizeof(char), real_byte_number, fp_target) != real_byte_number) {
            return 0;
        }
        if (fwrite(buffor, sizeof(char), real_byte_number, fp_destination) != real_byte_number) {
            return 0;
        }
    }
    fclose(fp_target);
    fclose(fp_destination);
    free(buffor);
    return 1;
}


int main(int argc, char* argv[]) {
    if(argc <6 || argc > 7){
        fprintf(stderr, "Wrong input!\n");
        return 1;
    }

    if(!strcmp(argv[1], "generate")){
        srand(time(NULL));
        if(!strcmp(argv[5], "sys")){
            if(!generate_sys(argv[2],atoi(argv[3]), atoi(argv[4]))){
                fprintf(stderr, "Generating file failed!\n");
                return 1;
            }
        } else if(!strcmp(argv[5], "lib")){
            if(!generate_lib(argv[2],atoi(argv[3]), atoi(argv[4]))){
                fprintf(stderr, "Generating file failed!\n");
                return 1;
            }
        } else {
            fprintf(stderr, "Wrong input!\n");
            return 1;
        }
        return 0;
    }

    printf("Executing test: ");
    int i;
    for(i = 0; i< argc; ++i){
        printf("%s ", argv[i]);
    }
    printf("\n");

    if(!strcmp(argv[1], "sort")){
        if(!strcmp(argv[5], "sys")){
            printf("Sorting file using system functions\nl");
            start_clock();
            if(!sort_file_sys(argv[2],atoi(argv[3]), atoi(argv[4]))){
                fprintf(stderr, "Sorting file failed!\n");
                return 1;
            }
        } else if(!strcmp(argv[5], "lib")){
            printf("Sorting file using library functions\n");
            start_clock();
            if(!sort_file_lib(argv[2],atoi(argv[3]), atoi(argv[4]))){
                fprintf(stderr, "Sorting file failed!\n");
                return 1;
            }
        } else {
            fprintf(stderr, "Wrong input!\n");
            return 1;
        }
        end_clock();
        return 0;
    }

    if(!strcmp(argv[1], "copy")){
        if(!strcmp(argv[6], "sys")){
            printf("Copying file using system functions\n");
            start_clock();
            if(!copy_file_sys(argv[2],argv[3], atoi(argv[4]), atoi(argv[5]))){
                fprintf(stderr, "Copying file failed!\n");
                return 1;
            }
        } else if(!strcmp(argv[6], "lib")){
            printf("Copying file using library functions\n");
            start_clock();
            if(!copy_file_lib(argv[2],argv[3], atoi(argv[4]), atoi(argv[5]))){
                fprintf(stderr, "Copying file failed!\n");
                return 1;
            }
        } else {
            fprintf(stderr, "Wrong input!\n");
            return 1;
        }
        end_clock();
        return 0;
    }

    return 1;
}