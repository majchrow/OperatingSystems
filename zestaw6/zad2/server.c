#include <stdio.h>
#include <mqueue.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include "keys.h"

int serverqid;
static int clients[MAX_CLIENTS]; // position in array = ClientID, value in array = Client Queue
static int friends[MAX_CLIENTS]; // friends array - we assume that there can be only one friend list per server

void cleanup() {
    printf("SERVER: Performing cleanup\nSERVER: Closing server process with id: %d\n", serverqid);
    if (mq_close(serverqid)) {
        fprintf(stderr, "SERVER: Failed to close queue\n");
        exit(EXIT_FAILURE);
    } else {
        printf("SERVER: Queue closed successfully\n");
    }
    mq_unlink(server_path);
}

int handle_request(struct message req_msg) {
    struct message message_send;

    if (req_msg.mtype == INIT) {
        printf("SERVER: Received INIT from client, trying to register\n");
        int current_client;
        for (current_client = 0; current_client < MAX_CLIENTS &&
                                 clients[current_client] != -1; ++current_client) {} // find first free client

        if (current_client == MAX_CLIENTS) { // MAX_CLIENTS ALREADY
            fprintf(stderr, "SERVER: There is %d clients already, can't serve more\n", current_client);
            return 1;
        } else {
            char client_path[MAX_PATH];
            if (sscanf(req_msg.message_info.buf, "%s", client_path) < 1) {
                fprintf(stderr, "SERVER: Received INIT without the key\n");
                return 1;
            }

            if ((clients[current_client] = mq_open(client_path, O_WRONLY)) < 0) { // open client queue
                fprintf(stderr, "SERVER: Failed to open client-side queue\n");
                return 1;
            }
            message_send.mtype = INIT; // sending init to client queue, because there is no way to send it to server one
            sprintf(message_send.message_info.buf, "%d", current_client);

            if (mq_send(clients[current_client], (char *) &message_send, MSG_SIZE, FIRSTORDER) ==
                -1) { // SEND BACK INIT WITH CLIENT_ID
                fprintf(stderr, "SERVER: Sending message failed\n");
                return 1;
            }
            printf("SERVER: Successfully registered client with ID %d\n", current_client);
        }
    } else if (req_msg.mtype == ECHO) {
        printf("SERVER: Received ECHO from client, sending the response\n");
        char tmp_time[MAX_PATH];
        time_t rawtime;
        time(&rawtime);
        strftime(tmp_time, 100, format, localtime(&rawtime));
        char buffer[MAX_PATH];
        sprintf(buffer, "%s, CURRENT SERVER DATE: %s", req_msg.message_info.buf, tmp_time);
        message_send.mtype = ECHO;
        strncpy(message_send.message_info.buf, buffer, MAXMSZ);
        if (mq_send(clients[req_msg.message_info.id], (char *) &message_send, MSG_SIZE, RESTORDER) == -1) {
            fprintf(stderr, "SERVER: Failed responding for ECHO request\n");
            return 1;
        }
        printf("SERVER: Response: %s, was sent successfully\n", buffer);
    } else if (req_msg.mtype == LIST) {
        printf("SERVER: Received LIST from client, searching for active clients\n");
        int current_client;
        for (current_client = 0; current_client < MAX_CLIENTS; ++current_client) {
            if (clients[current_client] != -1) {
                printf("SERVER: Client Active: ID %d Queue %d\n", current_client, clients[current_client]);
            }
        }
        printf("SERVER: There is no more active clients\n");
    } else if (req_msg.mtype == FRIENDS) {
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
                } else {
                    friends[tmp_int] = 1;
                }
                tmp = strtok(NULL, ",");
            }
            printf("SERVER: FRIENDS successfully added\n");
        } else {
            printf("SERVER: FRIENDS successfully cleared\n");
        }

    } else if (req_msg.mtype == ADD) {
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
            } else {
                friends[tmp_int] = 1;
            }
            tmp = strtok(NULL, ",");
        }
        printf("SERVER: FRIENDS successfully added\n");
    } else if (req_msg.mtype == DEL) {
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
            } else {
                friends[tmp_int] = -1;
            }
            tmp = strtok(NULL, ",");
        }
        printf("SERVER: FRIENDS successfully deleted\n");
    } else if (req_msg.mtype == ALL2) {
        printf("SERVER: Received 2ALL from client, sending the response\n");
        char tmp_time[MAX_PATH];
        time_t rawtime;
        time(&rawtime);
        strftime(tmp_time, 100, format, localtime(&rawtime));
        char buffer[MAX_PATH];
        sprintf(buffer, "MESSAGE FROM CLIENT_ID %d, %s, CURRENT SERVER DATE: %s", req_msg.message_info.id,
                req_msg.message_info.buf, tmp_time);
        strncpy(message_send.message_info.buf, buffer, MAXMSZ);
        message_send.mtype = ALL2;
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            if (clients[i] > -1) {
                if (mq_send(clients[i], (char *) &message_send, MSG_SIZE, RESTORDER) == -1) {
                    fprintf(stderr, "SERVER: Failed responding for 2ALL request\n");
                    return 1;
                }
            }
        }

        printf("SERVER: Response: %s, was sent successfully to all active clients\n", buffer);
    } else if (req_msg.mtype == FRIENDS2) {
        printf("SERVER: Received 2FRIENDS from client, sending the response\n");
        char tmp_time[MAX_PATH];
        time_t rawtime;
        time(&rawtime);
        strftime(tmp_time, 100, format, localtime(&rawtime));
        char buffer[MAX_PATH];
        sprintf(buffer, "MESSAGE FROM CLIENT_ID %d, %s, CURRENT SERVER DATE: %s", req_msg.message_info.id,
                req_msg.message_info.buf, tmp_time);
        strncpy(message_send.message_info.buf, buffer, MAXMSZ);
        message_send.mtype = FRIENDS2;
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            if (clients[i] > -1 && friends[i] > -1) {
                if (mq_send(clients[i], (char *) &message_send, MSG_SIZE, RESTORDER) == -1) {
                    fprintf(stderr, "SERVER: Failed responding for 2FRIENDS request\n");
                    return 1;
                }
            }
        }

        printf("SERVER: Response: %s, was sent successfully to all active friends\n", buffer);
    } else if (req_msg.mtype == ONE2) {
        printf("SERVER: Received ONE2 from client, sending the response\n");
        char *tmp = strtok(req_msg.message_info.buf, " ");
        int tmp_int;
        sscanf(tmp, "%d", &tmp_int);
        if (tmp_int > 9 || tmp_int < 0 || clients[tmp_int] == -1) {
            fprintf(stderr, "SERVER: Failed passed wrong ID for 2ONE request\n");
            return 1;
        }
        message_send.mtype = ONE2;
        char tmp_time[MAX_PATH];
        time_t rawtime;
        time(&rawtime);
        strftime(tmp_time, 100, format, localtime(&rawtime));
        char buffer[MAX_PATH];
        sprintf(buffer, "From Client %d, Message %s, Date %s", message_send.message_info.id, strtok(NULL, " "),
                tmp_time);
        strncpy(message_send.message_info.buf, buffer, MAXMSZ);

        if (mq_send(clients[tmp_int], (char *) &message_send, MSG_SIZE, RESTORDER) == -1) {
            fprintf(stderr, "SERVER: Failed responding for 2ONE request\n");
            return 1;
        }
        printf("SERVER: Response: %s, was sent successfully from client %d to client %d\n", buffer,
               message_send.message_info.id, tmp_int);
    } else if (req_msg.mtype == STOP) {
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
    message_send.mtype = STOP;
    int fl = 0;
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i] > -1) {
            fl = 1;
            printf("\nSERVER: Sending STOP signal to client %d\n", i);
            if (mq_send(clients[i], (char *) &message_send, MSG_SIZE, SECONDORDER) == -1) {
                fprintf(stderr, "SERVER: Failed sending STOP to client %d\n", i);
            }
        }
    }
    struct message message_receive;

    while (fl) {
        if ((mq_receive(serverqid, (char *) &message_receive, MSG_SIZE, NULL)) != -1 &&
            message_receive.mtype == STOP) { // WAIT FOR ALL STOP SIGNALS FROM
            if (!handle_request(message_receive) && !check()) { // ALL CLIENTS ARE CLOSED
                fl = 0;
            }
        }
    }
    printf("\nSERVER: All client are closed\n");
    exit(0); // just exiting process, atexit will done cleanup
}


int main() {
    struct message message;

    if (signal(SIGINT, exit_handler) ==
        SIG_ERR) { // exit_handler will closed all the client queues and then close server queue
        fprintf(stderr, "SERVER: Couldn't set CTRL-C handler\n");
        exit(EXIT_FAILURE);
    }

    if (atexit(cleanup)) {
        fprintf(stderr, "SERVER: Cannot set cleanup function\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < MAX_CLIENTS; ++i) { // clear out the client array
        clients[i] = -1;
        friends[i] = -1;
    }


    struct mq_attr attr;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MSG_SIZE;


    if ((serverqid = mq_open(server_path, O_RDONLY | O_CREAT | O_EXCL | O_NONBLOCK, 0664, &attr)) == -1) {
        fprintf(stderr, "SERVER: Failed to create server-side queue\n");
        exit(EXIT_FAILURE);
    } else {
        printf("SERVER: Server queue started successfully with id: %d\n", serverqid);
    }

    while (1) {
        sleep(1);
        if ((mq_receive(serverqid, (char *) &message, MSG_SIZE, NULL)) != -1 && handle_request(message)) {
            fprintf(stderr, "SERVER: Couldn't handle request\n");
            return 0; // atexit will clean up
        }

    }
}