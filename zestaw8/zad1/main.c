#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>

#define MAX_PATH 4096

struct image {
    int **matrix;
    int width;
    int height;
    int magnitude;
};

struct filter {
    double **matrix; // size x size matrix
    int size;
};

int threads_number;
struct image img;
struct filter filter;
struct image res;

int max(int x, int y) { return x > y ? x : y; }

int init(int argc, char *argv[]);

int parse_image(char *path);

int parse_filter(char *path);

int init_res_image();

int save_image(char *path);

int convolution(int x, int y);

int block();

void *thread_block(void *arg);

int interleaved();

void *thread_interleaved(void *arg);


int main(int argc, char *argv[]) {
    printf("Initializing the parameters ...\n");

    if (init(argc, argv)) { // read the args
        exit(EXIT_FAILURE);
    }

    printf("Initializing done ...\n");

    printf("Parsing the image ...\n");

    if (parse_image(argv[3])) { // read the image to the struct image
        exit(EXIT_FAILURE);
    }

    printf("Parsing image done ...\n");

    printf("Initializing result image...\n");

    if (init_res_image()) {
        exit(EXIT_FAILURE);
    }

    printf("Initializing done ...\n");

    printf("Parsing the filter ...\n");

    if (parse_filter(argv[4])) { // read the filter to the struct filter
        exit(EXIT_FAILURE);
    }

    printf("Parsing filter done ...\n");

    if (strcmp(argv[2], "block") == 0) { // block mode
        printf("Starting to filtering image in block mode ...\n");
        if (block()) {
            exit(EXIT_FAILURE);
        }

    } else { // interleaved mode, it can be either of those two, because we checked correctness of the input in init method
        printf("Starting to filtering image in interleaved mode ...\n");
        if (interleaved()) {
            exit(EXIT_FAILURE);
        }
    }

    printf("Successfully filtered image ...\n");

    printf("Saving image to the file ...\n");

    if (save_image(argv[5])) { // save the image to the file
        exit(EXIT_FAILURE);
    }

    printf("Image successfully saved ...\n");

    return 0;
}


int init(int argc, char *argv[]) {

    char usage[MAX_PATH];
    sprintf(usage, "Usage: %s (Number of threads) (block/interleaved) "
                   "(Input image file name) (Input filter file name) (Output file name)\n", argv[0]);

    if (argc != 6) {
        fprintf(stderr, "Wrong number of parameters expected 5\n%s", usage);
        exit(EXIT_FAILURE);
    }

    if (sscanf(argv[1], "%d", &threads_number) != 1) {
        fprintf(stderr, "Second parameter should be integer\n%s", usage);
        exit(EXIT_FAILURE);
    }


    if (strcmp(argv[2], "block") != 0 && strcmp(argv[2], "interleaved") != 0) {
        fprintf(stderr, "Second parameter %s should be one of (block/interleaved)\n%s", argv[2], usage);
        exit(EXIT_FAILURE);
    }

    if (access(argv[3], F_OK) == -1) {
        fprintf(stderr, "Input image file name %s doesn't exists\n%s", argv[3], usage);
        exit(EXIT_FAILURE);
    }

    if (access(argv[4], F_OK) == -1) {
        fprintf(stderr, "Input filter file name %s doesn't exists\n%s", argv[4], usage);
        exit(EXIT_FAILURE);
    }

    if (access(argv[5], F_OK) == -1) { // we assume that the output file exists, but it will be overwrite anyway
        fprintf(stderr, "Output file name %s doesn't exists\n%s", argv[5], usage);
        exit(EXIT_FAILURE);
    }

    assert(threads_number > 0);

    return 0;
}

