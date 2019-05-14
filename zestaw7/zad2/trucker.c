#include "shared.h"

struct queue *belt = NULL; // shared belt cyclic queue
int shared_mem = -1; // shared_memory id
int truck_count = 1; // count of the current_trucks loaded
sem_t* mutex = NULL; // binary semaphore for critical section
sem_t* empty = NULL; // prevent popping from empty blet
sem_t* full  = NULL; // prevent pushing on full belt

int init(int argc, char *argv[]);
void cleanup();
void handler();
long get_time();
void pop_and_load();

int main(int argc, char *argv[]) {

    if (init(argc, argv)) {
        printf("Trucker: Initializing failed\n");
        exit(EXIT_FAILURE);
    }



    printf("Trucker: Trucker has started press CTRL-C to stop it\n");

    printf("Trucker: Empty truck nr %d has arrived\n", truck_count);
    printf("Trucker: Waiting for truck nr %d to be loaded\n", truck_count);

    for(int loop = 0; loop < INT_MAX; ++loop){ // almost infinite loop

        sem_wait(empty); // wait till there is package in the belt
        sem_wait(mutex); // critical section
        pop_and_load();
        if(belt->truck_curr == belt->truck_size){ // truck is full, time to unload
            printf("Trucker: Truck nr %d is full, leaving\n", truck_count);
            printf("Trucker: Unloading the truck nr %d\n", truck_count);
            printf("Trucker: Truck nr %d has been unloaded, going for next delivery\n", truck_count);
            belt->truck_curr = 0;
            belt->truck_curr_worker = 0;
            printf("Trucker: Empty truck nr %d has arrived\n", ++truck_count);
            printf("Trucker: Waiting for truck nr %d to be loaded\n", truck_count);
        }
        sem_post(full); // notify workers there is one free space in the belt
        sem_post(mutex); // free critical section
    }
    return 0;
}

int init(int argc, char *argv[]) {
    printf("Trucker: Starting to initializing trucker process...\n");

    printf("Trucker: Checking for correct input...\n");

    char usage[MAX_PATH];
    sprintf(usage, "Usage: %s truck_size belt_size belt_capacity\n", argv[0]);

    if (argc != 4) {
        fprintf(stderr, "Trucker: Wrong number of parameters\n%s", usage);
        exit(EXIT_FAILURE);
    }
    int truck_size,    // maximum truck weight
        belt_size,     // maximum number of packages in the belt
        belt_capacity; // maximum belt weight

    if (sscanf(argv[1], "%d", &truck_size) != 1) {
        fprintf(stderr, "Trucker: Second parameter should be integer\n%s", usage);
        exit(EXIT_FAILURE);
    }

    if (sscanf(argv[2], "%d", &belt_size) != 1) {
        fprintf(stderr, "Trucker: Third parameter should be integer\n%s", usage);
        exit(EXIT_FAILURE);
    }

    if (sscanf(argv[3], "%d", &belt_capacity) != 1) {
        fprintf(stderr, "Trucker: Fourth parameter should be integer\n%s", usage);
        exit(EXIT_FAILURE);
    }

    assert(truck_size > 0 && belt_size > 0 && belt_capacity > 0 && belt_size < MAX_BELT_SIZE);

    printf("Trucker: Input OK, setting up cleanup functions...\n");

    if (atexit(cleanup)) {
        fprintf(stderr, "Trucker: Cannot set cleanup function\n");
        exit(EXIT_FAILURE);
    }

    if (signal(SIGINT, handler) == SIG_ERR) {
        fprintf(stderr, "Trucker: Couldn't set CTRL-C handler\n");
        exit(EXIT_FAILURE);
    }

    printf("Trucker: Cleanup functions set up, initializing shared memory...\n");


    if ((shared_mem = shm_open(MEMORY_PATH, O_RDWR | O_CREAT | O_EXCL, 0666)) == -1) { // creating shared memory
        fprintf(stderr, "Trucker: Failed to create shared memory\n");
        exit(EXIT_FAILURE);
    }

    ftruncate(shared_mem, sizeof(struct queue));

    if((belt = mmap(
            NULL,                    // address
            sizeof(struct queue),    // length
            PROT_READ | PROT_WRITE,  // read and write
            MAP_SHARED,              // shared mem
            shared_mem,              // file descriptor
            0                        // no offset
    )) == (void*) -1){
        fprintf(stderr, "Trucker: Failed to attach to shared memory\n");
        exit(EXIT_FAILURE);
    }

    belt->size = belt_size;
    belt->capacity = belt_capacity;
    belt->truck_size = truck_size;
    belt->end = 0;
    belt->start = 0;
    belt->pack_num = 0;
    belt->pack_cap = 0;
    belt->truck_curr = 0;
    belt->truck_curr_worker = 0;

    printf("Trucker: Shared memory initialized, initializing semaphores... \n");

    if((mutex = sem_open(MUTEX_SEM, O_CREAT, 0666, 1)) == SEM_FAILED){
        fprintf(stderr, "Trucker: Failed to create mutex semaphore\n");
        exit(EXIT_FAILURE);
    }

    if((empty = sem_open(EMPTY_SEM, O_CREAT, 0666, 0)) == SEM_FAILED){
        fprintf(stderr, "Trucker: Failed to create queue underflow semaphore\n");
        exit(EXIT_FAILURE);
    }

    if((full = sem_open(FULL_SEM, O_CREAT, 0666, belt_size)) == SEM_FAILED){
        fprintf(stderr, "Trucker: Failed to create queue  overflow semaphore\n");
        exit(EXIT_FAILURE);
    }

    printf("Trucker: Semaphores initialized, starting main program \n");

    return 0;
}

