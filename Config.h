#ifndef ConfigH
#define ConfigH

#include <stdio.h>
#include "DForm.h"
#include "Pages.h"

typedef struct isection_s {

        char *name;
        struct ihashitem_s **ihash;
        struct ihashitem_s *last;

} isection;

typedef struct ihashitem_s {

        char *name;
        char *value;
        struct ihashitem_s *next;

} ihashitem;

extern char *SessionName;

extern int AccessMethod;
extern int CommPort;
extern int Baudrate;
extern int MonitorInterval;
extern int ExecTimeout;

extern TFont *EditorFont;
extern TFont *DebuggerFont;
extern int DCharWidth;

extern bool TraceEvents;
extern bool TraceDebug;
extern bool TraceWire;
extern bool TraceComm;

extern char *KeilArmPath;
extern char *AsmArgs;
extern char *CompArgs;
extern char *LinkArgs;

extern char BaseDir[2048];
extern char *LogFile;
extern FILE *Logfd;
extern char *SLogFile;
extern FILE *SLogfd;

extern char *BootScript;

void VD(char *fmt, ...);
void VD_(char *fmt, ...);
void _VD_(char *fmt, ...);
void VW(char *fmt, ...);
void VW_(char *fmt, ...);
void _VW_(char *fmt, ...);
void VC(char *fmt, ...);
void VC_(char *fmt, ...);
void _VC_(char *fmt, ...);

char *makeCaption(char *name, unsigned int target);

void readConfig();
void applyConfig();
void saveConfig();

void newSession();
void readSession();
void readSession(char *name);
void saveSession();
void saveSessionAs();
bool closeSession();

char *shortName(char *name);
char *longName(char *name);
void newSource(asmpage *ap);
bool readSource(char *name, asmpage *ap, bool showmessage);
bool writeSource(char *name, asmpage *ap);
bool saveChanges(asmpage *ap);
void closePage(asmpage *ap);

#endif
