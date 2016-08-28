#include <vcl.h>
#include <stdio.h>
#include "DForm.h"
#include "Utility.h"

char *digits = "0123456789ABCDEF";
char *regnames[16] = {
        "R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7",
        "R8", "R9", "R10", "R11", "R12", "SP", "LR", "PC"
};

static char *msgbuf = NULL;
static unsigned long CalibratedLoop = 1;
bool ExtraFeatures;

void disableFullScreen() {

        static long z=0;
        long r;
        HKEY HK;

        r = RegOpenKeyEx(HKEY_CURRENT_USER, "Console", 0, KEY_ALL_ACCESS,&HK);
        if (r != ERROR_SUCCESS) return;
        RegSetValueEx(HK, "FullScreen", 0, REG_DWORD, (BYTE *)&z, sizeof(z));
        RegCloseKey(HK);

}

bool InfoMessage(char *fmt, ...) {

        va_list ap;

        if (msgbuf == NULL) msgbuf = (char *)malloc(4096);
        va_start(ap, fmt);
        vsnprintf(msgbuf, 4096, fmt, ap);
        va_end(ap);

        MessageBox(NULL, msgbuf, "ArmDebugger", MB_OK|MB_ICONINFORMATION);
        return true;

}

bool WarningMessage(char *fmt, ...) {

        va_list ap;

        if (msgbuf == NULL) msgbuf = (char *)malloc(4096);
        va_start(ap, fmt);
        vsnprintf(msgbuf, 4096, fmt, ap);
        va_end(ap);

        MessageBox(NULL, msgbuf, "ArmDebugger", MB_OK|MB_ICONWARNING);
        return false;

}

bool ErrorMessage(char *fmt, ...) {

        va_list ap;

        if (msgbuf == NULL) msgbuf = (char *)malloc(4096);
        va_start(ap, fmt);
        vsnprintf(msgbuf, 4096, fmt, ap);
        va_end(ap);

        MessageBox(NULL, msgbuf, "Warning", MB_OK|MB_ICONERROR);
        return false;

}

bool YesNoQuestion(char *fmt, ...) {

        va_list ap;
        int r;

        if (msgbuf == NULL) msgbuf = (char *)malloc(4096);
        va_start(ap, fmt);
        vsnprintf(msgbuf, 4096, fmt, ap);
        va_end(ap);

        r = MessageBox(NULL, msgbuf, "Warning", MB_YESNO|MB_ICONQUESTION);
        return (r == IDYES) ? true : false;

}

void setupExtraUI(bool e) {

        int i;

        for (i=0; i<MainForm->DebugSubmenu->Count; i++) {
                MainForm->DebugSubmenu->Items[i]->Enabled = e;
        }
        MainForm->Patch12->Enabled = e;
        MainForm->Watchpoint2->Enabled = e;
        MainForm->Memorymap1->Enabled = e;
        MainForm->Assemble12->Enabled = e;
        MainForm->Atomicassemble1->Enabled = e;
        MainForm->References2->Enabled = e;
        MainForm->Search1->Enabled = e;
        MainForm->SearchAgain1->Enabled = e;
        MainForm->eHexSearch->Enabled = e;
        MainForm->bSearchType->Enabled = e;
        MainForm->bSearch->Enabled = e;
        MainForm->bSearchTab->Enabled = e;
        MainForm->NucleusRTOSresources1->Enabled = e;
        MainForm->bVkp->Enabled = e;

        MainForm->bStepInto->Enabled = e;
        MainForm->bStepOver->Enabled = e;
        for (i=0; i<MainForm->Debug1->Count; i++) {
                MainForm->Debug1->Items[i]->Enabled = e;
        }

}

