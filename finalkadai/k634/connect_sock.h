/* connect_sock.h*/

void send_txdata(char buff[1024]);
char *receive_txdata();
void send_OK_NO(int i);
int wait_OK();
void close_socket(int mode);
void be_server();
void be_client();

