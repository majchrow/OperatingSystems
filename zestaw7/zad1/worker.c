#include "shared.h"

struct queue *belt = NULL; // shared belt cyclic queue

void cleanup();
long get_time();
void push(struct package);

int main(int argc, char* argv[]) {
    int my_pid = getpid();
    printf("Worker_pid_%d: Starting working ...\n", my_pid);

    int weight,    // weight for given worker
        cycle;     // cycle to make (one package = one cycle)

    sscanf(argv[1], "%d", &weight);
    sscanf(argv[2], "%d", &cycle);

    struct package pack = {.weight = weight, .pid = my_pid, .time = 0}; // time will be set when pushing

    if(!cycle) cycle = INT_MAX; // almost infinite loop

    key_t shared_key = ftok(MEMORY_PATH, MEMORY_KEY);
    int shared_mem = shmget(shared_key, 0, 0); // shared memory, loader checked already that it exists

    if( (belt = (struct queue*) shmat(shared_mem, (void*)0, 0)) == (void *) -1 ){ // shmat to attach to shared memory
        fprintf(stderr, "Worker_pid_%d: Failed to attach to shared memory\n", my_pid);
        exit(EXIT_FAILURE);
    }

    atexit(cleanup);

    key_t semaphore_key = ftok(MEMORY_PATH, SEM_KEY);

    int semaphores = semget(semaphore_key, 0, 0); // getting shared semaphores

    // prepering semaphore operations

   struct sembuf notify_belt =     {.sem_num = UF_SIZE, .sem_op = 1, .sem_flg = 0}; // notify trucker that package was put into the belt and can be taken
    struct sembuf push_belt[3] = { // pushing package to the belt if there is (!all three at the same time!) truck_size/belt_size/belt_size available in the belt
            {.sem_num = OF_SIZE,  .sem_op = -1,      .sem_flg = 0},
            {.sem_num = OF_CAP,   .sem_op = -weight, .sem_flg = 0},
            {.sem_num = OF_TRUCK, .sem_op = -weight, .sem_flg = 0}
    };

    for (int i = 0; i < cycle; ++i){

        printf("Worker_pid_%d: Doing my %d cycle\n", my_pid, i);

        printf("Worker_pid_%d: Waiting for belt to put my package(weight %d)\n", my_pid, weight);
        semop(semaphores, push_belt, 3); // push package to the belt, when it can be done
        semop(semaphores, &binary_down, 1); // take care that only one worker can put the package on the belt at the same time
        push(pack); //critical section for workers
        semop(semaphores, &binary_up, 1); // notify rest workers that they can put theirs packages
        semop(semaphores, &notify_belt, 1); // notify trucker that there is a package on the belt to be popped

    }
    return 0;

}

void cleanup(){
    if(belt){
        printf("Loader: Performing cleanup, detaching shared memory\n");
        if(shmdt(belt) == -1){ //detach from shared memory
            fprintf(stderr, "Loader: Failed to detach from shared memory\n");
            exit(EXIT_FAILURE);
        }
    }
}

long get_time() { // return current time in milliseconds
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    return time.tv_nsec / 1000; // nanoseconds / 1000
}

void push(struct package pack){
    pack.time = get_time();
    belt->arr[belt->end] = pack;
    belt->end = (belt->end + 1) % belt->size;
    belt->pack_num ++;
    belt->pack_cap += pack.weight;
    printf("Worker_pid_%d: Push: package_weight=%d, worker_pid=%d, current_time=%ld, queue_size_left=%d, queue_size_loaded=%d, queue_cap_left=%d, queue_cap_loaded=%d\n",
           pack.pid, pack.weight, pack.pid, pack.time, (belt->size - belt->pack_num), belt->pack_num, (belt->capacity - belt->pack_cap), belt->pack_cap);
}
