// 
#include <pthread.h>
#include <signal.h>
#include <sys/time.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
//#include <sys/ioctl.h>

#include "target-io.h"



struct itimerval timval[2];
pthread_mutex_t mutex[2];


int timer_on[2] = {0,0};
int lcd_clear_switch[2] = {0,0};

void busy(){
	for(int i = 0; i < 1000; i++){
	}
}

//シグナルハンドラ
void sig_handler(int signum){
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
	unsigned int bt;
	char dat[20];
	char space[7];
	//printf("%dあ\n", thn);
	
	for(int i=0; i < 6; i++){
		space[i] = 0b10100000;
	}
	space[6] = '\0';
	
	//printf("%dか\n", thn);
	
	if(thn == 0){
		strcpy(dat, "Hello");
	}else{
		strcpy(dat, "world");
	}
	
	//printf("%dさ\n", thn);
	//シグナルハンドラの立ち上げ
	if(signal(SIGALRM, sig_handler) == SIG_ERR){
		perror("signal:");
		exit(1);
	}
	//printf("%dた\n", thn);

	while(1){
		pthread_mutex_lock(&mutex[0]); 
		bt = check_button(thn+22);
		pthread_mutex_unlock(&mutex[0]); 
		//printf("%dな\n", thn);
		printf("%d%d\n", bt);
		if(bt == 0){//ボタンが押されている場合
			if(timer_on[thn] == 1){//ボタンが押し直されていたとき
				timer_on[thn] = 0;
				timval[thn].it_value.tv_sec = 0;
				timval[thn].it_value.tv_usec = 0;
				setitimer(ITIMER_REAL, &timval[thn], NULL);//er
			}
			printf("%dおされてる\n", thn);
			flag = 1;
			//printf("%dは\n", thn);
			//位置設定
			//表示オン
			pthread_mutex_lock(&mutex[1]); 
			write_LCD(dat, thn);
			pthread_mutex_unlock(&mutex[1]);  	
			//printf("%dま\n", thn);
		}else{//ボタンが離されている場合
			printf("%d離されてる\n", thn);
			if(flag == 1){//ボタンが離されて初めてのとき
				flag = 0;
				//タイマーリセット
				timval[thn].it_value.tv_sec = 1;
				timval[thn].it_value.tv_usec = 0;
				setitimer(ITIMER_REAL, &timval[thn], NULL);//er
				timer_on[thn] = 1;
			}
			//printf("%dや\n", thn);
			if(lcd_clear_switch[thn] == 1){
				lcd_clear_switch[thn] = 0;
				//スペースで埋める
				pthread_mutex_lock(&mutex[1]); 
				write_LCD(space, thn);
				pthread_mutex_unlock(&mutex[1]); 
			}
		}
		//pthread_mutex_unlock(&mutex); 
		//busy();
		sleep(0.5);
		printf("%dん\n", thn);
	}
	

}


int main(){
	int val;
	int fd;
	int i2c;
	pthread_mutex_init(&mutex[0], NULL);
	pthread_mutex_init(&mutex[1], NULL);
	pthread_t th[2];
	/*
	fd = open("/dev/gpiomem", O_RDWR);
	if(fd == -1){
		perror("open:");
		exit(1);
	}
	i2c=open("/dev/i2c-1", O_RDWR);
	if(i2c == -1){
		perror("open:");
		exit(1);
	}
	val=ioctl(i2c, I2C_SLAVE, 0x3e);
	*/
	
	for (int i=0; i < 2; i++){
		val = pthread_create(&th[i], NULL, fun, (void *)i);
		if(val != 0){
			perror("pthread_create:");
			exit(1);
		}	
	}


	//スレッドの終了
	for (int i=0; i < 2; i++){
		pthread_join(th[i], NULL);
	}	
	/*
	val=close(i2c);
	if(val == -1){
		perror("close:");
		exit(1);
	}
	
	val = close(fd);
	if(fd == -1){
		perror("close:");
		exit(1);
	}
	*/




}





