#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

int main(){
	char buff[10];
	int fd, valr, valw, valc;
	fd = open("test.txt", O_RDONLY);
	if(fd < 0){
		perror("open(2):");
		exit(1);
	}
	valr = read(fd, buff, 10);
	if(valr < 0){
		perror("read(2):");
		exit(1);
	}
	valw = write(1, buff, 10);
	if(valw < 0){
		perror("write(2):");
		exit(1);
	}
	valc = close(fd);
	if(valc < 0){
		perror("close(2):");
		exit(1);
	}
}
