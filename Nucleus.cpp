//---------------------------------------------------------------------------

#include <vcl.h>
#include <stdio.h>
#pragma hdrstop

#include "DForm.h"
#include "Debug.h"
#include "Wire.h"
#include "Nucleus.h"
#include "Utility.h"
#include "Globals.h"

#include "asm\arm_pointers.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TNuForm *NuForm;

bool firstrun = true;

signature siglist[] = {
        { "DMCE_Create_Memory_Pool", NONE, "20209de5000050e324309de514609015=414e5944", 1 },
        { "TCF_Established_Tasks", NONE, "??4?2de9??????eb??0??fe5000090e5??8?bde8??4?2de9??????eb??0??fe5000090e5", 0 },
        { "TCF_Established_HISRs", NONE, "??4?2de9??????eb??0??fe5000090e5??8?bde8??4?2de9??????eb??0??fe5000090e5", -20 },
        { "TCF_Task_Pointers", NONE, "??4?2de9??????eb??0??fe5000090e5??8?bde8??4?2de9??????eb??0??fe5000090e5", -40 },
        { "TCF_HISR_Pointers", NONE, "0600A0E1??????EB0400A0E1??8?BDE8??4?2DE90050A0E1??0??FE50170A0E1006090E5", -16 },
        { "DMCE_Delete_Memory_Pool", NONE, "000050e30300000a141090e5=414e5944020051e1", 0 },
        { "DMCE_Allocate_Memory", NONE, "140095e5=414e5944010050e10100000a0f00e0e3", 1 },
        { "DMCE_Deallocate_Memory", NONE, "142092e5=414e5944030052e10810d10500005103??????0a0e00e0e3", 20 },
        { "DMF_Established_Memory_Pools", NONE, "0e00e0e31eff2fe1414e5944??4?2de9??????eb??0??fe5000090e5??8?bde8", -12 },
        { "DMF_Memory_Pool_Pointers", NONE, "0e00e0e31eff2fe1414e5944??4?2de9??????eb??0??fe5000090e5??8?bde8", -32 },
        { "PMF_Established_Partition_Pools", NONE, "0e00e0e31eff2fe154524150??4?2de9??????eb??0??fe5000090e5", -12 },
        { "PMF_Partition_Pool_Pointers", NONE, "0e00e0e31eff2fe154524150??4?2de9??????eb??0??fe5000090e5??8?bde8", -32 },
        { "QUF_Established_Queues", NONE, "55455551??4?2de9??????eb??0??fe5000090e5", -4 },
        { "TMF_Established_Timers", NONE, "4b534154??????????4?2de9??????eb??0??fe5000090e5??8?bde8", -8 },
        { "TMF_Timer_Pointers", NONE, "4b534154??????????4?2de9??????eb??0??fe5000090e5??8?bde8", -28 },
        { NULL, 0, NULL, 0 }
};

static int sigcomp(char *text, unsigned char *sig, unsigned char *mask) {

        char *p;
        unsigned int s, m;
        unsigned char *sp, *mp;
        int i, l;
        char c;
        bool ld;

        p = text;
        sp = sig;
        mp = mask;

        l = 0;
        while (*p != 0) {
                if (*p == '=') {
                        ld = true;
                        p++;
                } else {
                        ld = false;
                }
                s = m = 0;
                for (i=0; i<8; i++) {
                        c = *(p++); if (c > 0x60) c -= 0x20;
                        s <<= 4; m <<= 4;
                        if (c != '?') {
                                c -= 0x30; if (c > 9) c -= 7;
                                s |= c; m |= 0xf;
                        }
                }
                if (ld) m = 0x00007f0f;
                for (i=0; i<4; i++) {
                        *(sp++) = s >> 24; s <<= 8;
                        *(mp++) = m >> 24; m <<= 8;
                }
                l += 4;
        }

        return l;
}

unsigned int locate(char *name) {

        signature *s;

        s = siglist;
        while (s->name != NULL) {
                if (strcmp(s->name, name) == 0) return s->address;
                s++;
        }
        return NONE;

}

unsigned int callV(char *func, unsigned int *res) {

        unsigned int a, r[16], cpsr;

        a = locate(func);
        if (a == NONE) return false;
        if (! armCall(a, r, 0, cpsr, 250)) return false;
        *res = r[0];
        return true;

}

