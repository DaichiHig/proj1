/* cy22226 日暮大地 manege_data.c*/
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <errno.h>
#include "manege_data.h"

#define TRANS_COUNT 10000
#define ACCOUNT_NUM 1000
#define THREAD_SETNUM 1


//グローバル変数----------------------------------------------------------------------------------------------
//read_data.cからの参照
char tx[TRANS_COUNT]; //振替のモードを表す文字t(自社内の振替),o(他社との振替)を格納する。
int from[TRANS_COUNT], to[TRANS_COUNT]; //添字の振替操作の振り込み主、振り込み先アカウントの番号
int amount[TRANS_COUNT]; //添字の振替操作の振替金額
int task[TRANS_COUNT]; //未処理のtxには1、処理し終えたtxには0を格納するフラグ
int account[ACCOUNT_NUM];



//同期関連
pthread_mutex_t mutex; //mutexのアドレスの初期化
pthread_cond_t cvar; //条件変数のアドレスの初期化

///タイマー関連
struct timeval now[THREAD_SETNUM*2 + 1]; //今の時間を取得する変数の宣言
struct timespec timeout[THREAD_SETNUM*2 + 1]; //timedwaitの時限を設定する変数
int retcode[THREAD_SETNUM*2 + 1]; //timedwaitの返り値を格納する変数。時限に達したかどうかがわかる

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

void set_sync(){
	
	pthread_mutex_init(&mutex, NULL);
	
	pthread_cond_init(&cvar, NULL);
}

void getmutex(){
	int val;
	printf("in_getmutex\n");
	val = pthread_mutex_lock(&mutex); //mutexのロック
	if(val != 0){
		printf("%d", val);
	}
	
}

int get_timedmutex(int tout){
	int val;
	double timer;
	double nowtime;
	printf("in_get_timedmutex\n");
	timer_set(1, tout);//timeoutに時限を設定する
	timer = timeout[1].tv_sec + timeout[1].tv_nsec/1000000; //時限の計算を行う
	while (val != 0){
		gettimeofday(&now[1], NULL); //現在時刻を計算する
		nowtime = now[1].tv_sec + now[1].tv_usec/1000000000;
		if(nowtime > timer){//タイムアウトのとき
			return 1;
		}
		val = pthread_mutex_trylock(&mutex);
		sleep(0.01);
	}//mutexを獲得したとき
	return 0;
}

void give_back_mutex(){
	pthread_mutex_unlock(&mutex);
}

void inform_didtx(){
	pthread_cond_signal(&cvar);
}


void timer_set(int thn, int tout){
	gettimeofday(&now[thn], NULL); //現在の時刻を取得する。
	timeout[thn].tv_sec = now[thn].tv_sec + tout; //現在の時刻のTIMEOUT_TIME秒後を時限とする。
	timeout[thn].tv_nsec = now[thn].tv_usec * 1000; //ナノ秒単位も時限に設定する。
	retcode[thn] = 0; //時限に達したかどうかを示すフラグ
}



char check_tx_type(int i){
	return tx[i];
}

int check_task(int i){
	return task[i];
}



//次のwhile文では、振替元の残高が足りなかった場合は、timedwaitでmutexを解除して条件変数cvarにシグナルが送られるまで待機する。
//シグナルが送られてくるとtimedwaitがmutexを再獲得してwhile文の判定に戻る。
//シグナルが送られたとき時限に達していた場合はtimedwaitからETIMEDOUTが返されて、次の判定でwhile文を出る。このときtimedwaitはmutexを再獲得する。
int start_check_tx(int i, int thn){
	while(account[from[i]] < amount[i] && retcode[thn] != ETIMEDOUT){
		//printf("wait\n");//テスト用
		retcode[thn] = pthread_cond_timedwait(&cvar, &mutex, &timeout[thn]);
	}
	return retcode[thn];
}

int start_check_receive(char buff[50], int thn){
	char mo;
	int frm, t, am;
	
	sscanf(buff, "%c,%d,%d,%d\n", &mo, &frm, &t, &am);
	printf("receive::check:%c,%d,%d,%d\n", mo, frm, t, am);
	if(mo != 'o'){
		printf("receive::受け付けない振替\n");
		exit(1);
	}
	
	while(account[t] + am < 0 && retcode[thn] != ETIMEDOUT){
		//printf("wait\n");//テスト用
		retcode[thn] = pthread_cond_timedwait(&cvar, &mutex, &timeout[thn]);
	}
	if(retcode[thn] == ETIMEDOUT){
		printf("receive::timeout!!\n");
	}else{
		printf("receive::go!!");
		do_receive(t, am);
	}
	return retcode[thn];
}

void do_receive(int t, int am){
	account[t] += am; 
	if(account[t] < 0){
		printf("do_receive:error\n");
		exit(1);
	}
	
	printf("comp,receive!\n");
}

