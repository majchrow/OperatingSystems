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

void handler(int signum, siginfo_t *info){
    int i;
    if(signum == SIGUSR1){
        counter++;
    }else if(signum == SIGUSR2){
        
        if(info->si_code == SI_USER){ // Kill mode
            for(i = 0; i < counter; ++i){
                kill(info->si_pid, SIGUSR1);                 
            }
            kill(info->si_pid, SIGUSR2);   
            printf("I'm a catcher and I've recieved %d signals \n", counter);
            exit(counter);
        }else if (info->si_code == SI_QUEUE){ // SIGQUEUE mode
        union sigval value;
        value.sival_int = 0;
        for (i = 0; i < counter; ++i){
            sigqueue(info->si_pid, SIGUSR1, value);
        }
        value.sival_int = counter;
        sigqueue(info->si_pid, SIGUSR2, value);
        printf("I'm a catcher and I've recieved %d signals \n", counter);
        exit(counter);
    } 
        } else if (signum == SIGRT1) {
            counter++;
            } else if (signum == SIGRT2) {
            for(i = 0; i < counter; ++i){
                kill(info->si_pid, SIGRT1);                 
            }
            kill(info->si_pid, SIGRT2);   
            printf("I'm a catcher and I've recieved %d signals \n", counter);
            exit(counter);
        }
}


int main(int argc, char* argv[]) {

    struct sigaction act;
    act.sa_sigaction = (void*)handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR1, &act, NULL);
    sigaction(SIGUSR2, &act, NULL);
    sigaction(SIGRT1, &act, NULL);
    sigaction(SIGRT2, &act, NULL);

    printf("Hello, I'm a catcher my PID is: %d\nI'm waiting for SIGUSR1 and SIGUSR2 signals\n", getpid());
    while(1){

    }
    return 0;

}