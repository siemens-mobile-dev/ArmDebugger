//---------------------------------------------------------------------------
#include <vcl.h>
#include <stdio.h>
#include "Config.h"
#include "Debug.h"
#include "Wire.h"
#include "Utility.h"
#pragma hdrstop

#include "RAMAccess.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TAccessAnalyzer *AccessAnalyzer;

#include "asm\arm_analyzer.h"
#include "asm\arm_iszero.h"
#include "asm\arm_readCP15.h"

//---------------------------------------------------------------------------
__fastcall TAccessAnalyzer::TAccessAnalyzer(TComponent* Owner)
        : TForm(Owner)
{
}

//---------------------------------------------------------------------------

void busy() {

        AccessAnalyzer->bInit->Enabled = false;
        AccessAnalyzer->bAnalyze->Enabled = false;
        AccessAnalyzer->bClose->Enabled = false;
        AccessAnalyzer->State->Caption = "Working...";
        AccessAnalyzer->Repaint();
        Sleep(1);

}

void ready() {

        AccessAnalyzer->bInit->Enabled = true;
        AccessAnalyzer->bAnalyze->Enabled = true;
        AccessAnalyzer->bClose->Enabled = true;
        AccessAnalyzer->State->Caption = " ";
        AccessAnalyzer->Repaint();

}


void __fastcall TAccessAnalyzer::Invoke() {

        StaticText1->Caption =
                "1. Reboot the phone\n"
                "2. Press \"Initialize\"\n"
                "3. Try to use ALL phone functionality\n"
                "4. Press \"Analyze\"\n"
                "5. Write 2-3 largest sections to devices.ini\n"
                "6. Reboot the phone";

        Visible = true;

}

//---------------------------------------------------------------------------


void __fastcall TAccessAnalyzer::bCloseClick(TObject *Sender)
{
        Visible = false;
}
//---------------------------------------------------------------------------
void __fastcall TAccessAnalyzer::bInitClick(TObject *Sender)
{
        static unsigned char arm_read10[] = {
                0x10, 0x00, 0xA0, 0xE3, 0x00, 0x00, 0x90, 0xE5,
                0x1E, 0xFF, 0x2F, 0xE1
        };

        unsigned int r[16], cpsr;
        unsigned int buf[4096], spt[4096];
        unsigned int a, e_addr;
        int e_count;
        int i, j;

        if (! is_online()) {
                WarningMessage("Not connected");
                return;
        }

        busy();

        ExtraFeatures = false;
        setSystemState(SS_READY);

        e_addr = TransTableAddr + (ExtRAMaddress / 0x100000) * 4;
        e_count = (ExtRAMsize / 0x100000);

        ensureRAMisCoarse();

        if (! readMemory(e_addr, (unsigned char *)buf, e_count * 4))
               { WarningMessage("Error reading translation table"); return; }

        if (! execute(arm_read10, sizeof(arm_read10), r, 1, cpsr, 250))
                { WarningMessage("Cannot read vector information"); return; }
        if ((r[0] & 0xfffff000) != 0xe59ff000)
                { WarningMessage("Unknown format of exception vector table"); return; }

        readMemory(0, (unsigned char *)spt, 256);
        a = 0x10 + (r[0] & 0xfff) + 8;
        a = spt[a/4];
        setMemory(a, arm_analyzer, sizeof(arm_analyzer), SM_INTERNAL);

        for (i=0; i<e_count; i++) {
                if ((buf[i] & 0x03) == 0x01) {
                        // coarse
                        a = buf[i] & 0xfffffc00;
                        readMemory(a, (unsigned char *)spt, 0x400);
                        for (j=0; j<0x100; j++) {
                                spt[j] &= ~0xff0;
                        }
                        setMemory(a, (unsigned char *)spt, 0x400, SM_INTERNAL);
                }
        }
        ready();
}
//---------------------------------------------------------------------------
void __fastcall TAccessAnalyzer::bAnalyzeClick(TObject *Sender)
{
        unsigned int spt[4096], buf[256];
        char state[16384];
        char tbuf[16];
        unsigned int a, r1, n, ta;
        unsigned int r[16], cpsr;
        unsigned int e_addr;
        int e_count;
        int i, j;

        if (! is_online()) {
                WarningMessage("Not connected");
                return;
        }

        busy();

        PageList2->Lines->Clear();

        for (i=0; i<16384; i++) {
                state[i] = 0;
        }

        ta = getTempAddress();
        setMemory(ta, arm_iszero, sizeof(arm_iszero), SM_INTERNAL);

        e_addr = TransTableAddr + (ExtRAMaddress / 0x100000) * 4;
        e_count = (ExtRAMsize / 0x100000);

        if (! readMemory(e_addr, (unsigned char *)buf, e_count * 4))
               { WarningMessage("Error reading translation table"); return; }

        for (i=0; i<e_count; i++) {
                if ((buf[i] & 0x03) == 0x01) {
                        // coarse
                        a = buf[i] & 0xfffffc00;
                        readMemory(a, (unsigned char *)spt, 0x400);
                        for (j=0; j<0x100; j++) {
                                if ((spt[j] & 0xff0) == 0) {
                                        r[1] = ExtRAMaddress + i*1048576 + j*4096;
                                        if (! armCall(ta, r, 2, cpsr, 1000)) {
                                                WarningMessage("Communication error");
                                                return;
                                        }
                                        if (r[0] == 0) {
                                                state[i*256+j] = 1;
                                        }
                                }
                                sprintf(tbuf, "%i of %i", i*256+j, e_count*256);
                                AccessAnalyzer->State->Caption = tbuf;
                                AccessAnalyzer->Repaint();
                                Sleep(1);
                        }
                }
        }

        i = 0;
        while (i < 2048) {
                if (state[i] == 1) {
                        a = 0xa8000000 + i * 4096;
                        n = 0;
                        while (i < 2048 && state[i] == 1) {
                                n++;
                                i++;
                        }
                        if (n >= 5) {
                                sprintf(tbuf, "%08X  %i blocks", a, n);
                                PageList2->Lines->Add(tbuf);
                        }
                }
                i++;
        }

        ready();
}
//---------------------------------------------------------------------------
