#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <signal.h>
#include <time.h>
#include "keys.h"

int serverqid;
static int clients[MAX_CLIENTS]; // position in array = ClientID, value in array = Client Queue
static int friends[MAX_CLIENTS]; // friends array - we assume that there can be only one friend list per server

void cleanup() {
    printf("SERVER: Performing cleanup\nSERVER: Closing server process with id: %d\n", serverqid);
    if (msgctl(serverqid, IPC_RMID, NULL)) {
        fprintf(stderr, "SERVER: Failed to close queue\n");
        exit(EXIT_FAILURE);
    } else {
        printf("SERVER: Queue closed successfully\n");
    }
}

int handle_request(struct message req_msg) {
    struct message message_send;

    if (req_msg.message_info.request == INIT) {
        printf("SERVER: Received INIT from client, trying to register\n");
        int current_client;
        for (current_client = 0; current_client < MAX_CLIENTS &&
                                 clients[current_client] != -1; ++current_client) {} // find first free client

        if (current_client == MAX_CLIENTS) { // MAX_CLIENTS ALREADY
            fprintf(stderr, "SERVER: There is %d clients already, can't serve more\n", current_client);
            return 1;
        } else {
            int client_key;
            if (sscanf(req_msg.message_info.buf, "%d", &client_key) != 1) {
                fprintf(stderr, "SERVER: Received INIT without the key\n");
                return 1;
            }

            if ((clients[current_client] = msgget(client_key, 0)) < 0) { // open client queue
                fprintf(stderr, "SERVER %d: Failed to open slient-side queue\n", getpid());
                return 1;
            }
            message_send.mtype = BACKORDER; // passing mtype 5 to server to avoid deadlocks
            message_send.message_info.request = INIT;
            sprintf(message_send.message_info.buf, "%d", current_client);
            if (msgsnd(serverqid, &message_send, MSG_SIZE, 0) == -1) { // SEND BACK INIT WITH CLIENT_ID
                fprintf(stderr, "CLIENT %d: Sending message failed\n", getpid());
                return 1;
            }
            printf("SERVER: Successfully registered client with ID %d\n", current_client);
        }
    } else if (req_msg.message_info.request == ECHO) {
        printf("SERVER: Received ECHO from client, sending the response\n");
        char tmp_time[MAX_PATH];
        time_t rawtime;
        time(&rawtime);
        strftime(tmp_time, 100, format, localtime(&rawtime));
        char buffer[MAX_PATH];
        sprintf(buffer, "%s, CURRENT SERVER DATE: %s", req_msg.message_info.buf, tmp_time);
        message_send.mtype = RESTORDER;
        message_send.message_info.request = ECHO;
        strncpy(message_send.message_info.buf, buffer, MAXMSZ);
        if (msgsnd(clients[req_msg.message_info.id], &message_send, MSG_SIZE, 0) == -1) { // SEND BACK TO THE CLIENT WITH DATE
            fprintf(stderr, "SERVER: Failed responding for ECHO request\n");
            return 1;
        }
        printf("SERVER: Response: %s, was sent successfully\n", buffer);
    } else if (req_msg.message_info.request == LIST) {
        printf("SERVER: Received LIST from client, searching for active clients\n");
        int current_client;
        for (current_client = 0; current_client < MAX_CLIENTS; ++current_client) {
            if (clients[current_client] != -1) {
                printf("SERVER: Client Active: ID %d Queue %d\n", current_client, clients[current_client]);
            }
        }
        printf("SERVER: There is no more active clients\n");
    } else if (req_msg.message_info.request == FRIENDS) {
        printf("SERVER: Received FRIENDS from client\n");
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            friends[i] = -1;
        }
        if (strlen(req_msg.message_info.buf) > 0) {
            printf("SERVER: Adding FRIENDS %s\n", req_msg.message_info.buf);
            char *tmp = strtok(req_msg.message_info.buf, "\n");
            int tmp_int;
            tmp = strtok(tmp, ",");
            while (tmp) {
                sscanf(tmp, "%d", &tmp_int);
                if (tmp_int > 9 || tmp_int < 0) {
                    fprintf(stderr, "SERVER: Failed passed wrong ID for FRIENDS request\n");
                    return 1;
                }else {
                    friends[tmp_int] = 1;
                }
                tmp = strtok(NULL, ",");
            }
            printf("SERVER: FRIENDS successfully added\n");
        } else {
            printf("SERVER: FRIENDS successfully cleared\n");
        }

    } else if (req_msg.message_info.request == ADD) {
        printf("SERVER: Received ADD from client\n");
        printf("SERVER: Adding FRIENDS %s\n", req_msg.message_info.buf);
        char *tmp = strtok(req_msg.message_info.buf, "\n");
        int tmp_int;
        tmp = strtok(tmp, ",");
        while (tmp) {
            sscanf(tmp, "%d", &tmp_int);
            if (tmp_int > 9 || tmp_int < 0) {
                fprintf(stderr, "SERVER: Failed passed wrong ID for ADD request\n");
                return 1;
            }else {
                friends[tmp_int] = 1;
            }
            tmp = strtok(NULL, ",");
        }
        printf("SERVER: FRIENDS successfully added\n");
    } else if (req_msg.message_info.request == DEL) {
        printf("SERVER: Received DEL from client\n");
        printf("SERVER: Deleting FRIENDS %s\n", req_msg.message_info.buf);
        char *tmp = strtok(req_msg.message_info.buf, "\n");
        int tmp_int;
        tmp = strtok(tmp, ",");
        while (tmp) {
            sscanf(tmp, "%d", &tmp_int);
            if (tmp_int > 9 || tmp_int < 0) {
                fprintf(stderr, "SERVER: Failed passed wrong ID for DEL request\n");
                return 1;
            }else {
                friends[tmp_int] = -1;
            }
            tmp = strtok(NULL, ",");
        }
        printf("SERVER: FRIENDS successfully deleted\n");
    } else if (req_msg.message_info.request == ALL2) {
        printf("SERVER: Received 2ALL from client, sending the response\n");
        char tmp_time[MAX_PATH];
        time_t rawtime;
        time(&rawtime);
        strftime(tmp_time, 100, format, localtime(&rawtime));
        char buffer[MAX_PATH];
        sprintf(buffer, "MESSAGE FROM CLIENT_ID %d, %s, CURRENT SERVER DATE: %s", req_msg.message_info.id,
                req_msg.message_info.buf, tmp_time);
        strncpy(message_send.message_info.buf, buffer, MAXMSZ);
        message_send.mtype = RESTORDER;
        message_send.message_info.request = ALL2;
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            if (clients[i] > -1) {
                if (msgsnd(clients[i], &message_send, MSG_SIZE, 0) == -1) { // SEND BACK TO ALL CLIENTS
                    fprintf(stderr, "SERVER: Failed responding for 2ALL request\n");
                    return 1;
                }
            }
        }

        printf("SERVER: Response: %s, was sent successfully to all active clients\n", buffer);
    } else if (req_msg.message_info.request == FRIENDS2) {
        printf("SERVER: Received 2FRIENDS from client, sending the response\n");
        char tmp_time[MAX_PATH];
        time_t rawtime;
        time(&rawtime);
        strftime(tmp_time, 100, format, localtime(&rawtime));
        char buffer[MAX_PATH];
        sprintf(buffer, "MESSAGE FROM CLIENT_ID %d, %s, CURRENT SERVER DATE: %s", req_msg.message_info.id,
                req_msg.message_info.buf, tmp_time);
        strncpy(message_send.message_info.buf, buffer, MAXMSZ);
        message_send.mtype = RESTORDER;
        message_send.message_info.request = FRIENDS2;
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            if (clients[i] > -1 && friends[i] > -1) {
                if (msgsnd(clients[i], &message_send, MSG_SIZE, 0) == -1) { // SEND BACK TO ALL FRIENDS
                    fprintf(stderr, "SERVER: Failed responding for 2FRIENDS request\n");
                    return 1;
                }
            }
        }

        printf("SERVER: Response: %s, was sent successfully to all active friends\n", buffer);
    } else if (req_msg.message_info.request == ONE2) {
        printf("SERVER: Received ONE2 from client, sending the response\n");
        char *tmp = strtok(req_msg.message_info.buf, " ");
        int tmp_int;
        sscanf(tmp, "%d", &tmp_int);
        if (tmp_int > 9 || tmp_int < 0 || clients[tmp_int] == -1) {
            fprintf(stderr, "SERVER: Failed passed wrong ID for 2ONE request\n");
            return 1;
        }
        message_send.mtype = RESTORDER;
        message_send.message_info.request = ONE2;
        char tmp_time[MAX_PATH];
        time_t rawtime;
        time(&rawtime);
        strftime(tmp_time, 100, format, localtime(&rawtime));
        char buffer[MAX_PATH];
        sprintf(buffer, "From Client %d, Message %s, Date %s", message_send.message_info.id, strtok(NULL, " "),
                tmp_time);
        strncpy(message_send.message_info.buf, buffer, MAXMSZ);

        if (msgsnd(clients[tmp_int], &message_send, MSG_SIZE, 0) == -1) { // SEND BACK TO SPECIFIED CLIENTS
            fprintf(stderr, "SERVER: Failed responding for 2ONE request\n");
            return 1;
        }
        printf("SERVER: Response: %s, was sent successfully from cleint %d to client %d\n", buffer,
               message_send.message_info.id, tmp_int);
    } else if (req_msg.message_info.request == STOP) {
        printf("SERVER: Received STOP from client, unregistering him\n");
        if (req_msg.message_info.id >= MAX_CLIENTS || req_msg.message_info.id < 0) {
            fprintf(stderr, "SERVER: Wrong Client ID received\n");
            return 1;
        }
        clients[req_msg.message_info.id] = -1;
        printf("SERVER: Successfully unregistered client with ID %d\n", req_msg.message_info.id);
    } else {
        fprintf(stderr, "SERVER: Wrong request received");
        return 1;
    }
    return 0;
}

