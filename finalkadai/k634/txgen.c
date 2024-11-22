#include <time.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    int num_txn, num_account, max_amount, i;//トランザクション数、アカウント数、最大金額
    if (argc!=4) {//引数エラー
            printf("num_txn = 1000: num_account = 10000: max_amount = 10000\n");
            num_txn = 1000;
            num_account = 10000;
            max_amount = 10000;
    }else{
    		num_txn = atoi(argv[1]);//引数代入
    		num_account = atoi(argv[2]);
    		max_amount = atoi(argv[3]);
     }
    srand(time(NULL));//乱数の準備

//    printf("num_txn=%d, num_account=%d, max_amount=%d\n", num_txn, num_account, max_amount);
    for (i = 0; i<num_txn; i++) {
            int toVal, amount;
            int fromVal = rand() % num_account;
            int txmode = rand() % 2;
            
            while (1) {
                    toVal = rand() % num_account;
                    if (fromVal != toVal) break;
            }
            amount = (rand() % max_amount) + 1;
            if (txmode) printf("t");
            else printf("o");
            printf(",%d,%d,%d\n", fromVal, toVal, amount);
    }
    return 0;
}
