/* cy22226 日暮大地 tx_func.c*/
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
//外部ファイルのヘッダをインクルード、プロトタイプされた関数のみ参照可----------------------------------------------------
#include "connect_sock.h"
#include "manege_data.h"
#include "tx-func.h"

#define TRANS_COUNT 5000 //txの数
#define ACCOUNT_NUM 1000 //アカウントの数
#define TIMEOUT_TIME 0.01 //timedwaitで用いる制限時間
#define THREAD_SETNUM 1 //main_tx,receiveのスレッドセットの数。マルチスレッド化を実装するために宣言したが、このコードは1組までしか対応できなかった。

//グローバル変数-----------------------------------------------------------------------------------------------------------

int endtick; 

//スレッドの関数----------------------------------------------------------------------------------------------------------


/*
timedwaitに定期的にシグナルを送るスレッド、これが無いとデッドロックする危険がある。
実際にはmain_tx,receiveともに停止しても、定期的にシグナルがどこからか送られているようだが、当てにしない。

*/
void *timer_tick_sig(void *arg){
	int thn = (int)arg;
	
	while(1){
		sleep(0.1);
		inform_didtx();
		if(endtick == 1) break;
		
	}
	
}


/*
他社からの振り込みを受け取るスレッド
*/
void *receive(void *arg){
	int thn = (int)arg;
	char *txdat;
	int val;
	
	
	while(1){
		
		txdat = receive_txdata();//connect_sock.c
		//sleep(0.1);
		
		//printf("receive::printtxdat\n");
		if(txdat[0] == 'E') break;//スレッドの終了
		printf("receive::into_getmutex\n");
		val = get_timedmutex(thn, 0.1);
		if(val != 0){
			printf("receive::timedmutex:timeout!!\n");
			//タイムアウトをレスポンスする。
			send_OK_NO(-1);
		}else{
			printf("receive::comp, getmutex\nreceive::into_timerset\n");
			timer_set(thn, TIMEOUT_TIME);
			printf("receive::into_check\n");
			val = start_check_receive(txdat, thn);
			printf("receive::comp_check\n");
			free(txdat);
			printf("receive::free,txdat\n");
			if(val == ETIMEDOUT){
				printf("receive::sen_NO\n");
				send_OK_NO(1);
			
			}else{
				printf("receive::sen_OK\n");
				send_OK_NO(0);
			}
			printf("receive::unlock_mutex\n");
			give_back_mutex();
			inform_didtx();
		}
		
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
	char *buff;
	int tx_count_in_rota;
	int mass_txdid = 0;
	//タイムアウトしたときに保存用として用いる変数manege_data.cへ移動
	/*
	int timeout_from;
	int timeout_to;
	int timeout_amount;
	*/
	while (1){
		tx_count_in_rota = 0;
		for (int i=0; i < taskuni; i++){ //iは自分が担当している振替操作の中で、何番目かを表している。
			nowtask = tasknum * taskuni + i;
			printf("main::nowtask:%d\n", nowtask);
			int val;
			printf("main::into_getmutex\n");
			getmutex();
			printf("main::comp, getmutex\n");
			
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
			val = check_task(nowtask);
			if(val == 0){//すでに処理したtxの場合
				give_back_mutex();
				continue;
			}
			
			
			
		//タイマーの時限の設定をする関数manege_data.c
			//printf("main::into settimer\n");
			timer_set(thn, 1);
		/*
		gettimeofday(&now, NULL); //現在の時刻を取得する。
		timeout.tv_sec = now.tv_sec + TIMEOUT_TIME; //現在の時刻のTIMEOUT_TIME秒後を時限とする。
		timeout.tv_nsec = now.tv_usec * 1000; //ナノ秒単位も時限に設定する。
		retcode = 0; //時限に達したかどうかを示すフラグ
		*/
		
		//次のwhile文では、振替元の残高が足りなかった場合は、timedwaitでmutexを解除して条件変数cvarにシグナルが送られるまで待機する。
		//シグナルが送られてくるとtimedwaitがmutexを再獲得してwhile文の判定に戻る。
		//シグナルが送られたとき時限に達していた場合はtimedwaitからETIMEDOUTが返されて、次の判定でwhile文を出る。このときtimedwaitはmutexを再獲得する。
			printf("main::into start_check\n");
			val = start_check_tx(nowtask, thn);
		/*
		while(account[from[thn * TRANS_COUNT/10 + i]] < amount[thn * TRANS_COUNT/10 + i] && retcode != ETIMEDOUT){
			//printf("wait\n");//テスト用
			retcode = pthread_cond_timedwait(&cvar, &mutex, &timeout);
		}
		*/
		
		//前のwhileを抜け出した要因がタイムアウトではなかった場合は、自分の担当のi番目の操作を行う。
			if(val != ETIMEDOUT){
				printf("main::go_tx!!\n");
				if(check_tx_type(nowtask) == 'o'){
				//txデータをまるごと送る
				//贈りたいデータの取得
					buff = make_send_data(nowtask);
					//printf("main::comp,moke_send:\n");
					if(buff == NULL){
						printf("main::send_data失敗!!\n");
						postpone_tx(nowtask, endtask);
						printf("main::unlock!!\n");
						give_back_mutex();
						i --;
						continue;
					}
					send_txdata(buff);//相手にデータを送って相手側の承認を待つ
					printf("main::comp,send\n");
					free(buff);
					printf("main::free,buff\n");
					val = wait_OK();
					printf("%d\n", val);
					if(val == 1){//送金が取り消された場合
						//postpone_tx(nowtask, endtask);
						//i -- ;
						printf("main::unlock!!\n");
						give_back_mutex();
						printf("main::送金を取り消しました\n");
						continue;
					}else if(val == -1){//相手のmutexが獲得できなかった場合、時間をあけてもう一度
						//postpone_tx(nowtask, endtask);//この関数を使うと、このtxの操作を一番最後に回せる
						printf("main::unlock!!\n");
						give_back_mutex();
						printf("main::相手のmutex取れず\n");
						sleep(0.01);
						i --;
						continue;
					}
				}
				//printf("main::into_do_tx\n");
				do_tx(nowtask);//
				tx_count_in_rota ++;
				//printf("main::comp_do_tx\n");
			
			}else{
				//未処理txフラグを新たに作成、tx一周の処理で一つも処理が行われなかっった場合、可能な操作は完了とする。
				printf("main::timeout!!\n");
			}	
		
			//mutexの解除
			printf("main::unlock_mutex\n");
			give_back_mutex();
			inform_didtx();
		}
		
		mass_txdid += tx_count_in_rota;
		printf("1ローテ%d:###################################\n",tx_count_in_rota);
		printf("全部で%d:####################################\n", mass_txdid);
		if(tx_count_in_rota == 0){//tx処理が１周で一回も行えなかった場合
			printf("main::tx終了\n");
			break;
		
		}
	}
	
	send_txdata("E--0--0--0\n");//スレッド終了を送信
	endtick = 1;
}

/*
スレッドの生成から終了まで行う関数
*/
void do_tx_thread(){
	//スレッドの初期化
	pthread_t th[THREAD_SETNUM*2+1]; //マルチスレッドで行くなら[マルチする数][2]かな
	endtick = 0;
	int val;
	
	
	//スレッドを生成
	for(int i=0; i < THREAD_SETNUM; i++){
		val = pthread_create(&th[i*2], NULL, main_tx, (void *)(i*2));
		if(val != 0){
			perror("pthrad_create:");
			exit(1);
		}	
		val = pthread_create(&th[i*2+1], NULL, receive, (void *)(i*2+1));
		if(val != 0){
			perror("pthrad_create:");
			exit(1);
		}
	}
	
	
	val = pthread_create(&th[THREAD_SETNUM*2], NULL, timer_tick_sig, (void *)(THREAD_SETNUM*2));
	if(val != 0){
		perror("pthrad_create:");
		exit(1);
	}
	
	for (int i=0; i < THREAD_SETNUM*2 +1 ; i++){//スレッドの終了
		pthread_join(th[i], NULL);
	}
}



