#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <signal.h>
#include "keys.h"

int clientqid, serverqid;
int client_id = -1;

void cleanup() {
    struct message message_send;
    printf("CLIENT %d: Received STOP signal, sending STOP back to server\n", client_id);
    message_send.mtype = SECONDORDER;
    message_send.message_info.id = client_id;
    message_send.message_info.request = STOP;
    if (msgsnd(serverqid, &message_send, MSG_SIZE, 0) == -1) {
        fprintf(stderr, "CLIENT %d: Sending STOP request failed\n", client_id);
        exit(EXIT_FAILURE);
    }

    printf("CLIENT %d: Performing cleanup\nCLIENT %d: Closing process with qid: %d\n", client_id, client_id, clientqid);
    if (msgctl(clientqid, IPC_RMID, NULL)) {
        fprintf(stderr, "CLIENT %d: Failed to close queue\n", client_id);
        _exit(EXIT_FAILURE);
    } else {
        printf("CLIENT %d: Queue closed successfully\n", client_id);
    }
}

void handler() {
    printf("\nSERVER: CTRL-C received\n"); // atexit will clean the queue
    exit(0);
}

int main() {
    struct message message_send, message_recieve; // message to send and receive
    struct msqid_ds buf;
    char *KEY_HOME;
    char buffer[MAX_PATH];
    key_t server_key, client_key;
    printf("CLIENT %d: Started working. CLIENT_ID -1 mean that it's not set up yet\n", client_id);

    if (signal(SIGINT, handler) == SIG_ERR) {
        fprintf(stderr, "CLIENT %d: Couldn't set CTRL-Z handler\n", client_id);
        exit(EXIT_FAILURE);
    }

    if (!(KEY_HOME = getenv("HOME"))) { 
        fprintf(stderr, "CLIENT %d: Couldn't get HOME variable\n", client_id);
        exit(EXIT_FAILURE);
    }

    if (atexit(cleanup)) {
        fprintf(stderr, "CLIENT %d: Cannot set cleanup function\n", client_id);
        exit(EXIT_FAILURE);
    }

    if ((server_key = ftok(KEY_HOME, KEY0)) == -1) {
        fprintf(stderr, "CLIENT %d: Server key generation failed\n", client_id);
        exit(EXIT_FAILURE);
    }

    if ((serverqid = msgget(server_key, 0)) < 0) { // open server queue
        fprintf(stderr, "CLIENT %d: Failed to open server-side queue, make sure that server is running\n", client_id);
        exit(EXIT_FAILURE);
    }

    if ((client_key = ftok(KEY_HOME, getpid())) == -1) {
        fprintf(stderr, "CLIENT %d: Client key generation failed\n", client_id);
        exit(EXIT_FAILURE);
    }

    if ((clientqid = msgget(client_key, IPC_CREAT | 0666)) < 0) { // create client side queue
        fprintf(stderr, "CLIENT %d: Failed to create client %d queue\n", client_id, client_id);
        exit(EXIT_FAILURE);
    } else {
        printf("CLIENT %d: Queue started successfully with id: %d\n", client_id, client_id);
    }

    // send INIT message with the highest priority before entering to main loop
    printf("CLIENT %d: Sending INIT request to the server \n", client_id);

    message_send.mtype = FIRSTORDER;
    message_send.message_info.request = INIT;
    sprintf(message_send.message_info.buf, "%d", client_key);
    if (msgsnd(serverqid, &message_send, MSG_SIZE, 0) == -1) {
        fprintf(stderr, "CLIENT %d: Failed to send INIT request\n", client_id);
        exit(EXIT_FAILURE);
    }

    while (msgrcv(serverqid, &message_recieve, MSG_SIZE, 5, 0) == -1) {
        sleep(1);
    } // wait for response using server queue with mtype 5 to avoid deadlock

    if (sscanf(message_recieve.message_info.buf, "%d", &client_id) != 1) { // get unique client_id
        fprintf(stderr, "CLIENT ?: Failed receiving client_id from server queue\n");
        exit(EXIT_FAILURE);
    }
    printf("CLIENT %d: INIT request was successful, my ID is %d\n", client_id, client_id);
    printf("CLIENT %d: Waiting for requests\n", client_id);
    while (fgets(buffer, MAX_PATH - 2, stdin)) {

        char *tmp = strtok(buffer, " ");
        message_send.message_info.id = client_id;

        if (!strcmp(tmp, "ECHO")) {
            message_send.mtype = RESTORDER;
            message_send.message_info.request = ECHO;
            tmp = strtok(NULL, "\n");
            printf("CLIENT %d: Received ECHO request, sending string %s to the server\n", client_id, tmp);
            strncpy(message_send.message_info.buf, tmp, MAXMSZ - 1);
            if (msgsnd(serverqid, &message_send, MSG_SIZE, 0) == -1) {
                fprintf(stderr, "CLIENT %d: Sending echo request failed\n", client_id);
                continue;
            }
        } else if (!strcmp(tmp, "LIST\n")) {
            message_send.mtype = THIRDORDER;
            message_send.message_info.request = LIST;
            printf("CLIENT %d: Received LIST request, sending LIST to server\n", client_id);
            if (msgsnd(serverqid, &message_send, MSG_SIZE, 0) == -1) {
                fprintf(stderr, "CLIENT %d: Sending LIST request failed\n", client_id);
                exit(EXIT_FAILURE);
            }
        } else if (!strcmp(tmp, "FRIENDS")) {
            tmp = strtok(NULL, "\n");
            printf("CLIENT %d: Received FRIENDS with list request, sending list %s to the server\n", client_id, tmp);
            message_send.mtype = THIRDORDER;
            message_send.message_info.request = FRIENDS;
            strcpy(message_send.message_info.buf, tmp);
            if (msgsnd(serverqid, &message_send, MSG_SIZE, 0) == -1) {
                fprintf(stderr, "CLIENT %d: Sending FRIEND request failed\n", client_id);
                exit(EXIT_FAILURE);
            }

        } else if (!strcmp(tmp, "FRIENDS\n")) { // friends without the list
            printf("CLIENT %d: Received FRIENDS, sending FRIEND clear request to the server\n", client_id);
            message_send.mtype = THIRDORDER;
            message_send.message_info.request = FRIENDS;
            strcpy(message_send.message_info.buf, "");
            if (msgsnd(serverqid, &message_send, MSG_SIZE, 0) == -1) {
                fprintf(stderr, "CLIENT %d: Sending FRIEND request failed\n", client_id);
                exit(EXIT_FAILURE);
            }

        } else if (!strcmp(tmp, "ADD")) {
            tmp = strtok(NULL, "\n");
            printf("CLIENT %d: Received ADD request, sending list %s to the server\n", client_id, tmp);
            message_send.mtype = THIRDORDER;
            message_send.message_info.request = ADD;
            strcpy(message_send.message_info.buf, tmp);
            if (msgsnd(serverqid, &message_send, MSG_SIZE, 0) == -1) {
                fprintf(stderr, "CLIENT %d: Sending ADD request failed\n", client_id);
                exit(EXIT_FAILURE);
            }

        } else if (!strcmp(tmp, "DEL")) {
            tmp = strtok(NULL, "\n");
            printf("CLIENT %d: Received DEL request, sending list %s to the server\n", client_id, tmp);
            message_send.mtype = THIRDORDER;
            message_send.message_info.request = DEL;
            strcpy(message_send.message_info.buf, tmp);
            if (msgsnd(serverqid, &message_send, MSG_SIZE, 0) == -1) {
                fprintf(stderr, "CLIENT %d: Sending DEL request failed\n", client_id);
                exit(EXIT_FAILURE);
            }

        } else if (!strcmp(tmp, "2ALL")) {
            message_send.mtype = RESTORDER;
            message_send.message_info.request = ALL2;
            tmp = strtok(NULL, "\n");
            printf("CLIENT %d: Received 2ALL request, sending string %s to the server\n", client_id, tmp);
            strncpy(message_send.message_info.buf, tmp, MAXMSZ - 1);
            if (msgsnd(serverqid, &message_send, MSG_SIZE, 0) == -1) {
                fprintf(stderr, "CLIENT %d: Sending 2ALL request failed\n", client_id);
                continue;
            }

        } else if (!strcmp(tmp, "2FRIENDS")) {
            tmp = strtok(NULL, "\n");
            printf("CLIENT %d: Received 2FRIENDS request, sending list %s to the server\n", client_id, tmp);
            message_send.mtype = RESTORDER;
            message_send.message_info.request = FRIENDS2;
            strcpy(message_send.message_info.buf, tmp);
            if (msgsnd(serverqid, &message_send, MSG_SIZE, 0) == -1) {
                fprintf(stderr, "CLIENT %d: Sending 2FRIENDS request failed\n", client_id);
                exit(EXIT_FAILURE);
            }
        } else if (!strcmp(tmp, "2ONE")) {
            message_send.mtype = RESTORDER;
            message_send.message_info.request = ONE2;
            tmp = strtok(NULL, " ");
            int tmp_int;
            if (sscanf(tmp, "%d", &tmp_int) != 1) {
                fprintf(stderr, "CLIENT %d: Wrong input for 2ONE request, middle word should be integer\n", client_id);
                continue;
            }
            tmp = strtok(NULL, "\n");
            printf("CLIENT %d: Received 2ONE request, sending string %s to the server\n", client_id, tmp);
            sprintf(buffer, "%d %s", tmp_int, tmp);
            strncpy(message_send.message_info.buf, buffer, MAXMSZ - 1);
            if (msgsnd(serverqid, &message_send, MSG_SIZE, 0) == -1) {
                fprintf(stderr, "CLIENT %d: Sending 2ONE request failed\n", client_id);
                continue;
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
        if ((msgctl(clientqid, IPC_STAT, &buf) < 0)) { // check stats for number of messages in the queue
            fprintf(stderr, "Checking queue size failed\n");
            _exit(EXIT_FAILURE);
        }

        if (buf.msg_qnum > 0 && msgrcv(clientqid, &message_recieve, MSG_SIZE, 0, 0) != -1) {
            if (message_recieve.message_info.request == STOP) {
                return 0; // atexit will clear the queue
            }
            printf("CLIENT %d: Received message from the server: %s\n", client_id, message_recieve.message_info.buf);
        }

        printf("CLIENT %d: Waiting for requests\n", client_id);

    }
    
    return 0;
}