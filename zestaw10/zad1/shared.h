// SHARED MACROS FOR CLIENT AND SERVER

#ifndef SHARED_H
#define SHARED_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <sys/epoll.h>

#define MAX_PATH 4096
#define MAX_CLIENTS 10 // max clients allowed
#define MAX_SIZE 32768 // 2**15 for max request size
#define TAG 1 // 1 byte for TAG in TLV
#define LENGTH 2 // 2 byte for VALUE in TLV
#define MAX_EVENTS 10 // max events for epoll
#define TRUE 1 // just for infinite loops
#define CRON 5 // one cycle for ping checking

enum msg_types {REGISTER, NAMETAKEN, CLIENTSEXCEED, OK, REQUEST, RESULT, PING, PONG, UNREGISTER};

struct Client {
    int client_fd;
    char client_name[10];
    int ping;
    int reserved;
};

struct request{
    char buffer[MAX_SIZE];
    int id;
};

#endif // SHARED_H
