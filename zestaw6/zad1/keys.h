// SHARED MACROS FOR CLIENT AND SERVER

#ifndef ZAD1_KEYS_H
#define ZAD1_KEYS_H

#define MAX_PATH 4096
#define MAX_CLIENTS 10

// low number keys generators
#define KEY0 0x123 // Server KEY

const char format[] = "%Y-%m-%d %H:%M:%S"; // local time format

// messages types
#define FIRSTORDER 1 // INIT
#define SECONDORDER 2 // STOP
#define THIRDORDER 3 // LIST AND FRIENDS
#define RESTORDER 4 // REST
#define BACKORDER 5 // TO SEND FROM SERVER TO CLIENT USING SERVER QUEUE

enum msg_types {INIT, ECHO, LIST, FRIENDS, ADD, DEL, ALL2, FRIENDS2, ONE2, STOP};

// max message length
#define MAXMSZ 512

// msg structure

struct message_info{
    int id;
    enum msg_types request; // the specified request
    char buf [MAXMSZ];
};

struct message {
    long mtype; // one of the "orders" defined above = priority queue
    struct message_info message_info;
};

const size_t MSG_SIZE = sizeof(struct message_info);

#endif //ZAD1_KEYS_H
