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

char *get_filename(char *buff){
	if(*buff == '/'){
		char *p, *fname;
		buff++;
		p = strchr(buff, ' ');
		if (p==0){//エラー
			printf("Illegal format\n");
			exit(1);	
		}
		*p = '\0';//Null文字の格納
		if (p==buff) return "index.html";//アクセスファイルが空欄だったとき
		fname = malloc(strlen(buff)+ 1);
		strcpy(fname, buff);//文字列データのコピー
		return fname;
	}else{//
		printf("Unknown header %s\n", buff);
		exit(1);
	}
}



int main(){
	int sockfd, new_sockfd;
	int val;
	int do_get_filename = 1;
	int count = 0;

	char buff[1024];
	char *p;
	char *req_filename;
	//char dat[2]; //受取データ抽出用
	//char buff_re[128];
	//int dat_int;
	int flag = 1;
	
	FILE *istream, *file;
	
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
	
	while(1){
		
	
		//要求の受付
		new_sockfd = accept(sockfd, NULL, NULL);
		if(new_sockfd == -1){
			perror("acceot:");
			exit(1);
		}
	
		//クライアントからリクエストデータを受け取る
		istream = fdopen(new_sockfd, "r+");//ソケットをストリームに変換する
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
			p = strchr(buff, '/');
			if(do_get_filename==1){
				req_filename = get_filename(p);
				do_get_filename = 0;
				//printf("doreq\n");
			}
			//count ++;
			if(strcmp("\r\n", buff) == 0){
				break;
			}
		}
		printf("request file:%s\n", req_filename);
		
		
		//レスポンス
		//fprintf(istream, "HTTP/1.1 200 OK\r\ncontent-Type:text/html\r\n\r\nHello\r\n\r\n");
		file = fopen(req_filename, "r");
		//ヘッダの送信
		while(1){
			if(fgets(buff, 1024, file) == 0){
				printf("fgets:error\n");
				break;
			}
			p = strchr(buff, '\n');
			if (p!=0) *p = '\0';
			fprintf(istream, "%s\r\n", buff);
			
			if(strcmp("\0", buff) == 0){
				break;
			}
		}
		//ボディの送信
		while(1){
			if(fgets(buff, 1024, file) == 0){
				printf("fgets:error\n");
				break;
			}
			p = strchr(buff, '\n');
			if (p!=0) *p = '\0';
			fprintf(istream, "%s\r\n", buff);
			
			if(strcmp("\0", buff) == 0){
				break;
			}
		}
		val = fclose(file);
		if(val != 0){
			perror("fclose(file):");
			exit(1);
		}
		
		sleep(1);
		val = fclose(istream);
		if(val != 0){
			perror("fclose:");
			exit(1);
		}
		/*
		val = close(new_sockfd);
		if(val == -1){
			perror("close(new_sockfd):");
			exit(1);
		}
		*/
	}
	
	
	val = close(sockfd);
	if(val == -1){
		perror("close(sockfd):");
		exit(1);
	}
	
	
	
}

