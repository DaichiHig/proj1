#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>

void sig_handler(int signum){
	int valw;
	valw = write(1, "sig", 3);
	if(valw < 0){
		perror("write:");
		exit(1);
	}
}

int main(){
	if(signal(SIGUSR1, sig_handler) == SIG_ERR){
		perror("signal:");
		exit(1);
	}
	
	int valw;
	while(1){
		valw = write(1, "tick\n", 6);
		if(valw < 0){
			perror("write:");
			exit(1);
		}
		sleep(1);
	}
}
