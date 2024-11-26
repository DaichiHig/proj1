/* manege_data.h */

void set_sync();
void getmutex();
void give_back_mutex();
void inform_didtx();
void timer_set(int thn, int tout);
char check_tx_type(int i);
int check_task(int i);
int start_check_tx(int i, int thn);
int start_check_receive(char buff[1024], int thn);
void do_receive(int t, int am);
void do_tx(int i);
char *make_send_data(int i);
int sumAmount();
void make_account_data(int first_maney);
void read_txdata(char mode);



void postpone_tx(int nowtask, int endtask);
int get_timedmutex(int tout);