void setupMainUI(TColor indy_color, char *indy_cap, char *s_text, bool e_con, bool e_onl) {

        MainForm->Indy->Color = indy_color;
        MainForm->Indy->Caption = indy_cap;
        MainForm->SBar->Panels->Items[0]->Text = s_text;

        MainForm->cbComPort->Enabled = ! e_con;
        MainForm->bConnect->Visible = ! e_con;
        MainForm->bDisconnect->Visible = e_con;
        MainForm->bBoot->Enabled = e_con;

        MainForm->bRun->Enabled = e_onl;
        MainForm->bOff->Enabled = e_onl;
        MainForm->AtLoadto->Enabled = e_onl;
        MainForm->AtRun->Enabled = e_onl;
        MainForm->Savememorytofile1->Enabled = e_onl;
        MainForm->CPUwindow1->Enabled = e_onl;
        MainForm->CP15registers1->Enabled = e_onl;
        MainForm->Memorymap1->Enabled = e_onl;
        MainForm->Calladdress1->Enabled = e_onl;
        MainForm->Terminal1->Enabled = e_onl;
        MainForm->RAMAccessAnalyzer1->Enabled = e_onl;
        MainForm->vCPU->PopupMenu = e_onl ? MainForm->CPUPopup : MainForm->CPUPopup2;
        MainForm->Dump1->PopupMenu = e_onl ? MainForm->DumpMenu : NULL;
        MainForm->Dump2->PopupMenu = e_onl ? MainForm->DumpMenu : NULL;
        MainForm->Registers->PopupMenu = e_onl ? MainForm->RegistersPopup : NULL;

        MainForm->Breakpoints1->Enabled = ExtraFeatures || !e_onl;
        MainForm->Patches1->Enabled = ExtraFeatures || !e_onl;
        MainForm->bPatches->Enabled = ExtraFeatures || !e_onl;
        MainForm->bSnaps->Enabled = ExtraFeatures || !e_onl;

        setupExtraUI(ExtraFeatures ? e_onl : false);

}

int shiftState() {

        return (GetKeyState(VK_SHIFT) & ~1);

}

int ctrlState() {

        return (GetKeyState(VK_CONTROL) & ~1);

}

void m_print(char *msg, ...) {

   va_list ap;
   AnsiString s;

   va_start(ap, msg);
   s.vprintf(msg, ap);
   va_end(ap);
   AnsiString s1 = s.Trim();
   delete &s;

   MainForm->Mes->Lines->Append(s1);
   MainForm->Mes->BringToFront();
   MainForm->LogWindow->Tag = 0;

}

void m_append(char *msg) {

   char buf[256];

   strcpy(buf, MainForm->Mes->Lines->Strings[MainForm->Mes->Lines->Count-1].c_str());
   strcat(buf, msg);
   MainForm->Mes->Lines->Strings[MainForm->Mes->Lines->Count-1] = buf;
   MainForm->Mes->BringToFront();

}


void updateFreePoolEntries(int n) {

        char buf[64];

        sprintf(buf, " Pool: %i", n);
        MainForm->SBar->Panels->Items[2]->Text = buf;

}

static inline void dl() { for (int i=0; i<1000; i++) ; }

void calibrateDelayLoop() {

        SYSTEMTIME t1, t2, t3;
        unsigned long ns[10];
        unsigned long n, maxn;
        int i, k, maxi;

        for (i=0; i<10; i++) {

                n = 0;
                GetSystemTime(&t1);
                do { GetSystemTime(&t2); } while (t1.wMilliseconds == t2.wMilliseconds);

                do { GetSystemTime(&t3); dl(); n++; } while (t2.wMilliseconds == t3.wMilliseconds);
                ns[i] = n;

        }

        for (k=0; k<5; k++) {
                maxn = 0; maxi = 0;
                for (i=0; i<10; i++) {
                        if (ns[i] > maxn) { maxn = ns[i]; maxi = i; }
                }
                ns[maxi] = 0;
        }

        n = 0; k = 0;
        for (i=0; i<10; i++) {
                if (ns[i] != 0) { n += ns[i]; k++; }
        }
        n /= k;

        CalibratedLoop = n;

}

