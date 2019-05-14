// SHARED MACROS FOR trucker and loader

#ifndef SHARED_H
#define SHARED_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <assert.h>
#include <signal.h>
#include <unistd.h>
#include <sys/sem.h>
#include <wait.h>
#include <limits.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <semaphore.h>

#define MAX_PATH 4096
#define MEMORY_PATH "/memory" // Path to access shared memory
#define MUTEX_SEM "/mutex"
#define FULL_SEM "/full"
#define EMPTY_SEM "/empty"
#define MAX_BELT_SIZE 1000  // Maximum number of belt_size
#define NUMBER_OF_CYCLES 0 // Number of cycles that each worker will do (0 if infinite loop)

struct package{
    int weight; // weight of the package
    int pid; // pid of the worker who put package on the belt
    long time; // load time to pass to the trucker
};

struct queue{
    int start; // pointer to start of the queue
    int end; // pointer to end of the queue
    int size; // max_size of the queue
    int capacity; // max capacity of the queue
    int pack_num; // current size of the queue
    int pack_cap; // current capacity of the queue
    int truck_size; // max truck size
    int truck_curr; // current truck size
    int truck_curr_worker; // current truck size info for worker
    struct package arr[MAX_BELT_SIZE];
};

#endif // SHARED_H
