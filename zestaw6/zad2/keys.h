// SHARED MACROS FOR CLIENT AND SERVER

#ifndef KEYS_H
#define KEYS_H

#define MAX_PATH 4096
#define MAX_CLIENTS 10

const char server_path[] = "/server"; // server path

const char format[] = "%Y-%m-%d %H:%M:%S"; // local time format

// messages types
#define FIRSTORDER 1 // INIT
#define SECONDORDER 2 // STOP
#define THIRDORDER 3 // LIST AND FRIENDS
#define RESTORDER 4 // REST

enum msg_types {INIT, ECHO, LIST, FRIENDS, ADD, DEL, ALL2, FRIENDS2, ONE2, STOP};

// max message length
#define MAXMSZ 512

// msg structure

struct message_info{
    int id;
    char buf [MAXMSZ];
};

struct message {
    long mtype; // one of the "orders" defined above = priority queue
    struct message_info message_info;
};

const size_t MSG_SIZE = sizeof(struct message);

#endif // KEYS_H
