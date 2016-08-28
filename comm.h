#ifndef CommH
#define CommH

extern char *EOL;

bool connect(int CommPort, int Baudrate);
void disconnect();
bool is_connected();
unsigned long readcom(unsigned char *buf, int len);
unsigned long writecom(unsigned char *buf, int len);
unsigned long writecomsz(char *buf);
char *exec_at(char *cmd);
bool setSpeed(int n);
bool setTimeout(int n);
void flushBuffers();
void comm_clear();
void comm_flush();

#endif

 