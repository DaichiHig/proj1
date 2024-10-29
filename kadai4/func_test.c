
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include "target-io.h"

int main(){
	int val;
	char dat[] = "unnco";
	char dat2[] = "onara";
	int thn = 0;
	printf("o");
	
	write_LCD(dat, thn);
	
	printf("na");
		
	sleep(10);
	
	char space[10];
	
	for(int i=0; i < 7; i++){
		space[i] = 0b10100000;
	}
	/*
	space[6] = '\0';
	write_LCD(space, thn);
	
	sleep(10);
	
	write_LCD(dat2, thn);
	*/
	
	thn = 1;
	write_LCD(dat, thn);
	
	sleep(10);
	
	write_LCD(space, thn);
	
	sleep(10);
	
	printf("ra");
	
}






