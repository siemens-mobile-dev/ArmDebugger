#ifndef UtilityH
#define UtilityH

extern bool ExtraFeatures;

extern char *digits;
extern char *regnames[16];

void disableFullScreen();
bool InfoMessage(char *fmt, ...);
bool WarningMessage(char *fmt, ...);
bool ErrorMessage(char *fmt, ...);
bool YesNoQuestion(char *fmt, ...);

void setupMainUI(TColor indy_color, char *indy_cap, char *s_text, bool e_con, bool e_onl);

int shiftState();
int ctrlState();

void m_print(char *msg, ...);
void m_append(char *msg);

void updateFreePoolEntries(int n);
void calibrateDelayLoop();
void uSleep(int us);

unsigned int readhex(char *p);
int scan16(char *s, unsigned char *buf, int len, bool skipspaces);
int scan16m(char *s, unsigned char *buf, unsigned char *mask, int len);
char *toHexString(unsigned char *data, int len);
int ustrncmp(unsigned char *s1, unsigned char *s2, int len);

void getClipboard(char *buf, int size);
bool setClipboard(char *text);

#endif
