/* cy22226 日暮大地 tx_func.c*/
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include "connect_sock.h"
#include "manege_data.h"
#include "tx-func.h"

#define TRANS_COUNT 10000
#define ACCOUNT_NUM 1000
#define TIMEOUT_TIME 1
#define THREAD_SETNUM 1

//外部のグローバル変数の参照----------------------------------------------------------------------------------------------
/*
//read_data.cからの参照
extern char tx[TRANS_COUNT]; //振替のモードを表す文字t(自社内の振替),o(他社との振替)を格納する。
extern int from[TRANS_COUNT], to[TRANS_COUNT]; //添字の振替操作の振り込み主、振り込み先アカウントの番号
extern int amount[TRANS_COUNT]; //添字の振替操作の振替金額


//connect_sock.cからの参照
extern int sockfd[2]; //sockfd[0]は相手の銀行に送金を行うソケット、sockfd[1]は相手の送金を受金するソケット
*/
//グローバル変数-----------------------------------------------------------------------------------------------------------
//manege_data.cに移動
/*
//同期関連
pthread_mutex_t mutex; //mutexのアドレスの初期化
pthread_cond_t cvar; //条件変数のアドレスの初期化

///タイマー関連
struct timeval now; //今の時間を取得する変数の宣言
struct timespec timeout; //timedwaitの時限を設定する変数
int retcode; //timedwaitの返り値を格納する変数。時限に達したかどうかがわかる
*/

//スレッドの関数----------------------------------------------------------------------------------------------------------




/*
関数:busy
引数:money int型で振替の金額を渡す
戻り値: なし
操作: 振替の金額の分だけ、ただfor文をまわす
*/
/*
void busy(int money){
	for(int i=0; i < money * 1000; i++){
	}
}
*/

//manege_data.cに移動
/*
void timer_stop(){


}

void timer_set(){
	gettimeofday(&now, NULL); //現在の時刻を取得する。
	timeout.tv_sec = now.tv_sec + TIMEOUT_TIME; //現在の時刻のTIMEOUT_TIME秒後を時限とする。
	timeout.tv_nsec = now.tv_usec * 1000; //ナノ秒単位も時限に設定する。
	retcode = 0; //時限に達したかどうかを示すフラグ
}
*/




/*

*/
void *receive(void *arg){
	int thn = (int)arg;
	char *txdat;
	int val;
	
	
	while(1){
		
		txdat = receive_txdata();//connect_sock.c
		if(txdat[0] = 'E') break;//スレッドの終了
		
		getmutex();
		timer_set(thn, TIMEOUT_TIME);
		val = start_check_receive(txdat, thn);
		
		if(val == ETIMEDOUT){
			send_OK_NO(1);
		
		}else{
			send_OK_NO(0);
		}
		give_back_mutex();
	}
}

