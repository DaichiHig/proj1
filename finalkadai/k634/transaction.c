/* cy22226 日暮大地 transaction.c*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

//外部ファイルのヘッダをインクルード、プロトタイプされた関数のみ参照可----------------------------------------------------
#include "connect_sock.h" //ソケットをつなげる関数を参照できる
#include "manege_data.h" //cvsファイルを読み込む関数を参照できる
#include "tx-func.h" //振替操作をするスレッド関数を参照できる

//
#define FIRST_MONEY 100000











/*
main
*/
int main(int argc, char *argv[]){
	int val;//エラー処理に使う変数
	int mode;

	//引数のエラー処理
	if(argc == 1){
		printf("used with argument: s or c\n");
		exit(1);
	}
	
	//初期設定-------------------------------------------------------------
	//アカウントの残高の初期化を行う外部関数:read_data.cより
	make_account_data(FIRST_MONEY);
	/*
	for(int i=0; i < ACCOUNT_NUM; i++) {
		account[i] = FIRST_MONEY;
	}
	*/
	
	
	//mutex,状態変数の初期化を行う外部関数:tx-func.cより
	set_sync();
	/*
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cvar, NULL);
	*/
	
	
	
	
	//ソケットをつなげる---------------------------------------------------
	if(argv[1][0] == 's'){
		mode = 1;
		printf("サーバを立ち上げます\n");
		be_server(); //サーバとしてソケットを接続する外部関数:connect_sock.cより
	}else if(argv[1][0] == 'c'){
		mode = 0;
		printf("クライアントを立ち上げます\n");
		be_client(); //クライアントとしてソケットを接続する外部関数:connect_sock.cより
	}else{
		printf("main:不明なモードです\n");
		exit(1);
	}
	
	
	//トランザクションデータの読み込み-------------------------------------
	//ファイルの読み込みを行う外部関数:manege_data.cより
	read_txdata(argv[1][0]);
	
	/*
	FILE *istream_f;
	
	if((istream_f = fopen("trans.csv", "r"))==NULL){ //課題用:trans.csv テスト用:random_data.csv
		printf("ファイルを開くことができません");
	}
	
	int tx_count=0;
	while(1){
		val = fscanf(istream_f, "%c,%d,%d,%d\n", &tx[tx_count], &from[tx_count], &to[tx_count], &amount[tx_count]);
		if(val == EOF){
			if(ferror(istream_f) == 0){
				printf("fscanf:complite-%d\n", tx);
				break;
			}
			else{
				printf("fscanf:error\n");
				exit(1);
			}
		}else if(val != 4){
			printf("読み込むデータの個数に間違いがあります");
			exit(1);
		}
		tx_count++;
	}
	val = fclose(istream_f);
	if(val != 0){
		perror("fclose:");
		exit(1);
	}
	*/
	
	//最初の合計金額の表示:manege_dataより
	printf("最初の合計金額：%d円\n", sumAmount());	
	
	//スレッドの立ち上げ--------------------------------------------------------
	//スレッドの初期化と生成を行う外部関数:tx-func.c
	do_tx_thread();
	
	/*
	//スレッドの初期化
	pthread_t th[2]; //マルチスレッドで行くなら[マルチする数][2]かな
	
	
	//スレッドを生成
	val = pthread_create(&th[0], NULL, main_tx, (void *)0);
	if(val != 0){
		perror("pthrad_create:");
		exit(1);
	}	
	val = pthread_create(&th[1], NULL, receive, (void *)0);
	if(val != 0){
		perror("pthrad_create:");
		exit(1);
	}
	*/
	
	//スレッドの終了--------------------------------------------------------------立ち上げと一緒の関数にした
	//:tx-func.c
	
	
	//最後の合計金額の表示:manege_dataより
	printf("最後の合計金額：%d円\n", sumAmount());
	
	//ソケットの切断--------------------------------------------------------------
	//:connect_sock.c
	close_socket(mode);


}


