#include <vcl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Config.h"
#include "EditAsc.h"
#include "Debug.h"
#include "BootWindow.h"
#include "Pages.h"
#include "Bookmarks.h"
#include "GotoWindow.h"
#include "PatchesWindow.h"
#include "Snappoints.h"
#include "IniFile.h"
#include "Utility.h"
#include "Globals.h"
//#include "DForm.h"

int AccessMethod;
int CommPort;
int Baudrate;
int MonitorInterval;
int ExecTimeout;

TFont *EditorFont;
TFont *DebuggerFont;
int DCharWidth;

bool TraceEvents;
bool TraceDebug;
bool TraceWire;
bool TraceComm;

char *KeilArmPath;
char *AsmArgs;
char *CompArgs;
char *LinkArgs;


char BaseDir[2048];
char *SessionName;
int SessionID;
char *LogFile;
FILE *Logfd = NULL;
char *SLogFile;
FILE *SLogfd = NULL;

static char *spaces = "                      ";

void VX(char c, bool cr, char *fmt, va_list ap) {

        static char buf[4096];
        static int tab = 0;
        int l;

        vsnprintf(buf, 4096, fmt, ap);

        l = strlen(buf);
        if (buf[0] == '}') tab--;
        if (tab < 0) tab = 0;
        if (SLogfd != NULL) {
                if (c != 0) {
                        fputc(c, SLogfd);
                        fputs(spaces+20-tab, SLogfd);
                }
                fputs(buf, SLogfd);
                if (cr) fputc('\n', SLogfd);
        }
        if (l > 0 && (buf[l-1] == '{' || buf[l-2] == '{')) tab++;
        if (tab > 20) tab = 20;

}

void VD(char *fmt, ...) {
        va_list ap;
        if (! TraceDebug) return;
        va_start(ap, fmt);
        VX('D', true, fmt, ap);
        va_end(ap);
}

void VD_(char *fmt, ...) {
        va_list ap;
        if (! TraceDebug) return;
        va_start(ap, fmt);
        VX('D', false, fmt, ap);
        va_end(ap);
}

void _VD_(char *fmt, ...) {
        va_list ap;
        if (! TraceDebug) return;
        va_start(ap, fmt);
        VX(0, false, fmt, ap);
        va_end(ap);
}

void VW(char *fmt, ...) {
        va_list ap;
        if (! TraceWire) return;
        va_start(ap, fmt);
        VX('W', true, fmt, ap);
        va_end(ap);
}

void VW_(char *fmt, ...) {
        va_list ap;
        if (! TraceWire) return;
        va_start(ap, fmt);
        VX('W', false, fmt, ap);
        va_end(ap);
}

void _VW_(char *fmt, ...) {
        va_list ap;
        if (! TraceWire) return;
        va_start(ap, fmt);
        VX(0, false, fmt, ap);
        va_end(ap);
}

void VC(char *fmt, ...) {
        va_list ap;
        if (! TraceComm) return;
        va_start(ap, fmt);
        VX('C', true, fmt, ap);
        va_end(ap);
}

void VC_(char *fmt, ...) {
        va_list ap;
        if (! TraceComm) return;
        va_start(ap, fmt);
        VX('C', false, fmt, ap);
        va_end(ap);
}

void _VC_(char *fmt, ...) {
        va_list ap;
        if (! TraceComm) return;
        va_start(ap, fmt);
        VX(0, false, fmt, ap);
        va_end(ap);
}

char *makeCaption(char *name, unsigned int target) {

        char buf[256], *p;

        p = strrchr(name, '\\');
        if (p == NULL) p = name; else p++;
        strcpy(buf, p);
        p = strrchr(buf, '.');
        if (p == NULL) p = buf + strlen(buf);
        *p = '\0';

        if (target != NONE) sprintf(p, " :%08X", target);
        return strdup(buf);

}

