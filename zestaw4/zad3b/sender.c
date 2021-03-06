#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

// REAL TIME UNIX SIGNALS
#define SIGRT1 SIGRTMIN
#define SIGRT2 SIGRTMIN+1

int counter = 0;
int signals = 0;

void handler(int signum, siginfo_t *info) {
    if (signum == SIGUSR1) {
        counter++;
        if(info->si_code == SI_USER){
            if(counter == signals){
                kill(info->si_pid, SIGUSR2);
            }else {
                kill(info->si_pid, SIGUSR1);
            }
        }else{
            union sigval value;
            value.sival_int = 0;
             if(counter == signals){
                sigqueue(info->si_pid, SIGUSR2, value);
            }else {
                sigqueue(info->si_pid, SIGUSR1, value);
            }
        }

    } else if (signum == SIGUSR2) {
        if (info->si_code == SI_USER) { // Kill mode
            printf("I'm a sender and I've recieved %d signals, and I should %d \n", counter, signals);
            exit(counter);
        } else if (info->si_code == SI_QUEUE) { // SIGQUEUE mode
            printf("I'm a sender and I've sent %d signals\nCatcher sent(recived) %d\nI recieved %d\n", signals,
                   info->si_value.sival_int, counter);
            exit(counter);
        }
    }if (signum == SIGRT1) {
        counter++;
        if(counter == signals){
            kill(info->si_pid, SIGRT2);
        }else {
            kill(info->si_pid, SIGRT1);
        };
    } else if (signum == SIGRT2) {
        printf("I'm a sender and I've recieved %d signals, but I should %d \n", counter, signals);
        exit(counter);
    }
}

int main(int argc, char *argv[]) {

    if (argc != 4) {
        fprintf(stderr, "3 parameter was expected\n");
        exit(EXIT_FAILURE);
    }

    int pid, iterator;

    if (sscanf(argv[1], "%d", &pid) != 1) {
        printf("Wrong input, first parameter should be integer\n");
        exit(EXIT_FAILURE);
    }

    if (sscanf(argv[2], "%d", &signals) != 1) {
        printf("Wrong input, second parameter should be integer\n");
        exit(EXIT_FAILURE);
    }

    struct sigaction act;
    act.sa_sigaction = (void*)handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR1, &act, NULL);
    sigaction(SIGUSR2, &act, NULL);
    sigaction(SIGRT1, &act, NULL);
    sigaction(SIGRT2, &act, NULL);

    if (!strcmp(argv[3], "KILL")) {
        kill(pid, SIGUSR1);

        while (1) {
        }

    } else if (!strcmp(argv[3], "SIGQUEUE")) {
        union sigval value;
        value.sival_int = 0;
        sigqueue(pid, SIGUSR1, value);

        while (1) {
        }
    } else if (!strcmp(argv[3], "SIGRT")) {
        kill(pid, SIGRT1);
        while (1) {
        }

    } else {
        printf("Wrong mode was given: %s \n", argv[3]);
        exit(EXIT_FAILURE);
    }

    return 0;

}