//---------------------------------------------------------------------------
__fastcall TNuForm::TNuForm(TComponent* Owner)
        : TForm(Owner)
{
        sgSymbols->ColCount = 2;
        sgSymbols->RowCount = 1;
        sgSymbols->ColWidths[0] = 250;
        sgSymbols->ColWidths[1] = 64;

        sgProc->ColCount = 8;
        sgProc->ColWidths[0] = 20;
        sgProc->ColWidths[1] = 90;
        sgProc->ColWidths[2] = 40;
        sgProc->ColWidths[3] = 40;
        sgProc->ColWidths[4] = 64;
        sgProc->ColWidths[5] = 60;
        sgProc->ColWidths[6] = 60;
        sgProc->ColWidths[7] = 120;
        sgProc->Cells[0][0] = "N";
        sgProc->Cells[1][0] = "Name";
        sgProc->Cells[2][0] = "Status";
        sgProc->Cells[3][0] = "Priority";
        sgProc->Cells[4][0] = "Scheduled";
        sgProc->Cells[5][0] = "Stack size";
        sgProc->Cells[6][0] = "Entry";
        sgProc->Cells[7][0] = "";

        sgHisr->ColCount = 8;
        sgHisr->ColWidths[0] = 20;
        sgHisr->ColWidths[1] = 90;
        sgHisr->ColWidths[2] = 0;
        sgHisr->ColWidths[3] = 40;
        sgHisr->ColWidths[4] = 64;
        sgHisr->ColWidths[5] = 60;
        sgHisr->ColWidths[6] = 60;
        sgHisr->ColWidths[7] = 120;
        sgHisr->Cells[0][0] = "N";
        sgHisr->Cells[1][0] = "Name";
        sgHisr->Cells[2][0] = "";
        sgHisr->Cells[3][0] = "Priority";
        sgHisr->Cells[4][0] = "Scheduled";
        sgHisr->Cells[5][0] = "Stack size";
        sgHisr->Cells[6][0] = "Entry";
        sgHisr->Cells[7][0] = "";

        sgDyna->ColCount = 8;
        sgDyna->ColWidths[0] = 20;
        sgDyna->ColWidths[1] = 90;
        sgDyna->ColWidths[2] = 64;
        sgDyna->ColWidths[3] = 64;
        sgDyna->ColWidths[4] = 64;
        sgDyna->ColWidths[5] = 0;
        sgDyna->ColWidths[6] = 64;
        sgDyna->ColWidths[7] = 64;
        sgDyna->Cells[0][0] = "N";
        sgDyna->Cells[1][0] = "Name";
        sgDyna->Cells[2][0] = "From addr";
        sgDyna->Cells[3][0] = "To addr";
        sgDyna->Cells[4][0] = "Size";
        sgDyna->Cells[5][0] = "";
        sgDyna->Cells[6][0] = "Free";
        sgDyna->Cells[7][0] = "% Used";

        sgPart->ColCount = 8;
        sgPart->ColWidths[0] = 20;
        sgPart->ColWidths[1] = 80;
        sgPart->ColWidths[2] = 64;
        sgPart->ColWidths[3] = 64;
        sgPart->ColWidths[4] = 45;
        sgPart->ColWidths[5] = 45;
        sgPart->ColWidths[6] = 60;
        sgPart->ColWidths[7] = 60;
        sgPart->Cells[0][0] = "N";
        sgPart->Cells[1][0] = "Name";
        sgPart->Cells[2][0] = "From addr";
        sgPart->Cells[3][0] = "To addr";
        sgPart->Cells[4][0] = "Size";
        sgPart->Cells[5][0] = "Part. size";
        sgPart->Cells[6][0] = "Free";
        sgPart->Cells[7][0] = "% Used";
}
//---------------------------------------------------------------------------

void TNuForm::Invoke() {

        Visible = true;
        BringToFront();
        if (firstrun) {
                FirstTimer->Enabled = true;
                NuPages->ActivePage = InfoPage;
        } else {
                UpdateTimer->Enabled = true;
        }

}

void updateInfoPage() {

        char buf[4096];
        unsigned int r;

        sprintf(buf,  "Tasks:           %i\n"
                      "HISRs:           %i\n"
                      "Memory pools:    %i\n"
                      "Partition pools: %i\n"
                      "Queues:          %i\n"
                      "Timers:          %i\n",
                callV("TCF_Established_Tasks", &r) ? r : 0,
                callV("TCF_Established_HISRs", &r) ? r : 0,
                callV("DMF_Established_Memory_Pools", &r) ? r : 0,
                callV("PMF_Established_Partition_Pools", &r) ? r : 0,
                callV("QUF_Established_Queues", &r) ? r : 0,
                callV("TMF_Established_Timers", &r) ? r : 0
        );

        NuForm->info1->Caption = buf;
        NuForm->info1->Repaint();

}