void do_tx(int i){
	//printf("%d:%d:%d:%d\n", thn, from[thn * TRANS_COUNT/10 + i],to[thn * TRANS_COUNT/10 + i],amount[thn * TRANS_COUNT/10 + i]);
	
	if(!(tx[i] == 'o' || tx[i] == 't')){
		printf("%s, %d, %d, %d,\n", tx[i], from[i], to[i], amount[i]);
		printf("do_tx:不明な振替操作です\n");
		exit(1);
	}
	
	busy(amount[i]);
	account[from[i]] -= amount[i];
	printf("main::tx_did\n");
	if(tx[i] == 't'){//相手銀行への送金の場合は振替先の操作は無い
		account[to[i]] += amount[i];
	}
	task[i] = 0;
	//mainのテスト用の変数に添字を足す
	//did_trans += thn * TRANS_COUNT/10 + i;
	//printf("do\n");//テスト用
	printf("main::comp%c, %d, %d, %d,\n", tx[i], from[i], to[i], amount[i]);
}



char *make_send_data(int i){
	//printf("make_send:\n");
	char *buff = malloc(40);
	if(buff == NULL){
		perror("make_send:malloc:");
		exit(1);
	}
	char data[6];
	printf("t\n");
	//charcat(buff , tx[i]);
	*buff = tx[i];
	buff[1] = '\0';
	strcat(buff, ",");
	sprintf(data, "%d", from[i]);
	strcat(buff, data);
	strcat(buff, ",");
	sprintf(data, "%d", to[i]);
	strcat(buff, data);
	strcat(buff, ",");
	sprintf(data, "%d\n", amount[i]);
	strcat(buff, data);
	//strcat(buff, "\n");
	printf("main::comp_make_send_data\n");
	return buff;
}


void postpone_tx(int nowtask, int endtask){
	int postpone_from;
	int postpone_to;
	int postpone_amount;
	int postpone_task;
	//テスト用の表示
	//printf("%dの振替をタイムアウトしました\n",  thn * TRANS_COUNT/10 + i);
			
	//タイムアウトした操作を上書きしないために保存する
	postpone_from = from[nowtask];
	postpone_to = to[nowtask];
	postpone_amount = amount[nowtask];
	postpone_task = task[nowtask];
			
	// 次以降の振替操作から担当の一番最後までの振替を一つ繰り上げる。
	for (int k=nowtask; k <= endtask; k++){
		from[k] = from[k + 1];
		to[k] = to[k + 1];
		amount[k] = amount[k + 1];
		task[k] = task[k + 1];
	}
			
	//タイムアウトした振替を配列の担当の一番最後に格納する。
	from[endtask] = postpone_from;
	to[endtask] = postpone_to;
	amount[endtask] = postpone_amount;
	task[endtask] = postpone_task;
}



/*
関数:sumAmountこの関数はmanege_data
全口座の総額を計算する関数
引数: なし
戻り値:sum 全口座の総額をint型で返す
操作: グローバル変数の配列accountを用いて全口座の総額を計算して戻り値として返す。
*/
int sumAmount(){
	int sum = 0;
	for(int i=0; i < ACCOUNT_NUM; i++){
		sum += account[i]; //口座の残高を足す
	}
	return sum;
}



void make_account_data(int first_maney){
	for(int i=0; i < ACCOUNT_NUM; i++) {
		account[i] = first_maney;
	}

}

/*
ファイルから振替のデータを取得する関数
*/
void read_txdata(char mode){
	FILE *istream_f;
	char filename[10];
	int val;
	char *val_gets;
	char buff[1024];
	//モード設定
	if(mode == 's'){
		strcpy(filename, "trans_s");
	}else if(mode == 'c'){
		strcpy(filename, "trans_c");
	}else{
		printf("read_txdata:正しいモードを渡されませんでした：\n");
		exit(1);
	}
	
	if((istream_f = fopen(filename, "r"))==NULL){ //課題用:trans.csv テスト用:random_data.csv
		printf("ファイルを開くことができません");
	}
	
	int tx_count=0;
	while(1){
		val_gets = fgets(buff, sizeof(buff), istream_f);
		if(val_gets == NULL ){
			if(ferror(istream_f) == 0){
				printf("fgets:complite%d\n", tx_count);
				break;
			}else{
				printf("fgets:error\n");
				exit(1);
			}
		}
		val = sscanf(buff, "%c,%d,%d,%d\n", &tx[tx_count], &from[tx_count], &to[tx_count], &amount[tx_count]);
		task[tx_count] = 1;
		//val = fscanf(istream_f, "%c,%d,%d,%d\n", &tx[tx_count], &from[tx_count], &to[tx_count], &amount[tx_count]);
		/*
		if(val == EOF){
			if(ferror(istream_f) == 0){
				printf("fscanf:complite%d\n", tx_count);
				break;
			}
			else{
				printf("fscanf:error\n");
				exit(1);
			}
		}else if(val != 4){
			
			printf("%d", val);
			printf("読み込むデータの個数に間違いがあります\n");
			exit(1);
			
			printf("fscanf:complite%d\n", tx_count);
			break;
		}
		*/
		tx_count++;
	}
	val = fclose(istream_f);
	if(val != 0){
		perror("fclose:");
		exit(1);
	}	
}