int parse_image(char *path) { // we expect greyscale image in .pgm format without the comment inside!
    FILE *fp;
    if (!(fp = fopen(path, "r"))) {
        fprintf(stderr, "Failed to open file %s \n", path);
        exit(EXIT_FAILURE);
    }

    char line[MAX_PATH];
    fgets(line, sizeof(line), fp);
    if (strcmp(line, "P2\n") == 0) {
        fclose(fp);
        fprintf(stderr, "Expected first line to be string `P2\n` but found `%s`\n", line);
        exit(EXIT_FAILURE);
    }

    fgets(line, sizeof(line), fp);
    if (sscanf(line, "%d %d", &img.width, &img.height) != 2) {
        fclose(fp);
        fprintf(stderr, "Expected second line to have two integer but found %s\n", line);
        exit(EXIT_FAILURE);
    }

    assert(img.width > 0 && img.height > 0);

    fgets(line, sizeof(line), fp);
    if (sscanf(line, "%d", &img.magnitude) != 1) {
        fclose(fp);
        fprintf(stderr, "Expected third line to have one integer but found %s\n", line);
        exit(EXIT_FAILURE);
    }

    if (img.magnitude != 255) {
        fclose(fp);
        fprintf(stderr, "Expected image to be greyscale (magnitude[third line] should equal 255 but found %d)\n",
                img.magnitude);
        exit(EXIT_FAILURE);
    }

    // if first three lines are valid, we finally parse the matrix


    img.matrix = calloc(img.height, sizeof(int *));
    for (int w = 0; w < img.height; ++w) img.matrix[w] = calloc(img.width, sizeof(int));

    for (int h = 0; h < img.height; ++h)
        for (int w = 0; w < img.width; ++w) {
            if (fscanf(fp, "%d ", &img.matrix[h][w]) != 1) {
                fclose(fp);
                fprintf(stderr, "Wrong file format, couldn't read matrix\n");
                exit(EXIT_FAILURE);
            } else if (img.matrix[h][w] < 0 || img.matrix[h][w] > 255) {
                fclose(fp);
                fprintf(stderr, "Wrong file format, given matrix values should be in range [0,255] but found %d\n",
                        img.matrix[h][w]);
                exit(EXIT_FAILURE);
            }
        }

    // reading image was successful

    fclose(fp);
    return 0;
}

int init_res_image() {
    res.height = img.height;
    res.width = img.width;
    res.magnitude = img.magnitude;
    res.matrix = calloc(res.height, sizeof(int *));
    for (int w = 0; w < res.height; ++w) res.matrix[w] = calloc(res.width, sizeof(int));
    return 0;
}

int parse_filter(char *path) { // we expect greyscale image in .pgm format without the comment inside!
    FILE *fp;
    if (!(fp = fopen(path, "r"))) {
        fprintf(stderr, "Failed to open file %s \n", path);
        exit(EXIT_FAILURE);
    }

    char line[MAX_PATH];

    fgets(line, sizeof(line), fp);
    if (sscanf(line, "%d", &filter.size) != 1) {
        fclose(fp);
        fprintf(stderr, "Expected first line to have one integer but found %s\n", line);
        exit(EXIT_FAILURE);
    }

    assert(filter.size > 0);

    filter.matrix = calloc(filter.size, sizeof(double *));

    for (int s = 0; s < filter.size; ++s) filter.matrix[s] = calloc(filter.size, sizeof(double));

    double counter = 0;

    for (int h = 0; h < filter.size; ++h)
        for (int w = 0; w < filter.size; ++w) {
            if (fscanf(fp, "%lf ", &filter.matrix[h][w]) != 1) {
                fclose(fp);
                fprintf(stderr, "Wrong file format, couldn't read matrix\n");
                exit(EXIT_FAILURE);
            }
            counter += filter.matrix[h][w];
        }

    if (fabs(counter - 1) > 1e-4) {
        fclose(fp);
        fprintf(stderr, "Matrix should sum to one, but found sum equal %lf\n", counter);
        exit(EXIT_FAILURE);
    }

    // reading filter was successful

    fclose(fp);
    return 0;
}

int save_image(char *path) {
    FILE *fp;
    if (!(fp = fopen(path, "w"))) {
        fprintf(stderr, "Failed to open file %s\n", path);
        exit(EXIT_FAILURE);
    }
    fprintf(fp, "P2\n");
    fprintf(fp, "%d %d\n", res.width, res.height);
    fprintf(fp, "%d\n", res.magnitude);
    for (int h = 0; h < res.height; ++h) {
        for (int w = 0; w < res.width; ++w) {
            if (w % 20 == 0) fprintf(fp, "\n");
            fprintf(fp, "%d ", res.matrix[h][w]);
        }
    }
    fclose(fp);
    return 0;
}

