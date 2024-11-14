//cy22226 日暮大地　websrv.c 余力まで完了 
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define HTML_PORTNUM 80//サーバの通信に使われるポート番号。

struct sockaddr_in serv_addr;//ソケットのアドレスの構造体を宣言

//ファイル名が先頭の文字列buffから、ファイルの名前を抜き出して返す関数、指定がなければindex.htmlを返す。
char *get_filename(char *buff){
	if(*buff == '/'){
		char *p, *fname;
		buff++;
		p = strchr(buff, ' '); //ファイル名の終わりのアドレス
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


/*
webページのサーバを起動するプログラム
同じディレクトリにindex.html,testpage.html,errpage.html,post.html,form.html,favicon.icoが必要
クライアントから要求を受け取ってクライアントにファイルデータを送信できる。
今回ファイルにはページを推移できるようにリンクを載せた。具体的なそれぞれのファイルの仕様は次である。
index.html　Helloと表示する。　testpage.html,form.htmlに飛べるリンクを表示する。
testpage.html wold!と表示する。　index.htmlに飛べるリンクを表示する。
errpage.html 404 can't define this file! と表示する。　index.htmlに飛べるリンクを表示する。
form.html 入力ボックスを表示して、フォームの送信ができる。　　index.htmlに飛べるリンクを表示する。
post.html OK Your address is 「フォーム入力内容」と表示する。　index.htmlに飛べるリンクを表示する。
余力完了
*/
int main(){
	int sockfd, new_sockfd; //ソケット用ファイルディスクリプタ、sockfdはlisten用ソケット、new_sockfdはデータのやり取り用ソケット
	int val; //システムコールの引数を格納する変数。エラー処理に用いる
	
	int do_get_filename; //要求ファイル名取得を行わせるフラグ

	char buff[1024]; //文字列データの転送や読み取りに使う領域
	char *req_val; //リクエストをfgetsで取得するときの引数。不明なリクエストの処理に用いる
	char *p; //文字列中の文字検索strchrの結果アドレスを格納する変数
	char *req_filename; //要求されたファイル名を格納する文字列
	char cliadd[1024];
	
	int flag = 1; //ソケットのオプションの設定に用いるフラグ
	int mode, m; //modeは-1のときモード未確定、0のときGET、1のときはPOSTモードを表す。mはcontent-lengthを読み込むためのフラグ
	
	int contentlen;
	
	FILE *istream, *file; //ストリーム、istreamはソケット用、fileはローカルファイルの読み込み用
	
	//ソケットの作成
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1){
		perror("socket:");
		exit(1);
	}
	
	
	//アドレスの作成
	memset(&serv_addr, 0, sizeof(struct sockaddr_in)); //ソケットが使用するアドレスを設定
	serv_addr.sin_family = AF_INET; //各メンバの設定
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
	
	
	//acceptを行い、要求されたデータをクライアントに送ることを繰り返すwhile文、
	//リクエストのヘッダが不正な形であればcontinueでやり直す
	//リクエストのファイルが存在しなければ、errpageに飛ばす。
	while(1){
		do_get_filename = 1; //要求ファイル名取得を行わせるフラグの初期化
		mode = -1; //モードの初期化
		m = 1; //content-length取得フラグの初期化
		//要求の受付
		new_sockfd = accept(sockfd, NULL, NULL);
		if(new_sockfd == -1){
			perror("accept:");
			exit(1);
		}
		printf("accept%d\n", new_sockfd);
	
		//クライアントからリクエストデータを受け取る
		istream = fdopen(new_sockfd, "r+"); //ソケットをストリームに変換する
		if(istream == NULL){
			perror("fdopen:");
			exit(1);
		}
		val = setvbuf(istream, NULL, _IONBF, 0);
		if(val != 0){
			perror("setvbuf:");
			exit(1);
		}
		
		//リクエストのヘッダとボディを読み込んで、ターミナルに表示するwhile文.
		//fgetsで一行ずつ読む
		while(1){
			req_val = fgets(buff, 1024, istream);
			if(req_val == 0){
				printf("fgets:non reqest\n");
				break;
			}
			printf("%s", buff);
			if(do_get_filename==1){ //フラグをみて、要求ファイル名の取得を行う
				p = strchr(buff, '/'); //ファイル名の直前にある文字のアドレスを取得して、文字列pとする
				req_filename = get_filename(p); //ファイル名の取得
				do_get_filename = 0;//フラグをオフにする
			}
			if(mode == -1){
				p = strchr(buff, ' ');
				*p = '\0';
				if(strcmp(buff, "GET") == 0) mode = 0;
				else if(strcmp(buff, "POST") == 0) mode = 1;
				else mode = 0;
				*p = ' ';
			}
			
			if(mode == 1 && m == 1){
				p = strstr(buff, "Content-Length:");
				if(p != NULL){
					p = strchr(buff, '\r');
					*p = '\0';
					p = strchr(buff, ' ');
					contentlen = atoi(p);
					m = 0;
				}
				
			}
			
			if(strcmp("\r\n", buff) == 0){ //この行が改行のみの場合、リクエストの終わり
				break;
			}
		}
		if(req_val == 0){ 
		//理由はよくわからないが、ファイアーフォックスでリンクにカーソルをかざすと、リクエストが空のアクセスが来るのでその処理
			printf("不明なリクエストが確認されました\n");
			sleep(1);
			val = fclose(istream); //ソケットのクローズ
			if(val != 0){
				perror("fclose:");
				exit(1);
			}
			continue;//リンクをカーソルでかざすといちいちプログラムが終了してしまうので、while文の先頭に戻す
		}
		//なおリンクのクリックは問題なく行える
		
		//bodyの表示
		if(mode == 1){
			req_val = fgets(cliadd, contentlen + 1, istream);
			if(req_val == 0 && ferror(file) != 0){
				printf("fgets:content:err \n");
			}
			printf("%s\n", cliadd);
		}
		
		printf("request file:%s\n", req_filename);
		
		
		/////////////////////////////レスポンス///////////////////////////////////////////////////
		file = fopen(req_filename, "r"); //要求されたファイルを開く
		if(file == NULL){ //ファイルが見つからない場合
			perror("fopen:");
			file = fopen("errpage.html", "r"); //「ページが見つからない」ページに移動
			if(file == NULL){ //ファイルがどうしても開けない場合はサーバを終了する
				perror("fopen(errpage.html):");
				exit(1);
			}
		}
		
		//ヘッダの送信
		while(1){
			if(fgets(buff, 1024, file) == 0){
				if(ferror(file) == 0){
					printf("request file:head only end\n");
				}else{
					printf("fgets:error1\n");
					exit(1);
				}
				break;
			}
			p = strchr(buff, '\n'); //改行を\r\nに変換して書き込む
			if (p!=0) *p = '\0';
			fprintf(istream, "%s\r\n", buff);
			
			if(strcmp("\0", buff) == 0){ //一行が改行だけだった場合はヘッダの終わり
				break;
			}
		}
		//ボディの送信
		while(1){
			if(fgets(buff, 1024, file) == 0){
				if(ferror(file) == 0){
					printf("conplete sending respons\n");
				}else{
					printf("fgets:error2\n");
					exit(1);
				}
				break;
			}
			p = strchr(buff, '\n'); //改行を\r\nに変換して書き込む
			if (p!=0) *p = '\0';
			fprintf(istream, "%s\r\n", buff);
		}
		
		if(mode == 1) {//入力情報の表示
			
			p = strchr(cliadd, '&');
			*p = '\0';
			fprintf(istream, "Your address is %s.\r\n", cliadd + 5);
			
			/*その他の情報の送信方法模索中
			*p = 't';
			p = strchr(cliadd, '&');
			*p = '\0';
			fprintf(istream, "Your name is %s.\r\n", cliadd + 10);
			/*
			i = strchr(p, '&');
			*i = '\0';
			if(strcmp(p+5, "on")){
				fprintf(istream, "Your are fine.\r\n");
			}else{
				fprintf(istream, "Your are not fine.\r\n");
			}
			i = strchr(p, '&');
			*i = '\0';
			fprintf(istream, "Your sex is %s.\r\n", p + 5);
			*/
		}
		
		val = fclose(file);//ページファイルのクローズ
		if(val != 0){
			perror("fclose(file):");
			exit(1);
		}
		
		sleep(1);
		val = fclose(istream);//ソケットのクローズ
		if(val != 0){
			perror("fclose:");
			exit(1);
		}
		
	}
	

	val = close(sockfd);//listen用ソケットのクローズ
	if(val == -1){
		perror("close(sockfd):");
		exit(1);
	}
	
	
	
}

