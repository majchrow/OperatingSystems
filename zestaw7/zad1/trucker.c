#include "shared.h"

struct queue *belt = NULL; // shared belt cyclic queue
int shared_mem = -1; // shared_memory id
int semaphores = -1; // set of semaphores
int current_truck_load = -1; // Variable to keep tracks of current load of the truck, only trucker uses it, so no semaphore is needed
int truck_size = -1; // maximum truck weight (todo this shouldn't be global)

int init(int argc, char *argv[]);
void cleanup();
void handler();
long get_time();
struct package pop();

struct belt_info{
    int size; // max_size of the queue
    int capacity; // max capacity of the queue
    int pack_num; // current size of the queue
    int pack_cap; // current ca[acity of the queue
};

int main(int argc, char *argv[]) {

    if (init(argc, argv)) {
        printf("Trucker: Initializing failed\n");
        exit(EXIT_FAILURE);
    }


    struct package last_pack; // last weight poped from the queue;
    struct sembuf wait_belt =     {.sem_num = UF_SIZE,  .sem_op = -1,         .sem_flg = 0}; // get package from the belt if there is any
    struct sembuf truck_unload =  {.sem_num = OF_TRUCK, .sem_op = truck_size, .sem_flg = 0}; // signal to workers that truck has arrived
    struct sembuf release_belt[2] = { // signal to workers that we freed up 1 package with popped weight from belt
            {.sem_num = OF_SIZE, .sem_op = 1, .sem_flg = 0},
            {.sem_num = OF_CAP,  .sem_op = 0, .sem_flg = 0} // sem_op for this depends on the popping element
    };
    printf("Trucker: Trucker has started press CTRL-C to stop it\n");

    for(int truck_count = 1; truck_count < INT_MAX; ++truck_count){ // almost infinite loop
        current_truck_load = truck_size;
        printf("Trucker: Empty truck nr %d has arrived\n", truck_count);
        printf("Trucker: Waiting for truck nr %d to be loaded\n", truck_count);
        semop(semaphores, &truck_unload, 1); // notify workers that truck has arrived
        while(current_truck_load > 0){

            semop(semaphores, &wait_belt, 1); // wait for workers to put package on the belt
            semop(semaphores, &binary_down, 1); // it's not needed for program to work correctly, but it is needed for printing belt state!
            last_pack = pop(); // inside critical section due to printing belt state requirement
            struct belt_info current_belt_load = {.size = belt->size, .capacity = belt->capacity, .pack_num = belt->pack_num, .pack_cap = belt->pack_cap}; // copy the belt status
            semop(semaphores, &binary_up, 1);   // it's not needed for program to work correctly, but it is needed for printing belt state!
            release_belt[1].sem_op = last_pack.weight;
            current_truck_load -= last_pack.weight;
            printf("Trucker: Load: worker_pid=%d, package_weight=%d, time_diff=%ld, truck_size_left=%d, truck_size_loaded=%d, queue_size_left=%d, queue_size_loaded=%d, queue_cap_left=%d, queue_cap_loaded=%d\n",
                    last_pack.pid, last_pack.weight, (get_time() - last_pack.time), current_truck_load, truck_size - current_truck_load,
                    (current_belt_load.size - current_belt_load.pack_num), current_belt_load.pack_num,
                    (current_belt_load.capacity - current_belt_load.pack_cap), current_belt_load.pack_cap);
            semop(semaphores, release_belt, 2); // notify workers about popped package
        }
        printf("Trucker: Truck nr %d is full, leaving\n", truck_count);
        printf("Trucker: Unloading the truck nr %d\n", truck_count);
        printf("Trucker: Truck nr %d has been unloaded, going for next delivery\n", truck_count);
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
    //int truck_size,  // maximum truck weight (this shouldn't be global)
    int belt_size,     // maximum number of packages in the belt
        belt_capacity; // maximum belt weight

    if (sscanf(argv[1], "%d", &truck_size) != 1) {
        fprintf(stderr, "Trucker: Second parameter should be integer\n%s", usage);
        exit(EXIT_FAILURE);
    }

    if (sscanf(argv[2], "%d", &belt_size) != 1) {
        fprintf(stderr, "Trucker: Thrid parameter should be integer\n%s", usage);
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

    size_t queue_size = sizeof(struct queue); // size of the queue (shared belt memory)
    key_t memory_key = ftok(MEMORY_PATH, MEMORY_KEY);

    if ((shared_mem = shmget(memory_key, queue_size, IPC_CREAT | 0666)) == -1) { // creating shared memory
        fprintf(stderr, "Trucker: Failed to create shared memory\n");
        exit(EXIT_FAILURE);
    }

    if ((belt = (struct queue *) shmat(shared_mem, (void *) 0, 0)) == (void *) -1) { // shmat to attach to shared memory
        fprintf(stderr, "Trucker: Failed to attach to shared memory\n");
        exit(EXIT_FAILURE);
    }
    belt->size = belt_size;
    belt->capacity = belt_capacity;
    belt->end = 0;
    belt->start = 0;
    belt->pack_num = 0;
    belt->pack_cap = 0;

    printf("Trucker: Shared memory initialized, initializing semaphores... \n");

    key_t semaphore_key = ftok(MEMORY_PATH, SEM_KEY);

    semaphores = semget(semaphore_key, 5 ,IPC_CREAT | 0666); // 5 semaphores is needed to synchronize our task

    semctl(semaphores, BINARY, SETVAL, 1); // binary semaphore for workers critical section (push to the belt) (!used for trucker as well ONLY for printing belt state!)
    semctl(semaphores, UF_SIZE, SETVAL, 0); // underflow semaphore to avoid popping empty queue
    semctl(semaphores, OF_TRUCK, SETVAL, 0); // overflow semaphore to make sure that whole truck is loaded (0 because it will be set by semaphore when truck arrive)
    semctl(semaphores, OF_SIZE, SETVAL, belt_size); // overflow semaphore to make sure that there is no more than belt_size packages in the queue
    semctl(semaphores, OF_CAP, SETVAL, belt_capacity); // overflow semaphore to make sure that there is no more weight than there can be

    printf("Trucker: Semaphores initialized, starting main program \n");

    return 0;
}

void cleanup() {
    if (belt) {
        printf("Trucker: Exiting process, blocking belt semaphores for workers\n");

        semctl(semaphores, OF_SIZE, SETVAL, 0); // block worker push semaphore

        printf("Trucker: Belt blocked successfully, loading the rest packages from the belt\n");
        struct sembuf wait_belt = {.sem_num = UF_SIZE, .sem_op = -1, .sem_flg = 0};
        while(semctl(semaphores, UF_SIZE, GETVAL, 0) > 0){
            semop(semaphores, &wait_belt, 1);
            semop(semaphores, &binary_down, 1);
            struct package last_pack = pop(); // inside critical section due to printing belt state requirement
            struct belt_info current_belt_load = {.size = belt->size, .capacity = belt->capacity, .pack_num = belt->pack_num, .pack_cap = belt->pack_cap}; // copy the belt status
            semop(semaphores, &binary_up, 1);
            current_truck_load -= last_pack.weight;
            printf("Trucker: Load: worker_pid=%d, package_weight=%d, time_diff=%ld, truck_size_left=%d, truck_size_loaded=%d, queue_size_left=%d, queue_size_loaded=%d, queue_cap_left=%d, queue_cap_loaded=%d\n",
                   last_pack.pid, last_pack.weight, (get_time() - last_pack.time), current_truck_load, truck_size - current_truck_load,
                   (current_belt_load.size - current_belt_load.pack_num), current_belt_load.pack_num,
                   (current_belt_load.capacity - current_belt_load.pack_cap), current_belt_load.pack_cap);
        }

        printf("Trucker: No more packages left in the belt leaving\n");

        printf("Trucker: Performing cleanup in shared memory\n");
        if (shmdt(belt) == -1) { //detach from shared memory
            fprintf(stderr, "Trucker: Failed to detach from shared memory\n");
            exit(EXIT_FAILURE);
        }
        if (shared_mem) {
            shmctl(shared_mem, IPC_RMID, NULL); // destroy the shared memory
        }
        printf("Trucker: Cleaned successfully exiting the process\n");
    }
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

struct package pop(){
    struct package ret = belt->arr[belt->start];
    belt->start = (belt->start +1) % belt->size;
    belt->pack_num --;
    belt->pack_cap -= ret.weight;
    return ret;
}
