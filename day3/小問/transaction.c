
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#define TRANS_COUNT 10000
#define FIRST_MANY 10000
#define ACCOUNT_NUM 100

int account[ACCOUNT_NUM];
char t;
int from[TRANS_COUNT], to[TRANS_COUNT];
int amount[TRANS_COUNT];
pthread_mutex_t mutex;
pthread_cond_t cvar;


void busy(int many){
	for(int i=0; i < many * 1000; i++){
	}
}


int sumAmount(){
	int sum = 0;
	for(int i=0; i< ACCOUNT_NUM; i++){
		sum += account[i];
	}
	return sum;
}



void *fun(void *arg){
	int thn = (int)arg;
	
	for (int i=0; i < TRANS_COUNT/10; i++){
		pthread_mutex_lock(&mutex);
		printf("%d:%d:%d:%d\n", thn, from[thn * TRANS_COUNT/10 + i],to[thn * TRANS_COUNT/10 + i],amount[thn * TRANS_COUNT/10 + i]);
		busy(amount[thn * TRANS_COUNT/10 + i]);
		account[from[thn * TRANS_COUNT/10 + i]] -= amount[thn * TRANS_COUNT/10 + i];
		account[to[thn * TRANS_COUNT/10 + i]] += amount[thn * TRANS_COUNT/10 + i];
		printf("%d\n", sumAmount());
		pthread_mutex_unlock(&mutex);
	}

}

int main(){
	for(int i=0; i < ACCOUNT_NUM; i++) {
		account[i] = FIRST_MANY;
	}
	
	//mutexの初期化
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cvar, NULL);
	
	//トランザクションデータの入力
	FILE *istream;
	
	if((istream = fopen("trans.csv", "r"))==NULL){
		printf("ファイルを開くことができません");
	}
	int val;
	int v=0;
	while(1){
		val = fscanf(istream, "%c,%d,%d,%d\n", &t, &from[v], &to[v], &amount[v]);
		//printf("%d:%d:%d\n", from[v], to[v], amount[v]);
		if(val == 0) break;
		else if(val < 4){
			printf("取り込めなかったデータがあります\n");
			break;
		} 
		v++;
	}
	fclose(istream);
	
	//スレッドの初期化
	pthread_t th[10];
	
	//最初の合計金額の表示
	printf("最初の合計金額：%d円\n", sumAmount());
	printf("z");
	//スレッドを生成
	for (int i=0; i < 10; i++){
		val = pthread_create(&th[i], NULL, fun, (void *)i);
		if(val != 0){
			perror("pthrad_create:");
			exit(1);
		}	
	}
	printf("a");
	//スレッドの終了
	for (int i=0; i < 10; i++){
		pthread_join(th[i], NULL);
	}
	printf("b");
	//最後の合計金額の表示
	printf("最後の合計金額：%d円\n", sumAmount());
	
}


