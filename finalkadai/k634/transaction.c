/* cy22226 日暮大地 transaction.c*/
//main関数を含むファイル
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
	
	//mutex,状態変数の初期化を行う外部関数:tx-func.cより
	set_sync();
	
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
	
	//最初の合計金額の表示:manege_dataより
	printf("最初の合計金額：%d円\n", sumAmount());	
	
	//スレッドの立ち上げ--------------------------------------------------------
	//スレッドの初期化と生成を行って、スレッドの終了まで行う外部関数:tx-func.cより
	do_tx_thread();
	
	//最後の合計金額の表示:manege_dataより
	printf("最後の合計金額：%d円\n", sumAmount());
	
	//ソケットの切断--------------------------------------------------------------
	//:connect_sock.c
	close_socket(mode);
	printf("処理完了!!!\n");

}


