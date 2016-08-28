#ifndef DebugH
#define DebugH

#define M_UNKNOWN 0
#define M_BOOT 1
#define M_RAM 2
#define M_FLASH 3
#define M_IO 4

#define SM_ISOLATED 1
#define SM_STICKY 2
#define SM_RAM 3
#define SM_BREAKPOINT 4
#define SM_UNDO 5
#define SM_INTERNAL 6
#define SM_REINSTALL 7

#define SS_DISCONNECTED 0
#define SS_CONNECTING 1
#define SS_READY 2
#define SS_INTBRK 3
#define SS_EXTBRK 4

#define SF_BREAK 0
#define SF_SNAP 1
#define SF_FAST 2

#define SP_NEW -1

#define RS_NORMAL 0
#define RS_LOCKED 1
#define RS_RELOCATED 2

#define IC_PFVECTOR 0
#define IC_DAVECTOR 1
#define IC_UIVECTOR 2
#define IC_CLEARTLB 3
#define IC_IMB 4
#define IC_COPY 5
#define IC_FILLL2T 6
#define IC_FASTOFF 7
#define IC_READ 8
#define IC_SEARCH 9
#define IC_EXEC 10
#define IC_EXSI 11
#define IC_FLASHBASE 12
#define IC_QRAMBASE 13
#define IC_XRAMBASE 14
#define ICB_FLASHSHIFT (15*4+0)
#define ICB_QRAMSHIFT (15*4+1)
#define ICB_SNAPNO (15*4+2)
#define ICB_XRAMSHIFT (15*4+3)
#define IC_LOGBUFFER 16
#define IC_LOGDATA 17
#define ICB_LOGCURRENT (17*4+0)
#define ICB_INBREAK (17*4+1)
#define IC_LOGLASTREAD 18
#define IC_LOSTCOUNT 19
#define IC_SNAPDATA 20
#define IC_SNAPMASK 21
#define IC_WATCHPAGE 22
#define IC_WATCHSTART 23
#define IC_WATCHEND 24
#define IC_SERVICESP 25
#define IC_SERVICELR 26
#define IC_TEMPEXECPAGE 27
#define IC_HOLDERPOS 28
#define IC_JUMPPOS 29
#define IC_TIMERPORT 30
#define IC_CHKCOND 31

#define IC_END 29

#define MAGIC 0xA65D0105

struct patch {

        unsigned int address;
        int length;
        unsigned char *olddata;
        unsigned char *newdata;
        bool disabled;

};

struct snap {

        int num;
        bool active;

        unsigned int addr;
        int width;              // 4 for arm, 2 or 4 for thumb
        bool thumb;
        char *text;
        char *condition;
        bool isbreak;

        char *compiled;
        unsigned int instruction;
        unsigned int request;

};

struct memoryinfo {

        unsigned int addr;
        unsigned int len;
        int type;

};

struct aname {

        char *name;
        unsigned int address;

};

extern unsigned int FlashAddress, FlashSize, FlashPageCount;
extern unsigned int BootROMaddress, BootROMsize;
extern unsigned int IntRAM1address, IntRAM1size;
extern unsigned int IntRAM2address, IntRAM2size;
extern unsigned int IntRAM3address, IntRAM3size;
extern unsigned int ExtRAMaddress, ExtRAMsize, ExtRAMpageCount;
extern unsigned int IOaddress, IOsize;
extern unsigned int ShadowFlashAddress, ShadowRAMaddress;
extern unsigned int NucleusLow, NucleusHigh;
extern unsigned int UseRAM;
extern bool SimpleBoot;
extern int PoolCount;
extern int PoolFree;
extern unsigned int ConfigRegister;
extern unsigned int TransTableAddr;
extern unsigned int mem_addr[16], mem_len[16], mem_type[16];
extern unsigned int InternalCodeAddr, LoadableCodeAddr, TempExecAddr;
extern unsigned int HeaderAddr, RelBitsAddr, SnapDataAddr, HolderAddr;
extern unsigned int ZeroPage, LogPage;
extern unsigned int WatchpointAddr, WatchpointLen;
extern int SystemState;
extern unsigned int CurrentAddr;
extern bool CurrentThumb;
extern int LastRead;
extern char *NuSleepSignature;