void updateProcessPage(TStringGrid *sg, TLabel *label, bool hisr) {

        static char cStatus[] = "rpsBQPSETMDft";
        static unsigned char task_offsets[] = { 0x10, 0x14, 0x18, 0x1c, 0x30, 0x5c };
        static unsigned char hisr_offsets[] = { 0x10, 0x14, 0x18, 0x1c, 0x30, 0x44 };
        unsigned char buf[4096];
        unsigned int r[16], cpsr;
        unsigned int *ip = (unsigned int *)arm_pointers;
        char *name, tbuf[64];
        unsigned int status, ds, prio, preempt, sched, stksize, entry;
        int i, j, n;

        ip[1] = locate(hisr ? "TCF_HISR_Pointers" : "TCF_Task_Pointers");
        ip[2] = TempExecAddr + 256;
        ip[3] = 6;
        memcpy(&ip[4], hisr ? hisr_offsets : task_offsets, 6);

        if (ip[1] == NONE ||
         ! execute(arm_pointers, sizeof(arm_pointers), r, 0, cpsr, 250) ||
         r[0] == 0 ||
         ! readMemory(TempExecAddr+512, buf, r[0]*4*6)) {
                sg->RowCount = 2;
                for (i=0; i<sg->ColCount; i++) sg->Cells[i][1] = "";
                label->Caption = "Information not available";
                return;
        }

        n = r[0];
        sprintf(tbuf, "Total: %i", n);
        label->Caption = tbuf;
        sg->RowCount = n+1;
        for (i=0; i<n; i++) {
                j = i*4*6;
                name = &buf[j];
                status = buf[j+8];
                ds = buf[j+9];
                prio = buf[j+0xa];
                preempt = buf[j+0xb];
                sched = *((unsigned int *)&buf[j+0x0c]);
                stksize = *((unsigned int *)&buf[j+0x10]);
                entry = *((unsigned int *)&buf[j+0x14]);

                sprintf(tbuf, "%i", i);
                sg->Cells[0][i+1] = tbuf;
                strncpy(tbuf, name, 8);
                tbuf[8] = 0;
                sg->Cells[1][i+1] = tbuf;
                sprintf(tbuf, "%c%c", (status<=12) ? cStatus[status] : '?', (preempt==0) ? 'n' : ' ');
                if (hisr) tbuf[0] = 0;
                sg->Cells[2][i+1] = tbuf;
                sprintf(tbuf, "%i", prio);
                sg->Cells[3][i+1] = tbuf;
                sprintf(tbuf, "%i", sched);
                sg->Cells[4][i+1] = tbuf;
                sprintf(tbuf, "%i", stksize);
                sg->Cells[5][i+1] = tbuf;
                sprintf(tbuf, "%08X", entry);
                sg->Cells[6][i+1] = tbuf;
        }

}