TFont *parseFont(char *s) {

        TFont *f;
        char buf[256], *p, *pp;
        int size = 9;
        int color = 0;

        strcpy(buf, s);
        p = strchr(buf, ',');
        if (p != NULL) {
                *(p++) = '\0';
                pp = strchr(p, ',');
                if (pp != NULL) {
                        *(pp++) = '\0';
                        color = strtoul(pp, NULL, 16);
                }
                size = atoi(p);
        }

        f = new TFont();
        f->Name = buf;
        f->Size = size;
        f->Color = (TColor)color;

        return f;
}

void readConfig() {

        int i, a, l, wr;
        int x, y, w, h;
        char buf[2048], *p;

        GetCurrentDirectory(sizeof(BaseDir), BaseDir);

        strcpy(buf, BaseDir);
        strcat(buf, "\\ArmDebugger.log");
        SLogFile = strdup(buf);
        SLogfd = fopen(SLogFile, "w");

        strcpy(buf, BaseDir);
        strcat(buf, "\\config.ini");
        openIniFile(buf);

        SessionName = strdup(getString("Main", "Session", "default"));

        AccessMethod = getInt("Main", "AccessMethod", 0);
        CommPort = getInt("Main", "Port", 1);
        Baudrate = getInt("Main", "Baudrate", 115200);
        MonitorInterval = getInt("Main", "MonitorInterval", 1000);
        ExecTimeout = getInt("Main", "ExecTimeout", 2000);

        p = getString("Appearance", "EditorFont", "Courier New,10,800000");
        EditorFont = parseFont(p);
        p = getString("Appearance", "DebuggerFont", "Courier New,8,0");
        DebuggerFont = parseFont(p);

        KeilArmPath = strdup(getString("Environment", "Keil", "C:\\Keil\\ARM"));
        AsmArgs = strdup(getString("Environment", "AsmArgs", "DEBUG ERRORPRINT"));
        CompArgs = strdup(getString("Environment", "CompArgs", "DEBUG ARM"));
        LinkArgs = strdup(getString("Environment", "LinkArgs", "NOPRINT"));

        x = getInt("Appearance", "Left", 0);
        y = getInt("Appearance", "Top", 10);
        w = getInt("Appearance", "Width", 800);
        h = getInt("Appearance", "Height", 564);
        MainForm->Left = x;
        MainForm->Top = y;
        MainForm->Width = w;
        MainForm->Height = h;
        w = getInt("Appearance", "EditorWidth", 410);
        h = getInt("Appearance", "EditorHeight", 320);
        MainForm->EditorPanel->Width = w;
        MainForm->Panel1->Height = h;
        h = getInt("Appearance", "DumpHeight", 180);
        MainForm->Dump1->Height = h;

        TraceEvents = getInt("Trace", "Events", 1);
        TraceDebug = getInt("Trace", "Debug", 0);
        TraceWire = getInt("Trace", "Wire", 0);
        TraceComm = getInt("Trace", "Comm", 0);

        closeIniFile();
}