void emptyPatchList();
patch *addPatch(unsigned int addr, unsigned char *olddata, unsigned char *newdata, unsigned int len, int stype);
void deletePatch(unsigned int addr);
patch *getPatchByAddress(unsigned int addr);
int getPatchCount();
patch **getPatchList();
bool undoPatch(unsigned int addr);
bool enablePatch(patch *p);
bool disablePatch(patch *p);

void emptyNamesList(bool is_global);
void addName(char *name, unsigned int addr, bool is_global);
void addNameFast(char *name, unsigned int addr, bool is_global);
void removeName(char *name);
void removeName(unsigned int a);
char *nameByAddress(unsigned int addr);
unsigned int addressByName(char *name);
unsigned int addressForText(char *p);
int getNameCount(bool is_global);
aname **getNameList(bool is_global);

void emptySnapList();
int freeSnap();
bool compileSnap(unsigned int addr, char *text, char *compiled, unsigned int *request);
bool compileCondition(char *text, unsigned int *data);
bool canSetSnap(unsigned int ins, bool is_thumb);
bool addSnap(unsigned int addr, bool is_thumb, char *text, char *condition, int position);
bool deleteSnap(int num);
void deleteSnapAt(unsigned int addr);
snap *snapAtAddress(unsigned int addr);
snap **getSnapList();

void emptyWatchpoint();
bool setWatchpoint(unsigned int as, unsigned int ae);
void removeWatchpoint();

void initChangesArray(int size);
void setChanged(unsigned int addr, unsigned int size);
void clearChanged(unsigned int addr, unsigned int size);
bool isChanged(unsigned int addr);
bool isChanged4(unsigned int addr);

bool splitPage(unsigned int addr, unsigned int l2taddr);
unsigned int acquirePage(bool lock);
bool lockPage(unsigned int addr);
bool lockPageUser(unsigned int addr);
bool unlockPage(unsigned int addr);
bool relocatePage(unsigned int addr);
unsigned int acquireVirtualPage();
void setAddressNonCacheable(unsigned int addr);

void setUsed(unsigned int addr);
bool isUsed(unsigned int addr);

void initialize(int sid);
bool synchronize();
void setSystemState(int state);
void eventMonitor(bool user);
bool ensureRAMisCoarse();
bool readMemory(unsigned int addr, unsigned char *buf, int len);
bool setMemory(unsigned int addr, unsigned char *buf, int len, int stype);
void getMemoryInfo(unsigned int addr, struct memoryinfo *mi);
unsigned int getTempAddress();
bool inSpace(unsigned int addr);

bool execute(unsigned char *code, int len, unsigned int r[16], int count, unsigned int &cpsr, int timeout);
bool execute2(unsigned char *code, int len, unsigned int r[16], int count, unsigned int &cpsr, int timeout);
bool startTrace(unsigned char *code, int len, unsigned int r[16], unsigned int &cpsr);
unsigned int service(unsigned int num, unsigned int r0, unsigned int r1);
bool copyData(unsigned int dest, unsigned int src, unsigned int len);
unsigned int searchMemory(unsigned char *what, unsigned char *mask, int n, unsigned int addr_s, unsigned int addr_e);
unsigned int searchSignature(unsigned char *sig, unsigned char *mask,
                                int len, int offset,
                                unsigned int startAddress, unsigned int endAddress);
unsigned int phone_off();

int getFreePoolEntries();

bool updateRegister(int num, unsigned int value);
bool updateCPSR(unsigned int value);
void stepInto();
void stepOver();
void runFromPoint();
void programReset();

#endif
