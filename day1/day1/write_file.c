#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

int main(){
	
	int fd;
	int val;
	const void *buf;
	size_t count;
	buf = "ちんこ";
	count = strlen(buf);
	
	fd = open("test.txt", O_WRONLY);
	if(fd == -1){
		perror("open:");
		exit(1);
	}
	
	val = write(fd, buf, count);
	if(val == -1) {
		perror("write:");
		exit(1);
	}
	
	val = close(fd);
	if(val == -1){
		perror("close:");	
		exit(1);
	}
}