void applyConfig() {

        int i, n, w, h;
        HWND hw;
        HDC dc;

        h = DebuggerFont->Height;
        if (h < 0) h = -h;

        for (i=0; i<9; i++) {
                AP[i].page->Font = EditorFont;
        }

        MainForm->Dump1->Font = DebuggerFont;
        MainForm->Dump1->DefaultRowHeight = h + 3;
        MainForm->Dump2->Font = DebuggerFont;
        MainForm->Dump2->DefaultRowHeight = h + 3;
        MainForm->vCPU->Font = DebuggerFont;
        MainForm->vCPU->DefaultRowHeight = h + 4;

        hw = MainForm->Dump1->Handle;
        dc = GetDC(hw);
        SelectObject(dc, DebuggerFont->Handle);
        GetCharWidth32(dc, '0', '0', &w);
        DCharWidth = w;

        n = w*8+w;
        MainForm->Dump1->ColWidths[0] = n;
        MainForm->Dump2->ColWidths[0] = n;
        n = w*2+w/2;
        for (int i=1; i<=16; i++) {
                MainForm->Dump1->ColWidths[i] = n;
                MainForm->Dump2->ColWidths[i] = n;
        }
        n += (w*5)/7;
        MainForm->Dump1->ColWidths[8] = n;
        MainForm->Dump2->ColWidths[8] = n;
        MainForm->Dump1->ColWidths[16] = n;
        MainForm->Dump2->ColWidths[16] = n;
        n = w;
        for (int i=17; i<=32; i++) {
                MainForm->Dump1->ColWidths[i] = n;
                MainForm->Dump2->ColWidths[i] = n;
        }

        MainForm->Registers->Width = w*27 + 20;
        MainForm->Registers->ColWidths[0] = w*4 + w*8 + 4;
        MainForm->Registers->ColWidths[1] = w*4 + w*8 + 4;
        MainForm->Registers->ColWidths[2] = w*3 + 4;
        MainForm->Registers->DefaultRowHeight = h;
        MainForm->Panel6->Height = h * 13 + 5;
//        MainForm->Registers->RowCount = 13;

        MainForm->WatchesList->ColWidths[0] = w*11;
        MainForm->WatchesList->ColWidths[1] = 1024;
        MainForm->WatchesList->DefaultRowHeight = h + 2;

        MainForm->MonitorTimer->Interval = MonitorInterval;

        MainForm->Invalidate();
        MainForm->Repaint();

}

void saveConfig() {

        int i;
        char buf[2048];

        strcpy(buf, BaseDir);
        strcat(buf, "\\config.ini");
        openIniFile(buf);

        setString("Main", "Session", SessionName);

        setInt("Main", "AccessMethod", AccessMethod);
        setInt("Main", "Port", CommPort);
        setInt("Main", "Baudrate", Baudrate);
        setInt("Main", "MonitorInterval", MonitorInterval);
        setInt("Main", "ExecTimeout", ExecTimeout);


        sprintf(buf, "%s,%i,%06x", EditorFont->Name.c_str(), EditorFont->Size, EditorFont->Color);
        setString("Appearance", "EditorFont", buf);
        sprintf(buf, "%s,%i,%06x", DebuggerFont->Name.c_str(), DebuggerFont->Size, DebuggerFont->Color);
        setString("Appearance", "DebuggerFont", buf);

        setString("Environment", "Keil", KeilArmPath);
        setString("Environment", "AsmArgs", AsmArgs);
        setString("Environment", "CompArgs", CompArgs);
        setString("Environment", "LinkArgs", LinkArgs);

        setInt("Appearance", "Left", MainForm->Left);
        setInt("Appearance", "Top", MainForm->Top);
        setInt("Appearance", "Width", MainForm->Width);
        setInt("Appearance", "Height", MainForm->Height);
        setInt("Appearance", "EditorWidth", MainForm->EditorPanel->Width);
        setInt("Appearance", "EditorHeight", MainForm->Pages->Height);
        setInt("Appearance", "DumpHeight", MainForm->Dump1->Height);

        setInt("Trace", "Events", TraceEvents);
        setInt("Trace", "Debug", TraceDebug);
        setInt("Trace", "Wire", TraceWire);
        setInt("Trace", "Comm", TraceComm);

        closeIniFile();
}

void getSessionPath(char *name, char *buf) {

        int l;

        if (strchr(name, '\\') == NULL) {
                strcpy(buf, BaseDir);
                strcat(buf, "\\");
                strcat(buf, name);
        } else {
                strcpy(buf, name);
        }
        l = strlen(buf);
        if (stricmp(buf+l-4, ".ads") != 0) strcat(buf, ".ads");
}

char *shortName(char *name) {

        char tbuf[2048], *p;
        int l;

        getSessionPath(SessionName, tbuf);
        p = strrchr(tbuf, '\\');
        l = p-tbuf+1;
        if (*p != NULL && strnicmp(name, tbuf, l) == 0) {
                return strdup(name+l);
        } else {
                return strdup(name);
        }
}

