#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define PORTNUM 50000
struct sockaddr_in serv_addr;




int main(){
	int sockfd;
	int val;
	char buff[128];
	char buff_re[128];
	char dat[2];
	buff[0] = '7';
	
	//ソケットの作成
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1){
		perror("socket:");
		exit(1);
	}
	
	//アドレスの作成
	//struct sockaddr_in *port;
	memset(&serv_addr, 0, sizeof(struct sockaddr_in));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr("172.28.34.65");
	serv_addr.sin_port = htons(PORTNUM);
	
	//コネクション要求
	val = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr_in));
	if(val == -1){
		perror("connect:");
		exit(1);
	}
	
	//128バイトのデータ送信
	val = write(sockfd, buff, 128);
	if(val == -1){
		perror("write:");
		exit(1);
	}
	
	//リターン
	/*
	val = listen(sockfd, 5);
	if(val == -1){
		perror("listen:");
		exit(1);
	}
	*/
	val = read(sockfd, buff_re, 128);
	if(val == -1){
		perror("read:");
		exit(1);
	}
	dat[0] = buff_re[0];
	val = write(1, dat, sizeof(dat));
	if(val == -1){
		perror("write:");
		exit(1);
	}

	//ソケットを終了する
	val = close(sockfd);
	if(val == -1){
		perror("close:");
		exit(1);
	}
}
