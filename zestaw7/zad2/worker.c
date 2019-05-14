#include "shared.h"

struct queue *belt = NULL; // shared belt cyclic queue
int shared_mem = -1; // shared_memory id
sem_t* mutex = NULL; // binary semaphore for critical section
sem_t* empty = NULL; // prevent popping from empty blet
sem_t* full  = NULL; // prevent pushing on full belt

void cleanup();
long get_time();
void push(struct package);
void handler();

int main(int argc, char* argv[]) {
    int my_pid = getpid();

    printf("Worker_pid_%d: Starting working ...\n", my_pid);

    int weight,    // weight for given worker
        cycle,     // cycle to make (one package = one cycle)
        curr_cycle; // current cycle
    sscanf(argv[1], "%d", &weight);
    sscanf(argv[2], "%d", &cycle);

    if(!cycle) cycle = INT_MAX; // almost infinite loop if cycle was set to 0

    if ((shared_mem = shm_open(MEMORY_PATH, O_RDWR, 0666)) == -1) { // accessing shared memory
        fprintf(stderr, "Worker_pid_%d: Failed to create shared memory\n", my_pid);
        exit(EXIT_FAILURE);
    }

    if((belt = mmap(
            NULL,                    // address
            sizeof(struct queue),    // length
            PROT_READ | PROT_WRITE,  // read and write
            MAP_SHARED,              // shared mem
            shared_mem,              // file descriptor
            0                        // no offset
    )) == (void*) -1){
        fprintf(stderr, "Worker_pid_%d: Failed to attach to shared memory\n", my_pid);
        exit(EXIT_FAILURE);
    }

    if((mutex = sem_open(MUTEX_SEM, 0)) == SEM_FAILED){
        fprintf(stderr, "Worker_pid_%d: Failed to open mutex semaphore\n", my_pid);
        exit(EXIT_FAILURE);
    }

    if((empty = sem_open(EMPTY_SEM, 0)) == SEM_FAILED){
        fprintf(stderr, "Worker_pid_%d: Failed to open queue underflow semaphore\n", my_pid);
        exit(EXIT_FAILURE);
    }

    if((full = sem_open(FULL_SEM, 0)) == SEM_FAILED){
        fprintf(stderr, "Worker_pid_%d: Failed to open queue overflow semaphore\n", my_pid);
        exit(EXIT_FAILURE);
    }

    atexit(cleanup);

    if (signal(SIGINT, handler) == SIG_ERR) {
        fprintf(stderr, "Worker_pid_%d: Couldn't set CTRL-C handler\n", my_pid);
        exit(EXIT_FAILURE);
    }

    curr_cycle = 1;
    struct package my_pack = {.weight = weight, .pid = my_pid, .time = 0}; // set time while push


    printf("Worker_pid_%d: Doing my %d cycle\n", my_pid, curr_cycle++);
    printf("Worker_pid_%d: Waiting for belt to put my package(weight %d)\n", my_pid, weight);


    for (int i = 0; i < cycle; ++i){ // main loop
        sem_wait(full); // if there is place in the queue try to push
        sem_wait(mutex); // enter to critical section
        if((belt->capacity - belt->pack_cap) >= weight && (belt->truck_size - belt->truck_curr_worker) >= weight){ // we can push package
            my_pack.time = get_time();
            push(my_pack);
            sem_post(empty);
            printf("Worker_pid_%d: Doing my %d cycle\n", my_pid, curr_cycle++);
            printf("Worker_pid_%d: Waiting for belt to put my package(weight %d)\n", my_pid, weight);
        }else{
            sem_post(full); // rollback, we didn't push to the queue due to the constraints
        }
        sem_post(mutex); // notify others about leaving critical section
    }
    return 0;

}


void handler() {
    printf("\nWorker_pid_%d: CTRL-C received\n", getpid()); // atexit will perform cleanup
    exit(0);
}

void cleanup(){
    printf("Worker_pid_%d: Performing cleanup, closing semaphores\n", getpid());
    if(mutex) {
        sem_close(mutex);
    }
    if(full){
        sem_close(full);
    }
    if(empty){
        sem_close(empty);
    }
    printf("Worker_pid_%d: Semaphores closed, exiting program\n", getpid());
}

long get_time() { // return current time in milliseconds
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    return time.tv_nsec / 1000; // nanoseconds / 1000
}

void push(struct package pack){
    // push to the queue
    belt->arr[belt->end] = pack;
    belt->end = (belt->end + 1) % belt->size;
    belt->pack_num ++;
    belt->pack_cap += pack.weight;
    belt->truck_curr_worker += pack.weight; // keep count of the trucker worker count

    // print info about pushed package
    printf("Worker_pid_%d: Push: package_weight=%d, worker_pid=%d, current_time=%ld, queue_size_left=%d, queue_size_loaded=%d, queue_cap_left=%d, queue_cap_loaded=%d\n",
           pack.pid, pack.weight, pack.pid, pack.time, (belt->size - belt->pack_num), belt->pack_num, (belt->capacity - belt->pack_cap), belt->pack_cap);
}