int convolution(int x, int y) {
    assert(x >= 0 && x < img.height);
    assert(y >= 0 && y < img.width);
    double sum = 0;
    for (int h = 0; h < filter.size; ++h) {
        for (int w = 0; w < filter.size; ++w) {
            int x_pos = max(1, x - (int) ceil(filter.size / 2.) + h + 2) - 1;
            int y_pos = max(1, y - (int) ceil(filter.size / 2.) + w + 2) - 1;
            sum += x_pos < img.height && y_pos < img.width ? img.matrix[x_pos][y_pos] * filter.matrix[h][w]
                                                           : 0; // if we are outside matrix do not sum
        }
    }

    int result = (int) round(sum) % img.magnitude; // rescale sum to interval [-255,255]

    return result < 0 ? result + img.magnitude : result; // rescale sum to interval [0,255]
}

int block() {
    struct timeval start;
    gettimeofday(&start, NULL);
    pthread_t tid[threads_number];
    int args[threads_number];

    for (int i = 0; i < threads_number; ++i) {
        args[i] = i;
        if (pthread_create(&tid[i], NULL, thread_block, &args[i]) == -1) {
            fprintf(stderr, "Failed to create thread\n");
            exit(EXIT_FAILURE);
        }
        printf("Successfully created thread %ld\n", tid[i]);

    }

    struct timeval result;
    void *res_ptr;

    for (int i = 0; i < threads_number; ++i) {

        if (pthread_join(tid[i], &res_ptr) != 0) {
            fprintf(stderr, "Failed to join thread\n");
            exit(EXIT_FAILURE);
        }
        result = (*(struct timeval *) res_ptr);

        printf("Filtering done by thread %ld, all lasts %ld.%06ld seconds\n", tid[i], result.tv_sec, result.tv_usec);

    }

    struct timeval end;
    gettimeofday(&end, NULL);
    timersub(&end, &start, &result);

    printf("Filtering done by all threads, all lasts %ld.%06ld seconds\n", result.tv_sec, result.tv_usec);

    return 0;
}

void *thread_block(void *arg) {
    struct timeval start;
    gettimeofday(&start, NULL);
    int k = *((int *) arg);
    int left = k * (int) ceil(res.width / (double) threads_number);
    int right = (k + 1) * (int) ceil(res.width / (double) threads_number);
    for (int h = 0; h < res.height; ++h) {
        for (int w = left; w < right; ++w) {
            res.matrix[h][w] = convolution(h, w);
        }
    }
    struct timeval end;
    gettimeofday(&end, NULL);
    struct timeval *result = calloc(1, sizeof(struct timeval));
    timersub(&end, &start, result);
    pthread_exit((void *) result);
}


int interleaved() {
    struct timeval start;
    gettimeofday(&start, NULL);
    pthread_t tid[threads_number];
    int args[threads_number];

    for (int i = 0; i < threads_number; ++i) {
        args[i] = i + 1;
        if (pthread_create(&tid[i], NULL, thread_interleaved, &args[i]) == -1) {
            fprintf(stderr, "Failed to create thread\n");
            exit(EXIT_FAILURE);
        }
        printf("Successfully created thread %ld\n", tid[i]);

    }

    struct timeval result;
    void *res_ptr;

    for (int i = 0; i < threads_number; ++i) {

        if (pthread_join(tid[i], &res_ptr) != 0) {
            fprintf(stderr, "Failed to join thread\n");
            exit(EXIT_FAILURE);
        }
        result = (*(struct timeval *) res_ptr);

        printf("Filtering done by thread %ld, all lasts %ld.%06ld seconds\n", tid[i], result.tv_sec, result.tv_usec);

    }

    struct timeval end;
    gettimeofday(&end, NULL);
    timersub(&end, &start, &result);

    printf("Filtering done by all threads, all lasts %ld.%06ld seconds\n", result.tv_sec, result.tv_usec);

    return 0;
}


void *thread_interleaved(void *arg) {
    struct timeval start;
    gettimeofday(&start, NULL);
    int k = *((int *) arg);
    for (int i = 0; i < res.height; i++) {
        for (int j = 0, x = 0; x < res.width; ++j, x = k + j * threads_number) {
            res.matrix[i][x] = convolution(i, x);
        }
    }
    struct timeval end;
    gettimeofday(&end, NULL);
    struct timeval *result = calloc(1, sizeof(struct timeval));
    timersub(&end, &start, result);
    pthread_exit((void *) result);
}