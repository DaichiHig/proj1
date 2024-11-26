/* cy22226 日暮大地 connect_sock.c*/
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
//外部ファイルのヘッダをインクルード、プロトタイプされた関数のみ参照可----------------------------------------------------
#include "connect_sock.h"

#define PORTNUM 50000
#define THREAD_SETNUM 1

//グローバル変数----------------------------------------------------------------------------------------------
struct sockaddr_in serv_addr;
int sockfd[THREAD_SETNUM*2];
int listen_sockfd;
FILE *istream[THREAD_SETNUM*2];

/*
ソケットをストリーム化する関数
*/
void make_sock_stream(){
	for(int i = 0; i < THREAD_SETNUM; i++){
		istream[i*2] = fdopen(sockfd[i*2], "r+");
		int val;
		if(istream[i*2] == NULL){
			perror("fdopen:\n");
			exit(1);
		}
		val = setvbuf(istream[i*2], NULL, _IONBF, 0);
		if(val != 0){
			perror("setvbuf1:");
			exit(1);
		}
		istream[i*2+1] = fdopen(sockfd[i*2+1], "r+");
		if(istream[i*2+1] == NULL){
			perror("fdopen:\n");
			exit(1);
		}
		val = setvbuf(istream[i*2+1], NULL, _IONBF, 0);
		if(val != 0){
			perror("setvbuf2:");
			exit(1);
		}
	}
}


/*
サーバとしてソケット接続する関数
*/
void be_server(){
	int val;
	int flag = 1;
	int cli_count = 0;
	//ソケットの作成
	listen_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sockfd == -1){
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
	val = setsockopt(listen_sockfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int));
	if (val == -1){
		perror("setsokopt:");
		exit(1);
	}
	
	
	//ソケットにアドレスを割り当てる
	val = bind(listen_sockfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr_in));
	if(val == -1){
		perror("bind:");
		exit(1);
	}
	
	//コネクションを待ち始める
	val = listen(listen_sockfd, 5);
	if(val == -1){
		perror("listen:");
		exit(1);
	}
	
	for(int k = 0; k < THREAD_SETNUM; k++){
		//main側を立ち上げる
		sockfd[k*2] = accept(listen_sockfd, NULL, NULL);
		if(sockfd[k*2] == -1){
			perror("accept:");
			exit(1);
		}
		//
		sockfd[k*2+1] = accept(listen_sockfd, NULL, NULL);
		if(sockfd[k*2+1] == -1){
			perror("accept:");
			exit(1);
		}
		printf("connected\n");
		
	}
	make_sock_stream();
	
}

/*
クライアントとして、ソケットを接続する関数
*/
void be_client(){
	int val;
	int sock_count = 0;
	
	//ソケットの作成
	sockfd[0] = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd[sock_count] == -1){
		perror("socket:");
		exit(1);
	}
	sockfd[1] = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd[sock_count] == -1){
		perror("socket:");
		exit(1);
	}
	
	//アドレスの作成
	//struct sockaddr_in *port;
	memset(&serv_addr, 0, sizeof(struct sockaddr_in));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");//サーバ
	serv_addr.sin_port = htons(PORTNUM);
	
	
	for(int i = 0; i < THREAD_SETNUM; i++){
		//コネクション要求
		val = connect(sockfd[i*2+1], (struct sockaddr *)&serv_addr, sizeof(struct sockaddr_in));//receive側
		if(val == -1){
		perror("connect:");
			exit(1);
		}
		val = connect(sockfd[i*2], (struct sockaddr *)&serv_addr, sizeof(struct sockaddr_in));//main側
		if(val == -1){
		perror("connect:");
			exit(1);
		}
		printf("connected\n");
	}
	make_sock_stream();

}


/*
他社へ振り込みのtxのデータを送る関数
*/
void send_txdata(char *buff){
	//printf("main::in send_txdata:\n");
	printf("main::send_txdata:%s\n", buff);
	fprintf(istream[0], buff);
}

/*
他社から振り込みのtxのデータを受け取る関数
*/
char *receive_txdata(){
	//printf("receive::in_receive_txdata:\n");
	char buff[50];
	char *complete_input = malloc(50);
	if(buff == NULL){
		perror("receive:malloc:");
		exit(1);
	}
	char *val;
	
	while (fgets(buff, sizeof(buff), istream[1])) {
        	strcat(complete_input, buff);
       		// 改行文字が見つかれば終了
        	if (strchr(buff, '\n') != NULL) break;
    	}
	
	printf("receive::receive:%s\n", buff);
	return complete_input;

}

/*
他社からの振り込みに対するレスポンスをする関数。処理ができたときはOK,残高不足によるタイムアウトの場合は
*/
void send_OK_NO(int i){
	
	if(i == 0) {
		fprintf(istream[1], "rok\n");
	}else if(i == 1) {
		fprintf(istream[1], "rno\n");
	}else if(i == -1) {
		fprintf(istream[1], "rtm\n");
	}else{
		fprintf(istream[1], "rer\n");
		printf("receive::send_OK_NO:不明なレスポンスです\n");
		exit(1);
	}
	if(ferror(istream[1]) != 0){
		printf("receive::miss send\n");
	
	}
	//printf("receive::comp, send!!\n");
}

/*
他社への振り込みデータに対するレスポンスを待つ関数
*/
int wait_OK(){
	//printf("main::wait_respons\n");
	char buff[6];
	char *val;
	while(1){
		val = fgets(buff, sizeof(buff), istream[0]);
		if(val == NULL && ferror(istream[0]) != 0){
			perror("main::wait_OK:fgets:error\n");
			exit(1);
		}
		printf("main::back %s\n", buff);
		if(buff[0] == 'r') break;
		sleep(0.01);
	}
	
	if(strcmp(buff, "rok\n") == 0){//相手に振り込める場合
		return 0;
	}else if(strcmp(buff, "rno\n") == 0){//相手の残高が足りずタイムアウトした場合
		return 1;
	}else if(strcmp(buff, "rtm\n") == 0){//相手のmutexが取れずタイムアウトした場合
		return -1;
	}else if(strcmp(buff, "rer\n") == 0){
		printf("main::wait_OK:相手のプロセスがエラーです \n");
		exit(1);
	}
}

/*
ソケット通信を切断する関数
*/
void close_socket(int mode){
	printf("通信を切ります\n");
	int val;
	val = fclose(istream[0]);
	if(val != 0){
		perror("close(sockfd):");
		exit(1);
	}
	
	if(mode == 1){
		val = close(listen_sockfd);
		if(val == -1){
			perror("close(listen_sockfd):");
			exit(1);
		}
	}



}


