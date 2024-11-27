#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

int main(int argc, char *argv[]){
	int valk;
	valk = kill(atoi(argv[1]), SIGUSR1);
	if(valk < 0){
		perror("kill:");
		exit(1);
	}
}
