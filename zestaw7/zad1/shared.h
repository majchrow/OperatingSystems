// SHARED MACROS FOR trucker and loader

#ifndef SHARED_H
#define SHARED_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
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

#define MAX_PATH 4096
#define MEMORY_KEY 0x123 // Key to access shared memory
#define SEM_KEY 0x124 // Key to access semaphores
#define MEMORY_PATH "./shared.h" // Path to access shared memory
#define MAX_BELT_SIZE 1000  // Maximum number of belt_size
#define NUMBER_OF_CYCLES 0 // Number of cycles that each worker will do (0 if infinite loop)

enum sem{ BINARY, UF_SIZE, OF_TRUCK, OF_SIZE, OF_CAP }; // alias for semaphores, to make it more readable

struct sembuf binary_down = {.sem_num = BINARY, .sem_op = -1, .sem_flg = 0}; // for worker that can push to the queue at the same time and for printing belt state
struct sembuf binary_up   = {.sem_num = BINARY, .sem_op =  1, .sem_flg = 0}; // for worker that can push to the queue at the same time and for printing belt state

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
    struct package arr[MAX_BELT_SIZE];
};

#endif // SHARED_H