char *longName(char *name) {

        char tbuf[2048], *p;

        if (name[1] == ':') {
                return strdup(name);
        } else {
                getSessionPath(SessionName, tbuf);
                p = strrchr(tbuf, '\\');
                if (*p != NULL) {
                        strcpy(p+1, name);
                        return strdup(tbuf);
                } else {
                        return strdup(name);
                }
        }
}


void jumpToFile(char *file) {

        char buf[2048], *p;

        strcpy(buf, file);
        p = strrchr(buf, '\\');
        if (p == NULL) return;
        *p = 0;
        SetCurrentDirectory(buf);

}

void updateHeader() {

        char tbuf[256], *s;

        s = strrchr(SessionName, '\\');
        if (s == NULL) s = SessionName; else s++;
        sprintf(tbuf, " ArmDebugger - %s", s);
        s = strrchr(tbuf, '.'); if (s != NULL) *s = '\0';
        MainForm->Caption = tbuf;
}

void readSession() {

        readSession(SessionName);

}

void readSession(char *name) {

        unsigned int a, l1, l2, t;
        int i, w, h, n;
        char buf[4096], tbuf[256];
        unsigned int ibuf[16];
        unsigned char ar1[16384], ar2[16384];
        char *s, *s1, *s2, *p, *pp, *ppp;
        int th, type, sid, en;
        unsigned int cond, cval, ins, width, req;
        patch *pt;
        snap **sn;

        SessionName = strdup(name);

        for (int i=0; i<9; i++) {
                AP[i].inuse = false;
                AP[i].binfile = NULL;
                AP[i].target = NONE;
                AP[i].sheet->TabVisible = false;
        }

        getSessionPath(name, buf);
        jumpToFile(buf);
        openIniFile(buf);

        sid = getInt("Main", "ID", 0);
        randomize();
        while (sid == 0) {
                sid = rand();
        }

        for (i=0; i<5; i++) {
                sprintf(tbuf, "C%i", i);
                s =getStringEscaped("BootCommands", tbuf, "");
                if (BootCmd[i]) free(BootCmd[i]);
                BootCmd[i] = strdup(s);
        }

        for (i=0; i<9; i++) {
                sprintf(tbuf, "Source%i", i);
                s = getString("Editor", tbuf, (i==0) ? "Program1.asm" : "");
                sprintf(tbuf, "Target%i", i);
                t = getInt("Editor", tbuf, NONE);
                AP[i].filename = longName(s);
                AP[i].inuse = (strlen(s) > 0) ? true : false;
                AP[i].target = (strlen(s) > 0) ? t : NONE;
                if (AP[i].inuse) {
                        AP[i].sheet->TabVisible = true;
                        readSource(s, &AP[i], 0);
                } else {
                        AP[i].sheet->TabVisible = false;
                }
        }

        RP[0].InitAddress = getInt("Appearance", "RP1address", NONE);
        RP[1].InitAddress = getInt("Appearance", "RP2address", NONE);
        CP.InitAddress = getInt("Appearance", "CPaddress", NONE);

        SessionID = sid;
        initialize(sid);

        FlashAddress = getInt("Main", "FlashAddress", 0);
        FlashSize = getInt("Main", "FlashSize", 0);
        if (FlashSize == 0) {
                initChangesArray(0);
        } else {
                initChangesArray(FlashSize);
        }
        NucleusLow = getInt("Main", "NucleusLow", 0);
        NucleusHigh = getInt("Main", "NucleusHigh", 0);

        n = getInt("Patches", "Count", 0);
        for (i=0; i<n; i++) {
                sprintf(tbuf, "Address%i", i);
                a = getInt("Patches", tbuf, NONE);
                if (a == 0xffffffff) continue;
                sprintf(tbuf, "OldData%i", i);
                s1 = getString("Patches", tbuf, "");
                if (s1[0] == 0) continue;
                l1 = scan16(s1, ar1, sizeof(ar1), true);
                sprintf(tbuf, "NewData%i", i);
                s2 = getString("Patches", tbuf, "");
                if (s2[0] == 0) continue;
                l2 = scan16(s2, ar2, sizeof(ar2), true);
                if (l1 != l2) continue;
                sprintf(tbuf, "Enabled%i", i);
                en = getInt("Patches", tbuf, 1);
                pt = addPatch(a, ar1, ar2, l1, SM_ISOLATED);
                if (en == 0) pt->disabled = true;
        }

        if (PatchesForm != NULL) PatchesForm->reloadList();

        sn = getSnapList();
        for (i=1; i<15; i++) {
                sprintf(tbuf, "Address%i", i);
                a = getInt("Snappoints", tbuf, NONE);
                if (a == NONE) continue;
                sprintf(tbuf, "Thumb%i", i);
                th = getInt("Snappoints", tbuf, 0);
                sprintf(tbuf, "Text%i", i);
                s1 = strdup(getStringEscaped("Snappoints", tbuf, ""));
                sprintf(tbuf, "Condition%i", i);
                s2 = strdup(getStringEscaped("Snappoints", tbuf, ""));
                sprintf(tbuf, "Instruction%i", i);
                ins = getInt("Snappoints", tbuf, 0);
                sprintf(tbuf, "Width%i", i);
                width = getInt("Snappoints", tbuf, 4);

                if (! compileSnap(a, s1, buf, &req)) continue;

                if (! compileCondition(s2, ibuf)) continue;

                if (sn[i] != NULL) {
                        free(sn[i]->text);
                        free(sn[i]->compiled);
                        free(sn[i]);
                }
                sn[i] = (snap *)malloc(sizeof(snap));
                sn[i]->active = true;
                sn[i]->num = i;
                sn[i]->addr = a;
                sn[i]->width = width;
                sn[i]->thumb = th;
                sn[i]->text = s1;
                sn[i]->compiled = strdup(buf);
                sn[i]->condition = s2;
                sn[i]->instruction = ins;
                sn[i]->request = req;
                sn[i]->isbreak = (sn[i]->text[0] == '~');

        }

        if (SnapsForm != NULL) SnapsForm->updateSnaps();

        n = getInt("Sidebar", "Count", 0);
        for (i=0; i<n; i++) {
                sprintf(tbuf, "Type%i", i);
                t = getInt("Sidebar", tbuf, 0);
                if (t == 0) continue;
                sprintf(tbuf, "Address%i", i);
                a = getInt("Sidebar", tbuf, 0);
                sprintf(tbuf, "Data%i", i);
                s1 = getString("Sidebar", tbuf, "");
                l1 = scan16(s1, ar1, sizeof(ar1), true);
                sprintf(tbuf, "Text%i", i);
                p = getString("Sidebar", tbuf, "");
                if (p[0] == 0) p = NULL;
                addSideEntryFast(t, a, p, ar1, l1);
        }
        updateSidebar();

        startReadSection("Names");
        while ((pp = nextPair()) != NULL) {
                a = strtoul(pp, &ppp, 16);
                if (*ppp == '=') {
                        pp = ppp + 1;
                        addNameFast(pp, a, true);
                }
        }

        updateSideNames();

        n = getInt("Watches", "Count", 0);
        WP.count = n;
        if (WP.items) free(WP.items);
        if (WP.count > 0) WP.items = (struct watchitem_t *)calloc(1, n*sizeof(struct watchitem_t));
        for (i=0; i<n; i++) {
                watchitem *wi = &(WP.items[i]);
                sprintf(tbuf, "W%iAddress", i);
                a = getInt("Watches", tbuf, NONE);
                if (a == NONE) continue;
                wi->Address = a;
                sprintf(tbuf, "W%iRef", i);
                wi->Ref = getInt("Watches", tbuf, 0);
                sprintf(tbuf, "W%iType", i);
                wi->Type = getInt("Watches", tbuf, 2);
                sprintf(tbuf, "W%iLength", i);
                wi->Length = getInt("Watches", tbuf, 1);
                sprintf(tbuf, "W%iOff1", i);
                wi->Offset[0] = getInt("Watches", tbuf, 0);
                sprintf(tbuf, "W%iOff2", i);
                wi->Offset[1] = getInt("Watches", tbuf, 0);
                sprintf(tbuf, "W%iOff3", i);
                wi->Offset[2] = getInt("Watches", tbuf, 0);
                sprintf(tbuf, "W%iOff4", i);
                wi->Offset[3] = getInt("Watches", tbuf, 0);
        }

        n = getInt("AddressHistory", "Count", 0);
        AdrSaveList = (char **)malloc((n+1) * sizeof(char *));
        for (i=0; i<n; i++) {
                sprintf(tbuf, "%i", i);
                s1 = getString("AddressHistory", tbuf, "");
                AdrSaveList[i] = strdup(s1);
        }
        AdrSaveList[n] = NULL;

        getSessionPath(name, buf);
        p = buf + strlen(buf) - 4;
        if (stricmp(p, ".ads") == 0) {
                strcpy(p, ".log");
        } else {
                strcat(buf, ".log");
        }
        LogFile = strdup(buf);
        Logfd = fopen(buf, "w");

        updateHeader();

        closeIniFile();
}