void updateMemoryPoolPage(TStringGrid *sg, TLabel *label, bool part) {

        static unsigned char dyna_offsets[] = { 0x18, 0x1c, 0x20, 0x24, 0x28, 0x2c };
        static unsigned char part_offsets[] = { 0x10, 0x14, 0x18, 0x1c, 0x20, 0x24 };
        unsigned char buf[4096];
        unsigned int r[16], cpsr;
        unsigned int *ip = (unsigned int *)arm_pointers;
        char *name, tbuf[64];
        unsigned int addr, size, partsize, avail;
        int i, j, n;

        ip[1] = locate(part ? "PMF_Partition_Pool_Pointers" : "DMF_Memory_Pool_Pointers");
        ip[2] = TempExecAddr + 256;
        ip[3] = 6;
        memcpy(&ip[4], part ? part_offsets : dyna_offsets, 6);

        if (ip[1] == NONE ||
         ! execute(arm_pointers, sizeof(arm_pointers), r, 0, cpsr, 250) ||
         r[0] == 0 ||
         ! readMemory(TempExecAddr+512, buf, r[0]*4*6)) {
                sg->RowCount = 2;
                for (i=0; i<sg->ColCount; i++) sg->Cells[i][1] = "";
                label->Caption = "Information not available";
                return;
        }

        n = r[0];
        sprintf(tbuf, "Total: %i", n);
        label->Caption = tbuf;
        sg->RowCount = n+1;
        for (i=0; i<n; i++) {
                j = i*4*6;
                name = &buf[j];
                addr = *((unsigned int *)&buf[j+0x08]);
                size = *((unsigned int *)&buf[j+0x0c]);
                partsize = *((unsigned int *)&buf[j+0x10]);
                avail = *((unsigned int *)&buf[j+0x14]);

                sprintf(tbuf, "%i", i);
                sg->Cells[0][i+1] = tbuf;
                strncpy(tbuf, name, 8);
                tbuf[8] = 0;
                sg->Cells[1][i+1] = tbuf;
                sprintf(tbuf, "%08X", addr);
                sg->Cells[2][i+1] = tbuf;
                sprintf(tbuf, "%08X", addr+size);
                sg->Cells[3][i+1] = tbuf;
                sprintf(tbuf, "%i", size);
                sg->Cells[4][i+1] = tbuf;
                sprintf(tbuf, "%i", partsize);
                if (! part) tbuf[0] = 0;
                sg->Cells[5][i+1] = tbuf;
                if (part) avail *= partsize;
                sprintf(tbuf, "%i", avail);
                sg->Cells[6][i+1] = tbuf;
                sprintf(tbuf, "%i%%", ((size-avail)*100)/size);
                sg->Cells[7][i+1] = tbuf;
        }

}

void updateCurrentPage() {

        TTabSheet *page = NuForm->NuPages->ActivePage;
        if (page == NuForm->InfoPage) {
                updateInfoPage();
        } else if (page == NuForm->ProcessPage) {
                updateProcessPage(NuForm->sgProc, NuForm->infoProc,  false);
        } else if (page == NuForm->HisrPage) {
                updateProcessPage(NuForm->sgHisr, NuForm->infoHisr, true);
        } else if (page == NuForm->DynaPage) {
                updateMemoryPoolPage(NuForm->sgDyna, NuForm->infoDyna, false);
        } else if (page == NuForm->PartPage) {
                updateMemoryPoolPage(NuForm->sgPart, NuForm->infoPart, true);
        }

}

void __fastcall TNuForm::FirstTimerTimer(TObject *Sender)
{
        unsigned char sig[64], mask[64];
        unsigned int a, aStart, aEnd, nuMax, nuMin;
        char buf[16];
        int l, n;
        signature *s;

        FirstTimer->Enabled = false;

        inprocess = true;

        if (NucleusLow == 0) {
                aStart = FlashAddress;
                aEnd = FlashAddress + 0x01000000;
                info1->Caption = "First run\nSearching Nucleus...";
        } else {
                aStart = NucleusLow-16;
                aEnd = NucleusHigh+16;
                info1->Caption = "Retrieving...";
        }
        NuForm->Repaint();

        nuMin = 0xffffffff;
        nuMax = 0;
        s = siglist;
        n = 0;
        while (s->name != NULL) {
                l = sigcomp(s->sig, sig, mask);
                a = searchSignature(sig, mask, l, s->offset, aStart, aEnd);
                if (s == siglist && NucleusLow == 0) {
                        if (a == NONE) {
                                WarningMessage("Cannot find Nucleus library");
                                inprocess = false;
                                return;
                        }
                        aStart = a - 65536;
                        if (aStart < FlashAddress) aStart = FlashAddress;
                        aEnd = a + 65536;
                }
                if (nuMin > a) nuMin = a;
                if (nuMax < a) nuMax = a;
                s->address = a;
                s++;
                n++;
        }



        NucleusLow = nuMin;
        NucleusHigh = nuMax;

        sgSymbols->RowCount = n;
        s = siglist;
        n = 0;
        while (s->name != NULL) {
                sgSymbols->Cells[0][n] = s->name;
                sprintf(buf, "%08X", s->address);
                sgSymbols->Cells[1][n] = (s->address == NONE) ? "" : buf;
                s++;
                n++;
        }

        firstrun = false;
        inprocess = false;
        UpdateTimer->Enabled = true;
        updateCurrentPage();

}
//---------------------------------------------------------------------------

void __fastcall TNuForm::UpdateTimerTimer(TObject *Sender)
{
        if (! Visible) { UpdateTimer->Enabled = false; return; }
        updateCurrentPage();
}
//---------------------------------------------------------------------------

