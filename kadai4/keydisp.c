
#include <pthread.h>
#include <signal.h>
#include <sys/time.h>
#include <errno.h>
#include <stdlib.h>


struct itimerval timval[2];
pthread_mutex_t mutex;


int timer_on[2] = {0,0};
int lcd_clear_swith[2] = {0,0};

void busy(){
	for(int i = 0; i < 1000; i++){
	}
}

//シグナルハンドラ
void sig_handler(int signum, int thn){
	int val;
	if(timval[1].it_value.tv_sec == 0 && timval[1].it_value.tv_usec == 0 && timer_on[1] == 1){
		lcd_clear_switch[1] = 1;
	}
	else if(timval[0].it_value.tv_sec == 0 && timval[1].it_value.tv_usec == 0 && timer_on[0] == 1){
		lcd_clear_switch[0] = 1;
	}

}



//スレッド
void *fun(void *arg){
	
	int thn = (int)arg;
	timval[thn].it_interval.tv_sec = 0;
	timval[thn].it_interval.tv_usec = 0;
	int flag = 0;
	int val;
	
	//シグナルハンドラの立ち上げ
	if(signal(SIGALRM, sig_handler) == SIG_ERR){
		perror("signal:");
		exit(1);
	}
	

	while(1){
		if(){//ボタンが押されている場合
			if(timer_on[thn] == 1){//ボタンが押し直されていたとき
				timer_on[thn] = 0;
				timval[thn].it_value.tv_sec = 0;
				timval[thn].it_value.tv_usec = 0;
				setitimer(ITIMER_REAL, &timval[thn], NULL);//er
			}
			
			flag = 1;
			
			//位置設定
			//表示オン
			val = location(i2c, thn);
			if(val == -1){
				perror("location:");
				exit(1);
			}
			
			
		}else{
			if(flag == 1){//ボタンが離されて初めてのとき
				flag = 0;
				//タイマーリセット
				timval[thn].it_value.tv_sec = 1;
				timval[thn].it_value.tv_usec = 0;
				setitimer(ITIMER_REAL, &timval[thn], NULL);//er
				timer_on[thn] = 1;
			}
			
			if(lcd_clear_switch[thn] == 1)
				lcd_clear_switch = 0;
				//スペースで埋める
			}
		}
		
		busy();
	
	}


}


int main(){
	int val;
	pthread_mutex_init(&mutex, NULL);
	pthread_t th[2];
	
	for (int i=0; i < 2; i++){
		val = pthread_create(&th[i], NULL, fun, (void *)i);
		if(val != 0){
			perror("pthrad_create:");
			exit(1);
		}	
	}


	//スレッドの終了
	for (int i=0; i < 2; i++){
		pthread_join(th[i], NULL);
	}




}