void uSleep(int us) {

        SYSTEMTIME t1;

        int i = 0;
        int n = (us * CalibratedLoop) / 1000;
        do { GetSystemTime(&t1); dl(); i++; } while (i < n);

}


unsigned int readhex(char *p) {

        char c1, c2;

        if (! isxdigit(*p) || ! isxdigit(*(p+1))) return 0xff;
        c1 = *p; if (c1 > 0x60) c1-=0x20; c1-=0x30; if (c1>9) c1-=7;
        c2 = *(p+1); if (c2 > 0x60) c2-=0x20; c2-=0x30; if (c2>9) c2-=7;
        return (c1<<4) + c2;
}

int scan16(char *s, unsigned char *buf, int len, bool skipspaces) {

        char c1, c2;
        int n = 0;

        while (n < len) {
                c1 = *s;
                if (skipspaces && (c1 == ' ' || c1 == '\t')) { s++; continue; }
                c2 = *(s+1);
                if ((! isxdigit(c1)) || (! isxdigit(c2))) break;
                if (c1 > 0x60) c1 -= 0x20; c1 -= 0x30; if (c1 > 9) c1 -= 7;
                if (c2 > 0x60) c2 -= 0x20; c2 -= 0x30; if (c2 > 9) c2 -= 7;
                buf[n] = (c1 << 4) | c2;
                n++;
                s += 2;
        }

        return n;
}

int scan16m(char *s, unsigned char *buf, unsigned char *mask, int len) {

        char c1, c2;
        int n = 0;

        while (n < len) {
                c1 = *s;
                if (c1 == ' ') { s++; continue; }
                c2 = *(s+1);
                mask[n] = 0xff;
                if (c1 == '?') { c1 = '0'; mask[n] &= 0x0f; }
                if (c2 == '?') { c2 = '0'; mask[n] &= 0xf0; }
                if ((! isxdigit(c1)) || (! isxdigit(c2))) break;
                if (c1 > 0x60) c1 -= 0x20; c1 -= 0x30; if (c1 > 9) c1 -= 7;
                if (c2 > 0x60) c2 -= 0x20; c2 -= 0x30; if (c2 > 9) c2 -= 7;
                buf[n] = (c1 << 4) | c2;
                n++;
                s += 2;
        }

        return n;
}

int ustrncmp(unsigned char *s1, unsigned char *s2, int len) {

        unsigned short *p1, *p2;
        int i = 0;

        p1 = (unsigned short *)s1;
        p2 = (unsigned short *)s2;
        while ((*p1 != 0 || *p2 != 0) && i < len) {
                if (*p1 != *p2) return p1-p2;
                p1++;
                p2++;
                i++;
        }
        return 0;
}

char *toHexString(unsigned char *data, int len) {

        char *s = (char *)malloc(len*2+4);
        for (int i=0; i<len; i++) {
                sprintf(s+i*2, "%02X", data[i]);
        }
        return s;

}

void getClipboard(char *buf, int size) {

        HGLOBAL H;
        int l;
        char *p;

        buf[0] = 0;
        if (! OpenClipboard(NULL)) return;

        H = GetClipboardData(CF_TEXT);
        if (H == NULL) {
                CloseClipboard();
                return;
        }

        p = (char *)GlobalLock(H);
        if (p == NULL) {
                CloseClipboard();
                return;
        }

        l = strlen(p);
        if (l >= size) l = size-1;
        memcpy(buf, p, l);
        buf[l] = 0;

        GlobalUnlock(H);
        CloseClipboard();

}

bool setClipboard(char *text) {

        HGLOBAL H;
        int n;
        char *p;

        n = strlen(text);
        H = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, n+1);
        if (H == NULL) return false;

        if (! OpenClipboard(NULL)) return false;
        p = (char *)GlobalLock(H);
        strcpy(p, text);
        GlobalUnlock(H);

        EmptyClipboard();
        if (! SetClipboardData(CF_TEXT, H)) {
                GlobalFree(H);
                return false;
        }
        CloseClipboard();
        return true;

}


