#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>


int main(int argc, char *argv[]){
	
	int fd;
	int val;
	char newfilename[51];
	char textdata[1001];
	char tab[] = {9};
	
	if (argc > 1){
		//command error
		if(strlen(argv[1]) > 50){
			write(2, "over 50 char filename:", 23);
			exit(1);
		}
		strcpy(newfilename, argv[1]);
	}else{
		strcpy(newfilename, "createdtxt.txt");
	}
	
	fd = open(newfilename, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
	if(fd == -1){
		perror("open:");
		exit(1);
	}
	
	for(int i=2; i < argc; i++){
		if(strlen(textdata)+strlen(tab)+strlen(argv[i]) > 1000){
			printf("over data:");
			break;
		}
		strcat(textdata, argv[i]);
		//strcat(textdata, tab);
	}
	
	if(argc > 2){
		val = write(fd, textdata, strlen(textdata));
		if(val == -1){
			perror("write:");
			exit(1);
		}
	}
	
	
	val = close(fd);
	if(val == -1){
		perror("close:");
		exit(1);
	}
}

