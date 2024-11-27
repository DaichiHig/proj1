// cy22226 日暮大地　target-io.c
/*
このファイルはraspberry pi のデバイスに関する関数を定義する。
*/
#include <errno.h>
#include <pthread.h>
#include <linux/i2c-dev.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <termios.h>

#include "target-io.h"



// peripheral register physical address
#define GPIO_PHY_BASEADDR  0x3F200000
#define GPIO_AREA_SIZE	4096	// PAGE_SIZE
#define GPIO_GPFSEL0	0x0000	// for gpio 0..9, MSB 2bits are reserved
// omit GPFSEL1..GPFSEL5
#define GPIO_GPSET0	0x001C	// gpio 0..31
#define GPIO_GPSET1	0x0020	// gpio 32..53
#define GPIO_GPCLR0	0x0028	// gpio 0..31
#define GPIO_GPCLR1	0x002C	// gpio 32..53
#define GPIO_GPLEV0	0x0034	// gpio 0..31
#define GPIO_GPLEV1	0x0038	// gpio 32..53


void ledOnOff();
unsigned int memread(void *baseaddr, int offset);
void memwrite(void *baseaddr, int offset, unsigned int x);

void *gpio_baseaddr;//制御レジスタの基準アドレスを格納するのに使用


/*mainで主に使用する関数はこの２つ
check_button:引数にGPIOのピン番号を入れると、オフセットGPLEV0のアドレスから、指定したピンだけの値を返す関数
write_LCD:引数に表示したい文字列と、表示したい列（０〜１）を入力すると、LCDに表示してくれる関数
*/
unsigned int check_button(int offset);
void write_LCD(char dat[], int y);


///////////////////////以下はGPIOの読み取り用関数//////////////////////
/*
mem_read
基準baseaddrからoffsetだけ先のアドレスに格納されている値を返す関数
*/
unsigned int mem_read(void *baseaddr, int offset)
{
    	unsigned int *p;
    	p = baseaddr+offset;
    	return *p;	// read memory-mapped register
}





//自作関数
/*
関数:check_button
引数:int button_num　読み取りたいGPIOのピン番号
戻り値:unsigned int 指定したGPIOピンの値
操作:メモリマップドIOを使う。gpio制御レジスタの基準アドレスと、gpio 0..31の場所の
オフセットを用いて、関数memでgpio 0..31の値を入手して、そこからビット演算でbutton_num
番目の値だけ抽出して返す。
*/
unsigned int check_button(int button_num){
	unsigned int x;
	int offset = GPIO_GPLEV0;
	int fd;
	int val;
	//pthread_mutex_lock(&mutex[0]); //mutexのロック iいらんかも
	
	fd = open("/dev/gpiomem", O_RDWR);
	if(fd == -1){
		perror("open:");
		exit(1);
	}

	gpio_baseaddr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIO_PHY_BASEADDR);
	x = mem_read(gpio_baseaddr, offset);
	val = close(fd);
	if(fd == -1){
		perror("close:");
		exit(1);
	}

	//pthread_mutex_unlock(&mutex[0]); 
	return (x >> button_num) & 0b1;
}

///////////////////////LCDの以下は書き込み用関数//////////////////////
/*
lcd_cmdwrite
書き込みに必要なコマンドの作成を行う関数
*/
int lcd_cmdwrite(int fd, unsigned char dat)
{
	unsigned char buff[2];
	buff[0] = 0;
	buff[1] = dat;
	return write(fd,buff,2);
}

/*
lcd_datawrite
LCDにデータを書き込む関数
*/
int lcd_datawrite(int fd, char dat[])
{
	int len;
	char buff[100];

	len = strlen(dat);  // don't count EOS (Null char)
	if (len>99) {printf("too long string\n"); exit(1); }
	memcpy(buff+1, dat, len);	// shift 1 byte, ignore EOS
	buff[0] = 0x40;	// DATA Write command
	return write(fd, buff, len+1);
}


/*
initLCD
LCDの初期化を行う関数
*/
void initLCD(int fd)
{
	int i;
	unsigned char init1[]={ 0x38, 0x39, 0x14, 0x70, 0x56, 0x6c };
	unsigned char init2[]={ 0x38, 0x0c, 0x01 };

	usleep(100000);	// wait 100ms
	for (i=0;i<sizeof(init1)/sizeof(unsigned char);i++) {
		if(lcd_cmdwrite(fd, init1[i])!=2){
			printf("internal error1\n");
			exit(1);
		}
		usleep(50); // wait 50us
	}

	usleep(300000);	// wait 300ms

	for (i=0;i<sizeof(init2)/sizeof(unsigned char);i++) {
		if(lcd_cmdwrite(fd, init2[i])!=2){
			printf("internal error2\n");
			exit(1);
		}
		usleep(50);
	}
	usleep(2000);	// wait 2ms
}


/*
location
fdにi2cの番号,yで表示したい行を指定する関数。
*/
int location(int fd, int y)
{
	int x = 0;
	int cmd=0x80 + y * 0x40 + x;
	return lcd_cmdwrite(fd, cmd);
}


/*
write_LCD:文字列をLCDに表示させる関数
引数１:char dat[]  表示させたい文字列のデータ
引数２:int y　文字列を表示させたい列の指定　0のときは上の行、1のときは下の行になる。
戻り値:なし
操作: LCDデバイスの初期設定から表示まで行い、デバイスをクローズする。
中では関数location、lcd_datawriteを使用している。
*/
void write_LCD(char dat[], int y){
	int val;
	int i2c;
	//デバイスのオープン
	i2c=open("/dev/i2c-1", O_RDWR);
	if(i2c == -1){
		perror("open:");
		exit(1);
	}
	//i2c初期設定
	val=ioctl(i2c, I2C_SLAVE, 0x3e);
	if(val == -1){
		perror("ioctl:");
		exit(1);
	}
	initLCD(i2c);
	//表示列の設定
	val = location(i2c, y);
	if(val == -1){
		perror("location:");
		exit(1);
	}
	//データの書き込み
	val = lcd_datawrite(i2c, dat);
	if(val == -1){
		perror("lcd_datawrite:");
		exit(1);
	}
	//デバイスのクローズ
	val=close(i2c);
	if(val == -1){
		perror("close:");
		exit(1);
	}

}


