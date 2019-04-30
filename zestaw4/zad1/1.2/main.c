#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <wait.h>

pid_t global_pid;

void handler(int signum){ // handler for signals

    if (signum == SIGTSTP){
        if (!waitpid(global_pid, NULL, WNOHANG)){ // Child already killed
            // Child printing date, just kill it
            kill(global_pid, SIGKILL);
            printf("\nWaiting for:\nCTRL+Z - continue or CTRL +C - terminate program\n");
        }else{
            int pid = fork();
            if (!pid){
                execl("./date.sh", "date.sh", NULL);
            }else{
                global_pid = pid;
            }
        }
    }else if (signum == SIGINT){
        printf("\nSignal SIGINT recived, terminating program\n");
        if (!waitpid(global_pid, NULL, WNOHANG)){
            kill(global_pid, SIGTERM);
        }
        _exit(0);
    }
}



int main(int argc, char* argv[]) {

    pid_t pid = fork();

    if(pid != 0){ // main process
        global_pid = pid;
        struct sigaction act;
        act.sa_handler = handler;
        act.sa_flags = 0;
        sigemptyset(&act.sa_mask);
        signal(SIGTSTP, handler);
        sigaction(SIGINT, &act, NULL);

      while(1) {
      }

    } else {
            execl("./date.sh", "date.sh", NULL);
    }

    return 0;
}