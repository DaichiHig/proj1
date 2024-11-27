// transaction.c cy22226 higurashi daichi
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/time.h>
#define TRANS_COUNT 10000
#define FIRST_MONEY 10000
#define ACCOUNT_NUM 100
#define TIMEOUT_TIME 1


int account[ACCOUNT_NUM]; //添字番号のアカウントの口座残高を格納する

///振替操作のデータ格納変数の宣言
char t; //捨てる入力データの一時入力場所
int from[TRANS_COUNT], to[TRANS_COUNT]; //添字の振替操作の振り込み主、振り込み先アカウントの番号
int amount[TRANS_COUNT]; //添字の振替操作の振替金額

pthread_mutex_t mutex; //mutexのアドレスの初期化
pthread_cond_t cvar; //条件変数のアドレスの初期化

struct timeval now; //今の時間を取得する変数の宣言
struct timespec timeout; //timedwaitの時限を設定する変数
int retcode; //timedwaitの返り値を格納する変数。時限に達したかどうかがわかる

//テスト用に使用したグローバル変数
int did_trans = 0; //すべての振替操作の添字を操作を行ったら合計していき、正しいか確かめるための変数


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
関数:sumAmount
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


/*
関数:fun
スレッドの処理として利用する。スレッド番号に応じて担当の番号の振替処理を行う。全体の1/10の操作を一つのスレッドが担当する。
引数:arg スレッドの番号を表すアドレス
戻り値: なし
操作: 振替先の配列to,振替元の配列from,振替金額の配列amountを用いて振替の操作を配列accountに行う。
テスト: 
TIMEOUT_TIMEの値を変えて、以下の実験をmain関数を使って行った。
・タイムアウトが機能しているかをprintf表示した。
・残高が0未満になっていることはないか、if文で表示して確認した。
・行った振替操作の添字番号を合計していって最後にすべての操作を行っていたかを確認した。

*/
void *fun(void *arg){
	
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

/*
関数:main
10つのスレッドを作成して、ファイルtrans.csvに記された振替処理を行う
テスト:
・すべての振替操作の添字の合計を確かめることにより、すべての振替を行ったか確認した。

テスト結果１
最初の合計金額：1000000円
wait
14の振替をタイムアウトしました
最後の合計金額：1000000円
correct:49995000
result :49995000

テスト結果２
最初の合計金額：1000000円
最後の合計金額：1000000円
correct:49995000
result :49995000

テスト結果3
最初の合計金額：1000000円
wait
wait
最後の合計金額：1000000円
correct:49995000
result :49995000

テスト結果4
最初の合計金額：1000000円
wait
最後の合計金額：1000000円
correct:49995000
result :49995000
結果はTIMEOUT_TIME=0.001のときは、テスト結果1になることと、テスト結果２になることがあったが、
TIMEOUT_TIME=0.0001のときは、テスト結果1にしかならなかった。
反対にTIMEOUNT_TIME=1に増やすと、テスト結果3,4のようにwaitは複数出るが、タイムアウトをしなくなった。
期待していた動作をするので、このコードは正しいと考える。ただ気になるのが、タイムアウトになるのが14番の振替でしか起こらない。
そこで自分で振替データファイルrandm_data.csvを作って振替の実行時にも表示を追加してテストしたところ以下のように他の振替でも
タイムアウト処理が行えていた。
しかし、そもそも、任意の振替の金額で作成してしまったため、最終的には同じ振替でタイムアウト繰り返すようになり、このデータでは処理は
完了しなかった。現実で行われた実際の振替データではこのようなことは起きないと考えられる。
以上のテストによりコードは正しいと考えられる。

..
do
do
do
do
do
do
wait
9981の振替をタイムアウトしました
do
wait
8982の振替をタイムアウトしました
wait
wait
1916の振替をタイムアウトしました
7968の振替をタイムアウトしました
do
do
do
wait
wait
..
..
wait
2996の振替をタイムアウトしました
wait
9985の振替をタイムアウトしました
wait
7978の振替をタイムアウトしました
wait
1962の振替をタイムアウトしました
wait
6971の振替をタイムアウトしました
wait
8984の振替をタイムアウトしました
wait
9985の振替をタイムアウトしました
wait
2996の振替をタイムアウトしました
wait
7978の振替をタイムアウトしました
wait
1962の振替をタイムアウトしました
wait
^C
*/
int main(){
	for(int i=0; i < ACCOUNT_NUM; i++) {
		account[i] = FIRST_MONEY;
	}
	
	//mutex,状態変数の初期化
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cvar, NULL);
	
	int val;
	
	//トランザクションデータの入力
	FILE *istream;
	
	if((istream = fopen("trans.csv", "r"))==NULL){ //課題用:trans.csv テスト用:random_data.csv
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
	
	/*//テスト用の表示
	//すべての添字の合計を確かめることにより、すべての振替を行ったか確認する
	int correct_did_trans = 0;
	for(int i=0; i < 10000; i++){
		correct_did_trans += i;
	}
	printf("correct:%d\n",correct_did_trans); 
	printf("result :%d\n",        did_trans); //上の表示と一致していれば正しい
	*/
	
}


