// transaction.c cy22226 higurashi daichi
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/time.h>
#define TRANS_COUNT 10000
#define FIRST_MANY 10000
#define ACCOUNT_NUM 100

int account[ACCOUNT_NUM]; //添字番号のアカウントの口座残高を格納する
char t; //捨てる入力データの一時入力場所
int from[TRANS_COUNT], to[TRANS_COUNT]; //添字の振替操作の振り込み主、振り込み先アカウントの番号
int amount[TRANS_COUNT]; //添字の振替操作の振替金額
pthread_mutex_t mutex; //mutexのアドレスの初期化
pthread_cond_t cvar; //条件変数のアドレスの初期化
struct timeval now; //今の時間を取得する変数の宣言
struct timespec timeout; //timedwaitの時限を設定する変数
int retcode; //timedwaitの返り値を格納する変数。時限に達したかどうかがわかる

/*ただfor文をまわすだけの関数

*/
void busy(int many){
	for(int i=0; i < many * 1000; i++){
	}
}

/*全口座の総額を計算する関数


*/
int sumAmount(){
	int sum = 0;
	for(int i=0; i< ACCOUNT_NUM; i++){
		sum += account[i];
	}
	return sum;
}


/*

*/
void *fun(void *arg){
	int thn = (int)arg;
	
	for (int i=0; i < TRANS_COUNT/10; i++){
		
		pthread_mutex_lock(&mutex);
		gettimeofday(&now, NULL);
		timeout.tv_sec = now.tv_sec + 1;
		timeout.tv_nsec = now.tv_usec * 1000;
		retcode = 0;
		while(account[from[thn * TRANS_COUNT/10 + i]] < amount[thn * TRANS_COUNT/10 + i] && retcode != ETIMEDOUT){
			retcode = pthread_cond_timedwait(&cvar, &mutex, &timeout);
		}
		if(retcode != ETIMEDOUT){
			//printf("%d:%d:%d:%d\n", thn, from[thn * TRANS_COUNT/10 + i],to[thn * TRANS_COUNT/10 + i],amount[thn * TRANS_COUNT/10 + i]);
			busy(amount[thn * TRANS_COUNT/10 + i]);
			account[from[thn * TRANS_COUNT/10 + i]] -= amount[thn * TRANS_COUNT/10 + i];
			account[to[thn * TRANS_COUNT/10 + i]] += amount[thn * TRANS_COUNT/10 + i];
			pthread_cond_signal(&cvar);
		}
		pthread_mutex_unlock(&mutex);
		
		if(account[from[thn * TRANS_COUNT/10 + i]]<0){
			printf("残高の下限を超えてしまっています\n");
		}
	}

}

/*

*/
int main(){
	for(int i=0; i < ACCOUNT_NUM; i++) {
		account[i] = FIRST_MANY;
	}
	
	//mutex,状態変数の初期化
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cvar, NULL);
	
	int val;
	
	//トランザクションデータの入力
	FILE *istream;
	
	if((istream = fopen("trans.csv", "r"))==NULL){
		printf("ファイルを開くことができません");
	}
	
	int v=0;
	while(1){
		val = fscanf(istream, "%c,%d,%d,%d\n", &t, &from[v], &to[v], &amount[v]);
		if(val == EOF) break;
		else if(val != 4){
			printf("読み込むデータの個数に間違いがあります");
		}
		v++;
	}
	val = fclose(istream);
	if(val != 0){
		perror("fclose:");
		exit(1);
	}	
	
	//スレッドの初期化
	pthread_t th[10];
	
	//最初の合計金額の表示
	printf("最初の合計金額：%d円\n", sumAmount());
	
	//スレッドを生成
	for (int i=0; i < 10; i++){
		val = pthread_create(&th[i], NULL, fun, (void *)i);
		if(val != 0){
			perror("pthrad_create:");
			exit(1);
		}	
	}
	
	//スレッドの終了
	for (int i=0; i < 10; i++){
		pthread_join(th[i], NULL);
	}
	
	//最後の合計金額の表示
	printf("最後の合計金額：%d円\n", sumAmount());
	
	int numbersum_c = 0;
	for(int i=0; i < 10000; i++){
		numbersum_c += i;
	}
	
}