void newSession() {

        char name[256];

        name[0] = '\0';
        if (EditAscForm->Invoke("Name", name, false, 255) == NULL) return;
        readSession(name);

}

void saveSession() {

        int i, n, len, bsize;
        char buf[2048], tbuf[16];
        unsigned int a;
        patch **pp;
        aname **xx;
        snap **sn;
        sb_entry *sbe;
        char *s;

        getSessionPath(SessionName, buf);
        jumpToFile(buf);
        openIniFile(buf);

        setInt("Main", "ID", SessionID);

        setInt("Main", "FlashAddress", FlashAddress);
        setInt("Main", "FlashSize", FlashSize);
        setInt("Main", "NucleusLow", NucleusLow);
        setInt("Main", "NucleusHigh", NucleusHigh);

        for (i=0; i<5; i++) {
                sprintf(tbuf, "C%i", i);
                setStringEscaped("BootCommands", tbuf, BootCmd[i]);
        }

        for (i=0; i<9; i++) {
                sprintf(tbuf, "Source%i", i);
                setString("Editor", tbuf, AP[i].inuse ? shortName(AP[i].filename) : "");
                sprintf(tbuf, "Target%i", i);
                setInt("Editor", tbuf, AP[i].inuse ? AP[i].target : NONE);
        }

        a = rpGetAddress(&RP[0]);
        setInt("Appearance", "RP1address", a);
        a = rpGetAddress(&RP[1]);
        setInt("Appearance", "RP2address", a);
        a = cpGetAddress();
        setInt("Appearance", "CPaddress", a);

        clearSection("Names");

        n = getNameCount(true);
        xx = getNameList(true);
        for (i=0; i<n; i++) {
                sprintf(tbuf, "%08X", xx[i]->address);
                addString("Names", tbuf, xx[i]->name);
        }

        clearSection("Patches");

        n = getPatchCount();
        pp = getPatchList();
        setInt("Patches", "Count", n);
        for (i=0; i<n; i++) {
                sprintf(tbuf, "Address%i", i);
                setInt("Patches", tbuf, pp[i]->address);
                sprintf(tbuf, "OldData%i", i);
                s = toHexString(pp[i]->olddata, pp[i]->length);
                setString("Patches", tbuf, s);
                free(s);
                sprintf(tbuf, "NewData%i", i);
                s = toHexString(pp[i]->newdata, pp[i]->length);
                setString("Patches", tbuf, s);
                sprintf(tbuf, "Enabled%i", i);
                setInt("Patches", tbuf, pp[i]->disabled ? 0 : 1);
                free(s);
        }

        clearSection("Snappoints");

        sn = getSnapList();
        for (i=1; i<16; i++) {
                if (sn[i] == NULL) continue;
                sprintf(tbuf, "Address%i", i);
                setInt("Snappoints", tbuf, sn[i]->addr);
                sprintf(tbuf, "Thumb%i", i);
                setInt("Snappoints", tbuf, sn[i]->thumb ? 1 : 0);
                sprintf(tbuf, "Text%i", i);
                setStringEscaped("Snappoints", tbuf, sn[i]->text);
                sprintf(tbuf, "Condition%i", i);
                setStringEscaped("Snappoints", tbuf, sn[i]->condition);
                sprintf(tbuf, "Instruction%i", i);
                setInt("Snappoints", tbuf, sn[i]->instruction);
                sprintf(tbuf, "Width%i", i);
                setInt("Snappoints", tbuf, sn[i]->width);
        }

        clearSection("Sidebar");

        setInt("Sidebar", "Count", sb_count);
        for (i=0; i<sb_count; i++) {
                sbe = &(sb_list[i]);
                sprintf(tbuf, "Type%i", i);
                setInt("Sidebar", tbuf, sbe->type);
                sprintf(tbuf, "Address%i", i);
                if (sbe->address != 0) setInt("Sidebar", tbuf, sbe->address);
                sprintf(tbuf, "Text%i", i);
                if (sbe->text != NULL) setString("Sidebar", tbuf, sbe->text);
                sprintf(tbuf, "Data%i", i);
                if (sbe->data != NULL && sbe->len != 0) {
                        s = toHexString(sbe->data, sbe->len);
                        setString("Sidebar", tbuf, s);
                        free(s);
                }
        }

        clearSection("Watches");

        n = WP.count;
        setInt("Watches", "Count", n);
        for (i=0; i<n; i++) {
                watchitem *wi = &(WP.items[i]);
                sprintf(tbuf, "W%iAddress", i);
                setInt("Watches", tbuf, wi->Address);
                sprintf(tbuf, "W%iRef", i);
                setInt("Watches", tbuf, wi->Ref);
                sprintf(tbuf, "W%iType", i);
                setInt("Watches", tbuf, wi->Type);
                sprintf(tbuf, "W%iLength", i);
                setInt("Watches", tbuf, wi->Length);
                sprintf(tbuf, "W%iOff1", i);
                if (wi->Ref >= 1) setInt("Watches", tbuf, wi->Offset[0]);
                sprintf(tbuf, "W%iOff2", i);
                if (wi->Ref >= 2) setInt("Watches", tbuf, wi->Offset[1]);
                sprintf(tbuf, "W%iOff3", i);
                if (wi->Ref >= 3) setInt("Watches", tbuf, wi->Offset[2]);
                sprintf(tbuf, "W%iOff4", i);
                if (wi->Ref >= 4) setInt("Watches", tbuf, wi->Offset[3]);
        }

        n = GotoForm->AddressEdit->Items->Count;
        if (n > 16) n = 16;
        setInt("AddressHistory", "Count", n);
        for (i=0; i<n; i++) {
                sprintf(tbuf, "%i", i);
                setString("AddressHistory", tbuf, GotoForm->AddressEdit->Items->Strings[i].c_str());
        }

        closeIniFile();
}

