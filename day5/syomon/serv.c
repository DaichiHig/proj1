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
	int sockfd, new_sockfd;
	int val;
	
	char buff[128];
	char dat[2]; //受取データ抽出用
	char buff_re[128];
	int dat_int;
	int flag = 1;
	
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
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(PORTNUM);
	
	//ソケットのオプション
	val = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int));
	if (val == -1){
		perror("setsokopt:");
		exit(1);
	}
	
	
	//ソケットにアドレスを割り当てる
	val = bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr_in));
	if(val == -1){
		perror("bind:");
		exit(1);
	}
	
	//コネクションを待ち始める
	val = listen(sockfd, 5);
	if(val == -1){
		perror("listen:");
		exit(1);
	}
	
	//要求の受付
	new_sockfd = accept(sockfd, NULL, NULL);
	if(new_sockfd == -1){
		perror("acceot:");
		exit(1);
	}
	
	//クライアントからデータ
	val = read(new_sockfd, buff, 128);
	if(val == -1){
		perror("read:");
		exit(1);
	}
	
	dat[0] = buff[0];
	dat_int = (int)dat[0];
	
	val = write(1, dat, sizeof(dat));
	if(val == -1){
		perror("write:");
		exit(1);
	}
	
	//データを送り返す
	dat_int ++;
	buff_re[0] = (char)dat_int;
	//データの送信
	write(new_sockfd, buff_re, 128);
	if(val == -1){
		perror("write2:");
		exit(1);
	}
	
	sleep(1);
	val = close(new_sockfd);
	if(val == -1){
		perror("close(new_sockfd):");
		exit(1);
	}
	val = close(sockfd);
	if(val == -1){
		perror("close(sockfd):");
		exit(1);
	}
	
	
	
}

