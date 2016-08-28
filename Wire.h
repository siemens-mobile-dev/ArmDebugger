#ifndef WireH
#define WireH

#define METHOD_CGSN 0
#define METHOD_JAVA 1

extern bool is_online();
extern bool ping();
extern void trackConnection();
extern void lostConnection(bool f);
extern bool readFromRAM(unsigned int addr, unsigned char *buf, int len);
extern bool writeToRAM(unsigned int addr, unsigned char *buf, int len);
extern bool queryRAM(unsigned int *addrs, unsigned int *data, int count);
extern bool armCall(unsigned int addr, unsigned int r[16], int count, unsigned int &cpsr, int timeout);

#endif