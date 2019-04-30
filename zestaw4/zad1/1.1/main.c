#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

int global_state;

void handler(int signum){ // handler for signals
    if (signum == SIGTSTP){
       		if(global_state == 1) printf("\nWaiting for:\nCTRL+Z - continue or CTRL +C - terminate program");
       		printf("\n");
       		global_state =  1 - global_state; 
    }else if (signum == SIGINT){
        printf("\nSignal SIGINT recived, terminating program\n");
        _exit(0);
    }
}	

int main(int argc, char* argv[]) {

    global_state = 1;
    struct sigaction act;
    act.sa_handler = handler;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    signal(SIGTSTP, handler);
    sigaction(SIGINT, &act, NULL);

	while(1){
		if(global_state == 1){
       		system("date");
       		sleep(1);	
		}
		else{
			pause();
		}
	}
    return 0;
}