int check() { // waiting for all stop to receive
    int i = 0;
    for (i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i] > -1) break;
    }
    if (i == MAX_CLIENTS) return 0;
    return 1;
}

void exit_handler() {
    printf("\nSERVER: CTRL-C received\n");
    struct message message_send;
    message_send.message_info.request = STOP;
    message_send.mtype = FIRSTORDER;
    int fl = 0;
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i] > -1) {
            fl = 1;
            printf("\nSERVER: Sending STOP signal to client %d\n", i);
            if (msgsnd(clients[i], &message_send, MSG_SIZE, 0) == -1) { // SEND STOPS TO ALL CLIENTS
                fprintf(stderr, "SERVER: Failed sending STOP to client %d\n", i);

            }
        }
    }
    struct message message_receive;

    while (fl) {
        if ((msgrcv(serverqid, &message_receive, MSG_SIZE, SECONDORDER, 0)) != -1) { // WAIT FOR ALL STOP SIGNALS FROM
            if (!handle_request(message_receive) && !check()) { // ALL CLIENTS ARE CLOSED
                fl = 0;
            }
        }
    }
    printf("\nSERVER: All client are closed\n");
    exit(0); // just exiting process, atexit will done cleanup
}


int main() {
    struct message message_receive; // message, second message and receive
    key_t server_key;
    char *KEY_HOME;
    for (int i = 0; i < MAX_CLIENTS; ++i) { // clear out the client array
        clients[i] = -1;
        friends[i] = -1;
    }

    if (signal(SIGINT, exit_handler) ==
        SIG_ERR) { // exit_handler will closed all the client queues and then close server queue
        fprintf(stderr, "SERVER: Couldn't set CTRL-C handler\n");
        exit(EXIT_FAILURE);
    }

    if (atexit(cleanup)) {
        fprintf(stderr, "SERVER: Cannot set cleanup function\n");
        exit(EXIT_FAILURE);
    }

    if (!(KEY_HOME = getenv("HOME"))) { 
        fprintf(stderr, "SERVER: Couldn't get HOME variable\n");
        exit(EXIT_FAILURE);
    }

    if ((server_key = ftok(KEY_HOME, KEY0)) == -1) {
        fprintf(stderr, "SERVER: Server key generation failed\n");
        exit(EXIT_FAILURE);
    }

    if ((serverqid = msgget(server_key, IPC_CREAT | 0666)) == -1) {
        fprintf(stderr, "SERVER: Failed to create server-side queue\n");
        exit(EXIT_FAILURE);
    } else {
        printf("SERVER: Server queue started successfully with id: %d\n", serverqid);
    }

    while (1) {
        sleep(1);
        if ((msgrcv(serverqid, &message_receive, MSG_SIZE, -RESTORDER, 0)) != -1 && handle_request(message_receive)) {
            fprintf(stderr, "SERVER: Couldn't handle request\n");
            cleanup();
            return 1;
        }
    }

}