void cleanup() {
    printf("Trucker: Performing cleanup, sending SIGINT to all workers\n");
    char command[MAX_PATH] = "killall -SIGINT worker.out";
    system(command);
    sleep(3); // wait for workers to close semaphores
    printf("Trucker: Workers closed semaphores, loading rest of the belt\n");

    while(belt->start != belt->end){ // load rest of the belt
        pop_and_load();
    }

    printf("Trucker: Truck nr %d is loaded, no more packages in the belt, leaving\n", truck_count);
    printf("Trucker: Unloading the truck nr %d\n", truck_count);
    printf("Trucker: Truck nr %d has been unloaded, no more work to be done, cleaning memory\n", truck_count);


    if (belt) {
        munmap(belt, sizeof(belt));
    }
    if(mutex){
        sem_close(mutex);
    }
    if(empty){
        sem_close(full);
    }
    if(full){
        sem_close(empty);
    }
    shm_unlink(MEMORY_PATH);
    shm_unlink(MUTEX_SEM);
    shm_unlink(EMPTY_SEM);
    shm_unlink(FULL_SEM);
}

void handler() {
    printf("\nTrucker: CTRL-C received\n"); // atexit will perform cleanup
    exit(0);
}

long get_time() { // return current time in seconds (to the precision of milliseconds)
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    return time.tv_nsec / 1000; // nanoseconds / 1000
}

void pop_and_load(){
    // pop from the belt
    struct package pack = belt->arr[belt->start];
    belt->start = (belt->start +1) % belt->size;
    belt->pack_cap -= pack.weight;
    belt->pack_num --;

    // load to the truct
    belt->truck_curr += pack.weight;

    //print info about popped and loaded package
    printf("Trucker: Load: worker_pid=%d, package_weight=%d, time_diff=%ld, truck_size_left=%d, truck_size_loaded=%d, queue_size_left=%d, queue_size_loaded=%d, queue_cap_left=%d, queue_cap_loaded=%d\n",
           pack.pid, pack.weight, (get_time() - pack.time), belt->truck_size - belt->truck_curr, belt->truck_curr,
           (belt->size - belt->pack_num), belt->pack_num,
           (belt->capacity - belt->pack_cap), belt->pack_cap);
}
