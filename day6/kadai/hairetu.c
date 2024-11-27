#include <stdio.h>
char *splitdata[50];//切り分けたそれぞれの文字列のアドレスを配列に格納する
/*

*/
void split_by_and(char *buff){
	char *buffcp;
	char *p; //文字列中の文字検索strchrの結果アドレスを格納する変数
	char *t; //文字列データを切り分けるのに使用する
	int i = 0;
	buffcp = (char *)malloc(sizeof(buff));
	strcpy(buffcp, buff);
	p = buffcp - 1;//初期値設定
	while(1){
	
		t = p + 1;//次のデータ
		p = strchr(t,'&');
		if(p == NULL) {
			splitdata[i] = t;
			break;//区切りがないとき
		}
		*p = '\0';//文字列を区切る
		splitdata[i] = t;//文字列アドレスを配列に格納
		i++;
	}
}


int main(){
	char *i;
	char p[5] = "llll";
	
	printf("i add:%d\n", i);
	printf("i[0] add:%d\n", i); 
	printf("d add:%d\n", p);
	printf("d[0] add:%d\n", p); 


}


