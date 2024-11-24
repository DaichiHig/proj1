#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "connect_sock.h"

#define PORTNUM 50000
struct sockaddr_in serv_addr;
int sockfd[2];
int listen_sockfd;
FILE *istream[2];


void make_sock_stream(){
	istream[0] = fdopen(sockfd[0], "r+");
	int val;
	if(istream[0] == NULL){
		perror("fdopen:\n");
		exit(1);
	}
	val = setvbuf(istream[0], NULL, _IONBF, 0);
	if(val != 0){
		perror("setvbuf1:");
		exit(1);
	}
	istream[1] = fdopen(sockfd[1], "r+");
	if(istream[1] == NULL){
		perror("fdopen:\n");
		exit(1);
	}
	val = setvbuf(istream[1], NULL, _IONBF, 0);
	if(val != 0){
		perror("setvbuf2:");
		exit(1);
	}
}


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
	
	//main側を立ち上げる
	sockfd[0] = accept(listen_sockfd, NULL, NULL);
	if(sockfd[0] == -1){
		perror("accept:");
		exit(1);
	}
	sockfd[1] = accept(listen_sockfd, NULL, NULL);
	if(sockfd[1] == -1){
		perror("accept:");
		exit(1);
	}
	make_sock_stream();
	printf("connected\n");
	
}

void be_client(){
	int val;
	int sock_count = 0;
	/*
	if(thread_mode == 'm'){
		mode = 0;
	}else if(thread_mode == 'r'){
		mode = 1;
	}else{
		printf("be_client():不明な引数\n");
		exit(1);
	}*/
	
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
	
	//コネクション要求
	val = connect(sockfd[1], (struct sockaddr *)&serv_addr, sizeof(struct sockaddr_in));
	if(val == -1){
	perror("connect:");
		exit(1);
	}
	val = connect(sockfd[0], (struct sockaddr *)&serv_addr, sizeof(struct sockaddr_in));
	if(val == -1){
	perror("connect:");
		exit(1);
	}
	make_sock_stream();
	printf("connected\n");
}


void send_txdata(char *buff){
	printf("main::in send_txdata:\n");
	printf("main::send_txdata:%s\n", buff);
	fprintf(istream[0], buff);
}

char *receive_txdata(){
	printf("receive::in_receive_txdata:\n");
	char buff[50];
	char *complete_input = malloc(50);
	if(buff == NULL){
		perror("receive:malloc:");
		exit(1);
	}
	char *val;
	/*
	val = fgets(buff, sizeof(buff), istream[1]);
	if(val == NULL && ferror(istream[1]) != 0){
		printf("receive::receive_txdata:fgets:error\n");
		exit(1);
	}
	*/
	
	while (fgets(buff, sizeof(buff), istream[1])) {
        	strcat(complete_input, buff);
       		// 改行文字が見つかれば終了
        	if (strchr(buff, '\n') != NULL) break;
    	}
	
	printf("receive::receive:%s\n", buff);
	return complete_input;

}

void send_OK_NO(int i){
	
	if(i = 0) fprintf(istream[1], "ok\n");
	else if(i = 1) fprintf(istream[1], "no\n");
	else{
		fprintf(istream[1], "er\n");
		printf("receive::send_OK_NO:不明なレスポンスです\n");
		exit(1);
	}
	if(ferror(istream[1]) != 0){
		printf("receive::miss send\n");
	
	}
}

int wait_OK(){
	char buff[5];
	char *val;
	val = fgets(buff, sizeof(buff), istream[0]);
	if(val == NULL && ferror(istream[0]) != 0){
		perror("main::wait_OK:fgets:error\n");
		exit(1);
	}
	printf("main::back %s\n", buff);
	if(strcmp(buff, "ok\n") == 0){
		return 0;
	}else if(strcmp(buff, "no\n") == 0){
		return 1;
	}else if(strcmp(buff, "er\n") == 0){
		printf("main::wait_OK:相手のプロセスがエラーです \n");
		return -1;
	}
}

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

/*
自身のsockfd[0]をmain_txに、sockfd[1]をreceiveに割り振って、それと対応する相手の役割を決定し送信する
*//*
int decide_sock_role(){
	char buff1[3], buff2[3];
	int val;
	
	istream[0] = fdopen(sockfd[0], "r+");
	if(istream[0] == NULL){
		perror("fdopen1:");
		exit(1);
	} 
	val = setvbuf(istream[0], NULL, _IONBF, 0);
	if(val != 0){
		perror("setvbuf1:");
		exit(1);
	}
	istream[1] = fdopen(sockfd[1], "r+");
	if(istream[1] == NULL){
		perror("fdopen2:");
		exit(1);
	}
	val = setvbuf(istream[1], NULL, _IONBF, 0);
	if(val != 0){
		perror("setvbuf2:");
		exit(1);
	}
	
	fprintf(istream[0],"receive\n");
	fprintf(istream[1],"main\n");
	val = fgets(buff1, sizeof(buff1[3]), istream[0]);
	if(val == 0){
		printf("fgets::");
		exit(1);
	}
	fgets(buff2, sizeof(buff2[3]), istream[1]);
	if(val == 0){
		printf("fgets::");
		exit(1);
	}
	
	if(strcmp(buff1, "OK")==0 && strcmp(buff2, "OK")==0){
		return 0;
	}else{
		printf("decide_sock_role():接続失敗\n");
	}
}*/

/*
int receive_sock_role(){
	FILE *istr[2];
	int val;
	char buff1[3], buff2[3];
	
	istr[0] = fdopen(sockfd[0], "r+");
	if(istr[0] == NULL){
		perror("fdopen3:");
		exit(1);
	} 
	val = setvbuf(istr[0], NULL, _IONBF, 0);
	if(val != 0){
		perror("setvbuf3:");
		exit(1);
	}
	istr[1] = fdopen(sockfd[1], "r+");
	if(istr[1] == NULL){
		perror("fdopen4:");
		exit(1);
	}
	val = setvbuf(istr[1], NULL, _IONBF, 0);
	if(val != 0){
		perror("setvbuf4:");
		exit(1);
	}
	
	val = fgets(buff1, sizeof(buff1[3]), istr[0]);
	if(val == 0){
		printf("fgets::");
		exit(1);
	}
	fgets(buff2, sizeof(buff2[3]), istr[1]);
	if(val == 0){
		printf("fgets::");
		exit(1);
	}

	if(strcmp("receive\n", buff1)==0 && strcmp("main\n", buff2)==0){
		
		istream[1] = fdopen(sockfd[0], "r+");
		istream[0] = fdopen(sockfd[1], "r+");
		
	}else if(strcmp("main\n", buff1)==0 && strcmp("receive\n", buff2)==0){
		istream[0] = fdopen(sockfd[0], "r+");
		istream[1] = fdopen(sockfd[1], "r+");
	}
	else{
		printf("receive_sock_role():接続失敗");
		fprintf("");
		exit
	}
}
*/
/*
int receive_sock_role(){




}*/

/*
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
	
	
	
}*/