void saveSessionAs() {

        char buf[1024];
        int sid;

        if (! MainForm->SaveSessionAsDlg->Execute()) return;

        randomize();
        while (sid == 0) {
                sid = rand();
        }
        getSessionPath(SessionName, buf);
        jumpToFile(buf);
        openIniFile(buf);
        setInt("Main", "ID", sid);
        closeIniFile();

        SessionName = strdup(MainForm->SaveSessionAsDlg->FileName.c_str());
        saveSession();

        updateHeader();
}

bool closeSession() {

        for (int i=0; i<9; i++) {
                if (AP[i].inuse) {
                        if (! saveChanges(&AP[i])) return false;
                }
        }

        saveSession();
        for (int i=0; i<9; i++) {
                if (AP[i].inuse) {
                        AP[i].inuse = false;
                        AP[i].target = NONE;
                        AP[i].binfile = NULL;
                        AP[i].sheet->Visible = false;
                }
        }

        if (Logfd != NULL) fclose(Logfd);
        SessionID = rand();
        return true;

}

void newSource(asmpage *ap) {

        ap->page->Clear();
        ap->filename = "";
        ap->target = NONE;
        ap->sheet->Caption = "Untitled";
        ap->sheet->TabVisible = true;
        ap->sheet->PageIndex = 8;
        ap->inuse = true;
        ap->binfile = NULL;

}


