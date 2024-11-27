// cy22226 日暮大地 keydisp.c
/*
このファイルはmain関数を含みtarget-io.hを用いて、target-io.cと結合して成立する。
*/
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


#include "target-io.h"
#define BUTTON_BROWN 22
#define BUTTON_ORANGE 24

int timer_on;//タイマーがオンかどうかを格納する
struct itimerval timval;//タイマーの構造体
int lcd_clear_switch;//表示を消すことを合図する変数

/*
busy
繰り返しの周りすぎを抑えるための関数
*/
void busy(){
	for(int i = 0; i < 1000; i++){
	}
}

//シグナルハンドラ
void sig_handler(int signum){
	int val;
	if(check_button(BUTTON_BROWN) == 1){
		lcd_clear_switch = 1;
	}
	timer_on = 0;
	printf("1秒経過\n");
}

/*
main関数
余力問題１まで行った。
茶ボタンが押されたときHelloと表示して、ボタンを離した一秒後に表示を消す。
一秒しないうちに茶ボタンが再び押されるとタイマーが停止して、Helloを表示する。
また、オレンジボタンを押すと、処理が終わる機能を個人的に追加した。
*/
int main(){
	lcd_clear_switch = 0;
	timer_on = 0;
	timval.it_interval.tv_sec = 0;
	timval.it_interval.tv_usec = 0;
	int flag_on = 0;
	int flag_off = 1;
	int val;
	unsigned int bt;
	unsigned int stop_bt;
	char dat[] = "Hello";
	char space[7];
	
	
	for(int i=0; i < 6; i++){
		space[i] = 0b10100000;
	}
	space[6] = '\0';	
	
	//シグナルハンドラの登録
	if(signal(SIGALRM, sig_handler) == SIG_ERR){
		perror("signal:");
		exit(1);
	}
	
	while(1){
		stop_bt = check_button(BUTTON_ORANGE);
		if(stop_bt == 0){
			break;
		}
		
		bt = check_button(BUTTON_BROWN);
		
		printf("%d\n", bt);
		if(bt == 0){//ボタンが押されている場合
			if(timer_on == 1){//1秒以内にボタンが押し直されていたとき
				timer_on = 0;
				timval.it_value.tv_sec = 0;
				timval.it_value.tv_usec = 0;
				val = setitimer(ITIMER_REAL, &timval, NULL);//er
				if(val == -1){
					perror("setitimer:");
					exit(1);
				}
			}
			printf("おされてる\n");
			if(flag_off == 1){//ボタンが押されたはじめの一周目
				flag_off = 0;
				flag_on = 1;
			//表示オン
				write_LCD(dat, 0);
				printf("表示した\n");
			}
			//printf("%dま\n", thn);
		}else{//ボタンが離されている場合
			printf("おされてない\n");
			if(flag_on == 1){//ボタンが離されて初めの一周目
				flag_on = 0;
				flag_off = 1;
				//タイマーリセット
				timval.it_value.tv_sec = 1;
				timval.it_value.tv_usec = 0;
				setitimer(ITIMER_REAL, &timval, NULL);
				if(val == -1){
					perror("setitimer:");
					exit(1);
				}
				timer_on = 1;
			}
			if(lcd_clear_switch == 1){//一秒経過したとき
				lcd_clear_switch = 0;
				//スペースで埋める
				write_LCD(space, 0);
				printf("表示を消した\n");
			}
		}
		
		busy();
		
	}
	
	//表示を消す
	write_LCD(space, 0);



}

