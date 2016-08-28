//---------------------------------------------------------------------------

#include <vcl.h>
#include <stdio.h>
#pragma hdrstop

#include "Xrefs.h"
#include "Config.h"
#include "Debug.h"
#include "Wire.h"
#include "Disarm.h"
#include "Utility.h"
#include "Globals.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

#include "asm\arm_xref.h"

TXrefForm *XrefForm;

unsigned int reg_addr[16];
unsigned int reg_len[16];
unsigned int reg_type[16];
int nregs;

unsigned int target;
bool searching = false;
bool cancelRequest = false;

unsigned int current;
int region;
unsigned int X[4096];
unsigned char T[4096];
int nx;
//---------------------------------------------------------------------------
__fastcall TXrefForm::TXrefForm(TComponent* Owner)
        : TForm(Owner)
{
        Grid->RowCount = 2;
        Grid->ColWidths[1] = 250;
        Grid->Cells[0][0] = " Address";
        Grid->Cells[1][0] = " Instruction/Data";
        Grid->Cells[0][1] = "";
        Grid->Cells[1][1] = "";
}
//---------------------------------------------------------------------------
bool TXrefForm::Invoke(unsigned int t) {

        char buf[256];
        memoryinfo mi;

        target = t;
        getMemoryInfo(target, &mi);
        if (mi.len == 0) return false;

        nregs = 0;
        for (int i=0; mem_type[i]!=0; i++) {
                if (mem_type[i] == M_RAM) {
                        reg_addr[nregs] = mem_addr[i];
                        reg_len[nregs] = mem_len[i];
                        reg_type[nregs] = mem_type[i];
                        nregs++;
                }
        }
        for (int i=0; mem_type[i]!=0; i++) {
                if (mem_type[i] == M_FLASH) {
                        reg_addr[nregs] = mem_addr[i];
                        reg_len[nregs] = mem_len[i];
                        reg_type[nregs] = mem_type[i];
                        nregs++;
                }
        }
        if (nregs == 0) return false;

        if (! writeToRAM(LoadableCodeAddr, arm_xref, sizeof(arm_xref))) {
                return WarningMessage("An error occured");
        }

        region = -1;
        cancelRequest = false;
        StatusBar1->SimpleText = " Searching... (press ESC to stop)";

        nx = 0;
        sprintf(buf, "References to %08X", target);
        Caption = buf;
        Visible = true;
        BringToFront();

        searching = true;
        XrefForm->Grid->Cursor = crAppStart;
        XTimer->Enabled = true;
        return true;
}


bool processStage0(unsigned int len) {

        unsigned int r[18], cpsr;
        unsigned char buf[64];
        char text[256], *p;
        instruction *ins;

        r[0] = 0;
        r[1] = current;
        r[2] = current + len;
        r[3] = target;

        if (! armCall(LoadableCodeAddr, r, 4, cpsr, 5000)) {
                return false;
        }

        unsigned int a = r[0];
        if (a == NONE) {
                current += len;
        } else {
                int th = a & 1;
                a &= ~1;
                current = a+4;
                X[nx] = a;
                if (readMemory(a, buf, 4)) {
                        unsigned int v = *((unsigned int *)buf);
                        if ((v & ~3) == (target & ~3)) {
                                sprintf(text, "0x%08X", v);
                                T[nx] = 0;
                        } else {
                                ins = (th == 1) ? disthumb(a, v, r) : disarm(a, v, r);
                                for (p=ins->resolved; *p!=0; p++) if (*p == '\t') *p = ' ';
                                sprintf(text, "%s", ins->resolved);
                                T[nx] = 1;
                        }
                } else {
                        sprintf(text, "?");
                        T[nx] = 0;
                }
                nx++;
                XrefForm->Grid->RowCount = nx+1;
                XrefForm->Grid->Cells[0][nx] = IntToHex((int)a, 8);
                XrefForm->Grid->Cells[1][nx] = text;
                XrefForm->Repaint();
        }

        return true;
}

bool processStage1() {

        return true;

}

void stopSearch() {

        searching = false;
        XrefForm->XTimer->Enabled = false;
        XrefForm->StatusBar1->SimpleText = "";
        XrefForm->Grid->Cursor = crDefault;

}


void __fastcall TXrefForm::XTimerTimer(TObject *Sender)
{
        XTimer->Enabled = false;
        if (cancelRequest || nx >= 4096) {
                stopSearch();
                return;
        }
        if (region < 0 || current >= reg_addr[region]+reg_len[region]-1) {
                region++;
                if (region >= nregs) {
                        processStage1();
                        stopSearch();
                        return;
                }
                if (reg_type[region] == M_RAM) {
                        StatusBar1->SimpleText = " Searching RAM... (press ESC to stop)";
                } else {
                        StatusBar1->SimpleText = " Searching FLASH... (press ESC to stop)";
                }
                StatusBar1->Repaint();
                current = reg_addr[region];
        }

        unsigned int len = reg_addr[region]+reg_len[region]-current;
        if (len > 0x100000) len = 0x100000;
        if (! processStage0(len)) {
                stopSearch();
                return;
        }

        XTimer->Enabled = true;
}
//---------------------------------------------------------------------------

void __fastcall TXrefForm::FormKeyPress(TObject *Sender, char &Key)
{
        if (Key == 0x1b) {
                if (searching) {
                        cancelRequest = true;
                } else {
                        Visible = false;
                }
        }
}
//---------------------------------------------------------------------------

void __fastcall TXrefForm::Goto1Click(TObject *Sender)
{
        int n = Grid->Row - 1;
        if (nx <= n) return;
        if (T[n] == 0) {
                rpSetAddress(RPC, X[n], true);
        } else {
                cpSetAddress(X[n], true);
        }
}
//---------------------------------------------------------------------------

void __fastcall TXrefForm::FormClose(TObject *Sender, TCloseAction &Action)
{
        if (searching) cancelRequest = true;
}
//---------------------------------------------------------------------------

