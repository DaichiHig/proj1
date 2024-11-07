#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define HTML_PORTNUM 80
struct sockaddr_in serv_addr;




int main(){
	int sockfd, new_sockfd;
	int val;
	
	char buff[1024];
	//char dat[2]; //受取データ抽出用
	//char buff_re[128];
	int dat_int;
	int flag = 1;
	
	FILE *istream;
	
	//ソケットの作成
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1){
		perror("socket:");
		exit(1);
	}
	
	
	//アドレスの作成
	memset(&serv_addr, 0, sizeof(struct sockaddr_in));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(HTML_PORTNUM);
	
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
	
	//クライアントからデータを受け取る
	istream = fdopen(new_sockfd, "r+");
	if(istream == NULL){
		perror("fdopen:");
		exit(1);
	}
	val = setvbuf(istream, NULL, _IONBF, 0);
	if(val != 0){
		perror("setvbuf:");
		exit(1);
	}
	
	
	
	while(1){
		if(fgets(buff, 1024, istream) == 0){
			write(2, "fgets:error", 12);
			break;
		}
		printf("%s", buff);
		
	}
	val = fclose(istream);
	if(val != 0){
		perror("fclose:");
		exit(1);
	}
	
	/*
	dat_int ++;
	buff_re[0] = (char)dat_int;
	//データの送信
	write(new_sockfd, buff_re, 128);
	*/
	
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

