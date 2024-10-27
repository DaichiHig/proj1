#include <errno.h>
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

void *gpio_baseaddr;

/*
void ledOnOff()
{
    unsigned int gpfsel0;
    gpfsel0= memread(gpio_baseaddr, GPIO_GPFSEL0);	// get GPIO5 mode
    gpfsel0 = gpfsel0 | (1<<15);		// 15=GPIO5*3, bit15 ON
    memwrite(gpio_baseaddr, GPIO_GPFSEL0, gpfsel0);	// GPIO5 output mode
    memwrite(gpio_baseaddr, GPIO_GPSET0, (1 << 5));	// GPIO5 high
    sleep(1);
    memwrite(gpio_baseaddr, GPIO_GPCLR0, (1 << 5));	// GPIO5 low
}
*/

unsigned int mem_read(void *baseaddr, int offset)
{
    unsigned int *p;
    p = baseaddr+offset;
    return *p;	// read memory-mapped register
}


/*
void memwrite(void *baseaddr, int offset, unsigned int x)
{
    unsigned int *p;
    p = baseaddr+offset;
    *p = x;	// write memory-mapped register
}
*/




int lcd_cmdwrite(int fd, unsigned char dat)
{
	unsigned char buff[2];
	buff[0] = 0;
	buff[1] = dat;
	return write(fd,buff,2);
}


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

/*いらんかも
int open_LCD()
{
	int val;
	int i2c;
	i2c=open("/dev/i2c-1", O_RDWR);
	if(i2c == -1){
		perror("open:");
		exit(1);
	}
	return i2c;

}
*/
/*
void ioctl_func(int fd)
{
	val=ioctl(fd, I2C_SLAVE, 0x3e);
	if(val == -1){
		perror("ioctl:");
		exit(1);
	}
}
*/

int location(int fd, int y)
{
	int x = 0;
	int cmd=0x80 + y * 0x40 + x;
	return lcd_cmdwrite(fd, cmd);
}

int clear(int fd)
{
	int val = lcd_cmdwrite(fd, 1);
	usleep(1000);	// wait 1ms
	return val;
}

int write_LCD(char dat[], int y){
	int val;
	int i2c;
	i2c=open("/dev/i2c-1", O_RDWR);
	if(i2c == -1){
		perror("open:");
		return -1;
	}
	val=ioctl(fd, I2C_SLAVE, 0x3e);
	if(val == -1){
		perror("ioctl:");
		return -1;
	}
	val = location(i2c, y);
	if(val == -1){
		perror("location:");
		return -1;
	}
	val = lcd_datawrite(i2c, dat);
	if(val == -1){
		perror("lcd_datawrite:");
		return -1;
	}
	val=close(i2c);
	if(val == -1){
		perror("close:");
		return -1;
	}
	return 0;

}


