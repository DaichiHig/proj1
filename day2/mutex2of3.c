//第２回課題１C　mutex2of3.c
//cy22226 higurashi daichi 2024/10/10
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#define COUNT1 5//処理A,Bを行う回数；少なくしたのはそのほうが目で数えて確認しやすかったから
#define COUNT2 5 //処理A内のbusy()を回す回数；少なくしたのは、このほうが一回のABの処理が目視できるから
#define LOOP 10000//busy()のなかの繰り返し処理の回数

struct timespec ts[4][COUNT1 * COUNT2];//動いているスレッド番号とそのときの取得時間を記録する２次元配列
pthread_mutex_t mutex[3];//mutexのアドレスの初期化

//ただfor文を回すだけの関数
void busy(){
	for(int i=0; i < LOOP; i++){
	}
}

void *fun(void *arg){
	int thn = (int)arg; //スレッド番号の取得
	int k = 0; //時間取得のの実行回数をカウントする変数
	
	for(int i=0; i < COUNT1; i++){
		
		int mn = 0; //どのmutexにアクセスするかを表す添字
		int allowed = 0; //すでにmutexを一つ取得しているかどうかを0,1で表す
		int getm_index[3] = {0,0,0};//現在どのmutexを取得しているかを0,1で保存する配列
		
		// 3つのmutexにアクセスしていくwhile文.2つmutexを取得するとbreakする
		while (1){
			//1つめのmutex取得のあと、2つめを取得できずにmnの番号が１周した場合は
			//取得済みのmutexを解除する
			if(getm_index[mn] == 1){
				pthread_mutex_unlock(&mutex[mn]);
				getm_index[mn] = 0;
				allowed = 0;
				
				//すぐに同じmutexにアクセスするとdeedlockを起こす危険があるので
				//次のmutexに更新をしておく
				mn++;
				mn = mn % 3;
			}
			
			//mn番目のmutexにアクセス
			if (pthread_mutex_trylock(&mutex[mn]) == 0){
				getm_index[mn] = 1;
				
				//すでに一つのmutexを取得している場合は処理Aに進む、
				//そうでなければ次のmutexにアクセスをする
				if(allowed){
					break;
				}else{
					allowed = 1;
				}
			}
			
			// 次のmutexに更新(0~2)
			mn++;
			mn = mn % 3; //3まで来たら0に戻す
		}
		
		//処理A
		//busy()をして、時刻測定することを繰り返す
		for(int j=0; j<COUNT2; j++){
			busy();
			clock_gettime(CLOCK_REALTIME, &ts[thn][k]);
			k++;
		}
		//ロックの解除
		for (int i=0; i < 3; i++){
			//取得しているmutexのみを解除
			if(getm_index[i] == 1){
				pthread_mutex_unlock(&mutex[i]);
				getm_index[i] = 0;
				allowed = 0;
			}
		}
		//処理B
		busy();
	}
}

int main(){
	//mutexの初期化
	for (int i=0; i < 3; i++){
		pthread_mutex_init(&mutex[i], NULL);
	}
	
	//スレッドのアドレスの初期化
	pthread_t th[4];
	
	
	//スタートの時刻の取得
	struct timespec x;
	clock_gettime(CLOCK_REALTIME, &x);
	long startt = x.tv_sec * 1000000000 + x.tv_nsec;
	
	//スレッドを生成
	for (int i=0; i < 4; i++){
		pthread_create(&th[i], NULL, fun, (void *)i);
	}
	
	
	//スレッドの終了
	for (int i=0; i < 4; i++){
		pthread_join(th[i], NULL);
	}
	
	
	//tsに保存したスレッド番号と取得時間のデータの書き出し
	for (int thn=0; thn<4; thn++){
		for (int i=0; i<COUNT1*COUNT2; i++){
			long t = ts[thn][i].tv_sec *1000000000 + ts[thn][i].tv_nsec;
			printf("%ld\t%d\n", t - startt, thn);
		}
	
	}


}

