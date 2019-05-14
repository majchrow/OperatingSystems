#include "shared.h"

int main(int argc, char* argv[]) {
    printf("Loader: Starting to initializing loader process...\n");

    printf("Loader: Checking for correct input...\n");

    char usage[MAX_PATH];
    sprintf(usage, "Usage: %s number_of_workers \n", argv[0]);

    if (argc != 2) {
        fprintf(stderr, "Loader: Wrong number of parameters\n%s", usage);
        exit(EXIT_FAILURE);
    }

    int workers; // number of workers

    if (sscanf(argv[1], "%d", &workers) != 1) {
        fprintf(stderr, "Loader: Second parameter should be integer\n%s", usage);
        exit(EXIT_FAILURE);
    }

    assert(workers > 0);

    printf("Loader: Input OK, checking if the shared memory is initialized...\n");


    if (shm_open(MEMORY_PATH, O_RDWR, 0666) == -1){ // checking if shared memory exists
        fprintf(stderr, "Loader: Shared memory not initialized, please check if the trucker is running\n");
        exit(EXIT_FAILURE);
    }

    printf("Loader: Shared-memory OK, starting to creating %d number of workers \n", workers);

    char *args[] = {"worker.out", NULL, NULL, NULL}; // worker program name | worker_weight | number_of_cycles (0 if infinite loop)
    char buffer1[MAX_PATH];
    char buffer2[MAX_PATH];
    sprintf(buffer1, "%d", NUMBER_OF_CYCLES);
    args[2] = buffer1;

    for(int worker = 1; worker <= workers; ++worker){
        pid_t parent = fork();
        if(!parent){ // child
            printf("Loader: Creating worker number %d \n", worker);
            sprintf(buffer2, "%d", worker);
            args[1] = buffer2;
            execv(args[0], args); // creating workers (since we are executing workers manually there is no need input checking in worker!)
            exit(0);
        }
    }
    wait(NULL);
    return 0;
}
