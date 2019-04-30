#include <stdio.h>
#include <mqueue.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include "keys.h"

int clientqid, serverqid;
int client_id = -1;
char clientPath[MAX_PATH];

void cleanup() {
    if(client_id < 0) return;
    struct message message;
    printf("CLIENT %d: Received STOP signal, sending STOP back to server\n", client_id);
    message.mtype = STOP;
    message.message_info.id = client_id;

    if (mq_send(serverqid, (char *) &message, MSG_SIZE, SECONDORDER) == -1) {
        fprintf(stderr, "CLIENT %d: Sending STOP request failed\n", client_id);
        exit(EXIT_FAILURE);
    }

    printf("CLIENT %d: Performing cleanup\nCLIENT %d: Closing process with qid: %d\n", client_id, client_id, clientqid);
    if (mq_close(clientqid)) {
        fprintf(stderr, "CLIENT %d: Failed to close queue\n", client_id);
        _exit(EXIT_FAILURE);
    } else {
        printf("CLIENT %d: Queue closed successfully\n", client_id);
    }
    mq_unlink(clientPath);
}

void exit_handler() {
    printf("\nCLIENT %d: CTRL-C received\n", client_id); // atexit will clean the queue
    exit(0);
}

int main() {
    struct message message;
    char buffer[MAX_PATH];
    sprintf(clientPath, "/%d", getpid());

    if (signal(SIGINT, exit_handler) == SIG_ERR) {
        fprintf(stderr, "CLIENT %d: Couldn't set CTRL-Z handler\n", client_id);
        exit(EXIT_FAILURE);
    }


    if (atexit(cleanup)) {
        fprintf(stderr, "CLIENT %d: Cannot set cleanup function\n", client_id);
        exit(EXIT_FAILURE);
    }


    if ((serverqid = mq_open(server_path, O_WRONLY)) < 0) { // open server queue
        fprintf(stderr, "CLIENT %d: Failed to open server-side queue, make sure that server is running\n", client_id);
        exit(EXIT_FAILURE);
    }

    fprintf(stderr, "CLIENT %d: Successfully opened server-side queue\n", client_id);

    struct mq_attr attr;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MSG_SIZE;


    if ((clientqid = mq_open(clientPath, O_RDONLY | O_CREAT | O_EXCL | O_NONBLOCK, 0664, &attr)) == -1) {
        fprintf(stderr, "CLIENT %d: Failed to create client-side queue\n", client_id);
        exit(EXIT_FAILURE);
    } else {
        printf("CLIENT %d: Client queue started successfully with id: %d\n", client_id, clientqid);
    }


    message.mtype = INIT;
    message.message_info.id = getpid();
    sprintf(message.message_info.buf, "%s", clientPath);

    if (mq_send(serverqid, (char *) &message, MSG_SIZE, FIRSTORDER) == -1) {
        fprintf(stderr, "CLIENT %d: Failed to send INIT request\n", client_id);
        exit(EXIT_FAILURE);
    }


    fprintf(stderr, "CLIENT %d: Successfully send init message to server\n", client_id);

    while (mq_receive(clientqid, (char *) &message, MSG_SIZE, NULL) == -1) {
        sleep(1);
    } // wait for INIT response using client queue

    if (sscanf(message.message_info.buf, "%d", &client_id) != 1) { // get unique client_id
        fprintf(stderr, "CLIENT ?: Failed receiving client_id from server queue\n");
        exit(EXIT_FAILURE);
    }

    printf("CLIENT %d: INIT request was successful, my ID is %d\n", client_id, client_id);
    printf("CLIENT %d: Waiting for requests\n", client_id);

    while (fgets(buffer, MAX_PATH - 2, stdin)) {

        char *tmp = strtok(buffer, " ");
        message.message_info.id = client_id;

        if (!strcmp(tmp, "ECHO")) {
            message.mtype = ECHO;
            tmp = strtok(NULL, "\n");
            printf("CLIENT %d: Received ECHO request, sending string %s to the server\n", client_id, tmp);
            strncpy(message.message_info.buf, tmp, MAXMSZ - 1);
            if (mq_send(serverqid, (char *) &message, MSG_SIZE, RESTORDER) == -1) {
                fprintf(stderr, "CLIENT %d: Sending echo request failed\n", client_id);
                exit(EXIT_FAILURE);
            }
        } else if (!strcmp(tmp, "LIST\n")) {
            message.mtype = LIST;
            printf("CLIENT %d: Received LIST request, sending LIST to server\n", client_id);
            if (mq_send(serverqid, (char *) &message, MSG_SIZE, THIRDORDER) == -1) {
                fprintf(stderr, "CLIENT %d: Sending LIST request failed\n", client_id);
                exit(EXIT_FAILURE);
            }
        } else if (!strcmp(tmp, "FRIENDS")) {
            tmp = strtok(NULL, "\n");
            printf("CLIENT %d: Received FRIENDS with list request, sending list %s to the server\n", client_id, tmp);
            message.mtype = FRIENDS;
            strcpy(message.message_info.buf, tmp);
            if (mq_send(serverqid, (char *) &message, MSG_SIZE, THIRDORDER) == -1) {
                fprintf(stderr, "CLIENT %d: Sending FRIEND request failed\n", client_id);
                exit(EXIT_FAILURE);
            }

        } else if (!strcmp(tmp, "FRIENDS\n")) { // friends without the list
            printf("CLIENT %d: Received FRIENDS, sending FRIEND clear request to the server\n", client_id);
            message.mtype = FRIENDS;
            strcpy(message.message_info.buf, "");
            if (mq_send(serverqid, (char *) &message, MSG_SIZE, THIRDORDER) == -1) {
                fprintf(stderr, "CLIENT %d: Sending FRIEND request failed\n", client_id);
                exit(EXIT_FAILURE);
            }
        } else if (!strcmp(tmp, "ADD")) {
            tmp = strtok(NULL, "\n");
            printf("CLIENT %d: Received ADD request, sending list %s to the server\n", client_id, tmp);
            message.mtype = ADD;
            strcpy(message.message_info.buf, tmp);
            if (mq_send(serverqid, (char *) &message, MSG_SIZE, THIRDORDER) == -1) {
                fprintf(stderr, "CLIENT %d: Sending ADD request failed\n", client_id);
                exit(EXIT_FAILURE);
            }

        } else if (!strcmp(tmp, "DEL")) {
            tmp = strtok(NULL, "\n");
            printf("CLIENT %d: Received DEL request, sending list %s to the server\n", client_id, tmp);
            message.mtype = DEL;
            strcpy(message.message_info.buf, tmp);
            if (mq_send(serverqid, (char *) &message, MSG_SIZE, THIRDORDER) == -1) {
                fprintf(stderr, "CLIENT %d: Sending DEL request failed\n", client_id);
                exit(EXIT_FAILURE);
            }
        } else if (!strcmp(tmp, "2ALL")) {
            message.mtype = ALL2;
            tmp = strtok(NULL, "\n");
            printf("CLIENT %d: Received 2ALL request, sending string %s to the server\n", client_id, tmp);
            strncpy(message.message_info.buf, tmp, MAXMSZ - 1);
            if (mq_send(serverqid, (char *) &message, MSG_SIZE, RESTORDER) == -1) {
                fprintf(stderr, "CLIENT %d: Sending 2ALL request failed\n", client_id);
                exit(EXIT_FAILURE);
            }

        } else if (!strcmp(tmp, "2FRIENDS")) {
            tmp = strtok(NULL, "\n");
            printf("CLIENT %d: Received 2FRIENDS request, sending list %s to the server\n", client_id, tmp);
            message.mtype = FRIENDS2;
            strcpy(message.message_info.buf, tmp);
            if (mq_send(serverqid, (char *) &message, MSG_SIZE, RESTORDER) == -1) {
                fprintf(stderr, "CLIENT %d: Sending 2FRIENDS request failed\n", client_id);
                exit(EXIT_FAILURE);
            }
        } else if (!strcmp(tmp, "2ONE")) {
            message.mtype = ONE2;
            tmp = strtok(NULL, " ");
            int tmp_int;
            if (sscanf(tmp, "%d", &tmp_int) != 1) {
                fprintf(stderr, "CLIENT %d: Wrong input for 2ONE request, middle word should be integer\n", client_id);
                continue;
            }
            tmp = strtok(NULL, "\n");
            printf("CLIENT %d: Received 2ONE request, sending string %s to the server\n", client_id, tmp);
            sprintf(buffer, "%d %s", tmp_int, tmp);
            strncpy(message.message_info.buf, buffer, MAXMSZ - 1);
            if (mq_send(serverqid, (char *) &message, MSG_SIZE, RESTORDER) == -1) {
                fprintf(stderr, "CLIENT %d: Sending 2ONE request failed\n", client_id);
                exit(EXIT_FAILURE);
            }
        } else if (!strcmp(tmp, "STOP\n")) {
            return 0; // atexit will clear the queue
        } else {
            fprintf(stderr, "CLIENT %d: Wrong request, please enter correct one.\n"
                            "CLIENT %d: Don't put the space at the end of the line!!\n"
                            "CLIENT %d: For FRIENDS/ADD/DEL specify list without the bracket and spaces e.g FRIENDS 0,1,2,5\n",
                    client_id, client_id, client_id);
            continue;
        }
        sleep(1);
        if (mq_receive(clientqid, (char *) &message, MSG_SIZE, NULL) != -1) {
            if (message.mtype == STOP) {
                return 0; // atexit will clear the queue
            }
            printf("CLIENT %d: Received message from the server: %s\n", client_id, message.message_info.buf);
        } // wait for INIT response using client queue

        printf("CLIENT %d: Waiting for requests\n", client_id);

    }

    return 0;
}