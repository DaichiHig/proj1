// tx-func.c
#include <stdio.h>
#include <pthread.h>
#include "receive.h"

//外部のグローバル変数の参照----------------------------------------------------------------------------------------------
//read_data.cからの参照
extern char tx[TRANS_COUNT]; //振替のモードを表す文字t(自社内の振替),o(他社との振替)を格納する。
extern int from[TRANS_COUNT], to[TRANS_COUNT]; //添字の振替操作の振り込み主、振り込み先アカウントの番号
extern int amount[TRANS_COUNT]; //添字の振替操作の振替金額

//connect_sock.cからの参照
extern int sockfd[2]; //sockfd[0]は相手の銀行に送金を行うソケット、sockfd[1]は相手の送金を受金するソケット

//グローバル変数-----------------------------------------------------------------------------------------------------------
//同期関連
pthread_mutex_t mutex; //mutexのアドレスの初期化
pthread_cond_t cvar; //条件変数のアドレスの初期化

///タイマー関連
struct timeval now; //今の時間を取得する変数の宣言
struct timespec timeout; //timedwaitの時限を設定する変数
int retcode; //timedwaitの返り値を格納する変数。時限に達したかどうかがわかる

//スレッドの関数----------------------------------------------------------------------------------------------------------




/*
関数:busy
引数:money int型で振替の金額を渡す
戻り値: なし
操作: 振替の金額の分だけ、ただfor文をまわす
*/
void busy(int money){
	for(int i=0; i < money * 1000; i++){
	}
}



/*

*/
void *receive(void *arg){
	int val = (int)arg;
	
}

/*
自社の振替操作を行うスレッド

*/
void *main_tx(void *arg){
	
	int thn = (int)arg; //スレッドの番号を表す
	
	//タイムアウトしたときに保存用として用いる変数
	int timeout_from;
	int timeout_to;
	int timeout_amount;
	
	
	for (int i=0; i < TRANS_COUNT/10; i++){ //iは自分が担当している振替操作の中で、何番目かを表している。
		pthread_mutex_lock(&mutex); //mutexのロック
		gettimeofday(&now, NULL); //現在の時刻を取得する。
		timeout.tv_sec = now.tv_sec + TIMEOUT_TIME; //現在の時刻のTIMEOUT_TIME秒後を時限とする。
		timeout.tv_nsec = now.tv_usec * 1000; //ナノ秒単位も時限に設定する。
		retcode = 0; //時限に達したかどうかを示すフラグ
		
		//次のwhile文では、振替元の残高が足りなかった場合は、timedwaitでmutexを解除して条件変数cvarにシグナルが送られるまで待機する。
		//シグナルが送られてくるとtimedwaitがmutexを再獲得してwhile文の判定に戻る。
		//シグナルが送られたとき時限に達していた場合はtimedwaitからETIMEDOUTが返されて、次の判定でwhile文を出る。このときtimedwaitはmutexを再獲得する。
		while(account[from[thn * TRANS_COUNT/10 + i]] < amount[thn * TRANS_COUNT/10 + i] && retcode != ETIMEDOUT){
			//printf("wait\n");//テスト用
			retcode = pthread_cond_timedwait(&cvar, &mutex, &timeout);
		}
		
		//前のwhileを抜け出した要因がタイムアウトではなかった場合は、自分の担当のi番目の操作を行う。
		if(retcode != ETIMEDOUT){
		
			//printf("%d:%d:%d:%d\n", thn, from[thn * TRANS_COUNT/10 + i],to[thn * TRANS_COUNT/10 + i],amount[thn * TRANS_COUNT/10 + i]);
			busy(amount[thn * TRANS_COUNT/10 + i]);
			account[from[thn * TRANS_COUNT/10 + i]] -= amount[thn * TRANS_COUNT/10 + i];
			account[to[thn * TRANS_COUNT/10 + i]] += amount[thn * TRANS_COUNT/10 + i];
			pthread_cond_signal(&cvar); //条件変数にシグナルを送る。
			
			//mainのテスト用の変数に添字を足す
			//did_trans += thn * TRANS_COUNT/10 + i;
			//printf("do\n");//テスト用
			
		}else{//タイムアウトのときは操作の順番を一つ繰り上げて、タイムアウトした振替は担当の一番最後にまわす。
		
			//テスト用の表示
			//printf("%dの振替をタイムアウトしました\n",  thn * TRANS_COUNT/10 + i);
			
			//タイムアウトした操作を上書きしないために保存する
			timeout_from = from[thn * TRANS_COUNT/10 + i];
			timeout_to = to[thn * TRANS_COUNT/10 + i];
			timeout_amount = amount[thn * TRANS_COUNT/10 + i];
			
			// 次以降の振替操作から担当の一番最後までの振替を一つ繰り上げる。
			for (int k=i; k < TRANS_COUNT/10; k++){
				from[thn * TRANS_COUNT/10 + k] = from[thn * TRANS_COUNT/10 + k + 1];
				to[thn * TRANS_COUNT/10 + k] = to[thn * TRANS_COUNT/10 + k + 1];
				amount[thn * TRANS_COUNT/10 + k] = amount[thn * TRANS_COUNT/10 + k + 1];
			}
			
			//タイムアウトした振替を配列の担当の一番最後に格納する。
			from[(thn + 1) * TRANS_COUNT/10 - 1 ] = timeout_from;
			to[(thn + 1) * TRANS_COUNT/10 - 1 ] = timeout_to;
			amount[(thn + 1) * TRANS_COUNT/10 - 1 ] = timeout_amount;
			
			//このままiをカウントアップすると一つ振替を抜かしてしまうため一つカウントダウンしておく
			i --;
		}
		
		//mutexの解除
		pthread_mutex_unlock(&mutex);
		
		//テスト用の表示
		//if(account[from[thn * TRANS_COUNT/10 + i]]<0){
		//	printf("残高の下限を超えてしまっています\n");
		//}
	}

}