/*
自社の振替操作を行うスレッド

*/
void *main_tx(void *arg){
	
	int thn = (int)arg; //スレッドの番号を表す
	
	int tasknum = thn / 2;
	int taskuni = TRANS_COUNT/THREAD_SETNUM;
	int nowtask;
	int endi = taskuni - 1;
	int endtask = tasknum * taskuni + endi;
	//タイムアウトしたときに保存用として用いる変数manege_data.cへ移動
	/*
	int timeout_from;
	int timeout_to;
	int timeout_amount;
	*/
	
	for (int i=0; i < taskuni; i++){ //iは自分が担当している振替操作の中で、何番目かを表している。
		nowtask = tasknum * taskuni + i;
		int val;
		getmutex();
		/*
		pthread_mutex_lock(&mutex); //mutexのロック	これはそれぞれのmanege_data関数に入れたほうがよいかな
		*/
		
		/*
		if(check_tx_type(nowtask) == 'o'){
			//txデータをまるごと送る
			//贈りたいデータの取得
			send_data(make_send_data(nowtask));
			if(wait_OK() == 0){
				do_tx();
			
			
			}
		
		
		
		
		}
		*/
		
		//タイマーの時限の設定をする関数manege_data.c
		timer_set(thn, TIMEOUT_TIME);
		/*
		gettimeofday(&now, NULL); //現在の時刻を取得する。
		timeout.tv_sec = now.tv_sec + TIMEOUT_TIME; //現在の時刻のTIMEOUT_TIME秒後を時限とする。
		timeout.tv_nsec = now.tv_usec * 1000; //ナノ秒単位も時限に設定する。
		retcode = 0; //時限に達したかどうかを示すフラグ
		*/
		
		//次のwhile文では、振替元の残高が足りなかった場合は、timedwaitでmutexを解除して条件変数cvarにシグナルが送られるまで待機する。
		//シグナルが送られてくるとtimedwaitがmutexを再獲得してwhile文の判定に戻る。
		//シグナルが送られたとき時限に達していた場合はtimedwaitからETIMEDOUTが返されて、次の判定でwhile文を出る。このときtimedwaitはmutexを再獲得する。
		val = start_check_tx(nowtask, thn);
		/*
		while(account[from[thn * TRANS_COUNT/10 + i]] < amount[thn * TRANS_COUNT/10 + i] && retcode != ETIMEDOUT){
			//printf("wait\n");//テスト用
			retcode = pthread_cond_timedwait(&cvar, &mutex, &timeout);
		}
		*/
		
		//前のwhileを抜け出した要因がタイムアウトではなかった場合は、自分の担当のi番目の操作を行う。
		if(val != ETIMEDOUT){
			
			if(check_tx_type(nowtask) == 'o'){
				//txデータをまるごと送る
				//贈りたいデータの取得
				send_txdata(make_send_data(nowtask));//相手にデータを送って相手側の承認を待つ
				val = wait_OK();
				if(val != 0){//送金が取り消された場合
					postpone_tx(nowtask, endtask);
					i -- ;
					give_back_mutex();
					continue;
				}
			}
			do_tx(nowtask);//
			
			/*
			//printf("%d:%d:%d:%d\n", thn, from[thn * TRANS_COUNT/10 + i],to[thn * TRANS_COUNT/10 + i],amount[thn * TRANS_COUNT/10 + i]);
			busy(amount[thn * TRANS_COUNT/THREAD_NUM + i]);
			account[from[thn * TRANS_COUNT/THREAD_NUM + i]] -= amount[thn * TRANS_COUNT/THREAD_NUM + i];
			account[to[thn * TRANS_COUNT/THREAD_NUM + i]] += amount[thn * TRANS_COUNT/THREAD_NUM + i];
			pthread_cond_signal(&cvar); //条件変数にシグナルを送る。
			
			//mainのテスト用の変数に添字を足す
			//did_trans += thn * TRANS_COUNT/10 + i;
			//printf("do\n");//テスト用
			*/
		}else{//タイムアウトのときは操作の順番を一つ繰り上げて、タイムアウトした振替は担当の一番最後にまわす。manege_data.c
			postpone_tx(nowtask, endtask);
			/*
			//テスト用の表示
			//printf("%dの振替をタイムアウトしました\n",  thn * TRANS_COUNT/10 + i);
			
			//タイムアウトした操作を上書きしないために保存する
			timeout_from = from[thn * TRANS_COUNT/THREAD_NUM + i];
			timeout_to = to[thn * TRANS_COUNT/THREAD_NUM + i];
			timeout_amount = amount[thn * TRANS_COUNT/THREAD_NUM + i];
			
			// 次以降の振替操作から担当の一番最後までの振替を一つ繰り上げる。
			for (int k=i; k < TRANS_COUNT/THREAD_NUM; k++){
				from[thn * TRANS_COUNT/THREAD_NUM + k] = from[thn * TRANS_COUNT/THREAD_NUM + k + 1];
				to[thn * TRANS_COUNT/THREAD_NUM + k] = to[thn * TRANS_COUNT/THREAD_NUM + k + 1];
				amount[thn * TRANS_COUNT/THREAD_NUM + k] = amount[thn * TRANS_COUNT/THREAD_NUM + k + 1];
			}
			
			//タイムアウトした振替を配列の担当の一番最後に格納する。
			from[(thn + 1) * TRANS_COUNT/THREAD_NUM - 1 ] = timeout_from;
			to[(thn + 1) * TRANS_COUNT/THREAD_NUM - 1 ] = timeout_to;
			amount[(thn + 1) * TRANS_COUNT/THREAD_NUM - 1 ] = timeout_amount;
			*/
			//このままiをカウントアップすると一つ振替を抜かしてしまうため一つカウントダウンしておく
			i --;
		}
		
		//mutexの解除
		give_back_mutex();
		/*
		pthread_mutex_unlock(&mutex);
		*/
		//テスト用の表示
		//if(account[from[thn * TRANS_COUNT/10 + i]]<0){
		//	printf("残高の下限を超えてしまっています\n");
		//}
	}
	send_txdata("E--0--0--0\n");//スレッド終了を送信
	
}

void do_tx_thread(){
	//スレッドの初期化
	pthread_t th[2]; //マルチスレッドで行くなら[マルチする数][2]かな
	int val;
	
	
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
	
	for (int i=0; i < 2; i++){//スレッドの終了
		pthread_join(th[i], NULL);
	}
}