bool readSource(char *name, asmpage *ap, bool showmessage) {

        HANDLE H;
        unsigned long nr;
        int l;
        char *buf, *p, *pp;
        char tbuf[2048];

        TTabSheet *t = ap->sheet;
        TRichEdit *m = ap->page;

        getSessionPath(SessionName, tbuf);
        jumpToFile(tbuf);

        H = CreateFile(name,
                        GENERIC_READ,
                        0,
                        NULL,
                        stricmp(name, "Program1.asm") == 0 ? OPEN_ALWAYS : OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);

        if (H == INVALID_HANDLE_VALUE) {
                if (showmessage) {
                        WarningMessage("Cannot open %s", name);
                }
                return false;
        }

        l = GetFileSize(H, NULL);
        buf = (char *)malloc(l+2);
        if (! ReadFile(H, buf, l, &nr, NULL)) {
                if (showmessage) {
                        WarningMessage("Error reading %s", name);
                }
                CloseHandle(H);
                return false;
        }

        t->Caption = makeCaption(name, ap->target);
        t->TabVisible = true;

        buf[nr] = 0;
        m->Clear();
        m->Lines->SetText(buf);
        m->SelStart = 0;
        m->Tag = 0;
        free(buf);

        CloseHandle(H);

        ap->filename = longName(name);
        ap->sheet->TabVisible = true;
        ap->inuse = true;
        ap->binfile = NULL;

        return true;
}

bool writeSource(char *name, asmpage *ap) {

        HANDLE H;
        char tbuf[2048];
        char *buf;
        unsigned long nw;

        TRichEdit *m = ap->page;

        getSessionPath(SessionName, tbuf);
        jumpToFile(tbuf);

        H = CreateFile(name,
                        GENERIC_WRITE,
                        0,
                        NULL,
                        CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);

        if (H == INVALID_HANDLE_VALUE) {
                return WarningMessage("Cannot open %s", name);
        }

        buf = m->Lines->GetText();
        if (! WriteFile(H, buf, strlen(buf), &nw, NULL)) {
                WarningMessage("Error writing %s", name);
                CloseHandle(H);
                return false;
        }

        ap->filename = longName(name);
        m->Tag = 0;
        CloseHandle(H);
        return true;
}

bool saveChanges(asmpage *ap) {

        char buf[256];

        if (ap->page->Tag == 1) {
                sprintf(buf, "Save changes to %s?", ap->sheet->Caption);
                switch (MessageBox(NULL, buf, "ArmDebugger", MB_YESNOCANCEL|MB_ICONQUESTION)) {

                        case IDYES:
                                return writeSource(ap->filename, ap);
                        case IDNO:
                                return true;
                        case IDCANCEL:
                                return false;

                }
        }
        return true;
}

void closePage(asmpage *ap) {

        ap->filename = "";
        ap->sheet->TabVisible = false;
        ap->inuse = false;
        ap->binfile = NULL;
        ap->target = NONE;

}


