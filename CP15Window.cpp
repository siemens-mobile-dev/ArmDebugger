//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "CP15Window.h"
#include "Config.h"
#include "Debug.h"
#include "Wire.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TCP15Form *CP15Form;

struct cpline {

        int CRn;
        int CRm;
        int op2;
        unsigned int value;
        char *descr;

} cpdata[] = {

        { 0, 0, 0,      0, "ID Code"},
        { 0, 0, 1,      0, "Cache type"},
        { 0, 0, 2,      0, "TCM status"},

        { 1, 0, 0,      0, "Control Register"},

        { 2, 0, 0,      0, "Translation Table Base"},

        { 3, 0, 0,      0, "Domain Access Control Register"},

        { 5, 0, 0,      0, "Data Fault Status Register"},
        { 5, 0, 1,      0, "Instruction Fault Status Register"},

        { 6, 0, 0,      0, "Fault Address Register"},

        { 9, 0, 0,      0, "DCache Lockdown Register"},
        { 9, 0, 1,      0, "ICache Lockdown Register"},

        { 9, 1, 0,      0, "Data TCM Region Register"},
        { 9, 1, 1,      0, "Instruction TCM Region Register"},

        { 10, 0, 0,     0, "TLB Lockdown Register"},

        { 13, 0, 0,     0, "FCSE PID"},
        { 13, 0, 1,     0, "Context ID"},

        { 15, 0, 0,     0, "Debug Override Register"},
        { 15, 1, 0,     0, "Debug and Test Address"},

};


//---------------------------------------------------------------------------
__fastcall TCP15Form::TCP15Form(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------

bool __fastcall TCP15Form::reload() {

        int i, j, n;
        unsigned int v;
        unsigned int proc[16];
        unsigned int r[16], cpsr;
        char buf[64];

        n = sizeof(cpdata) / sizeof(struct cpline);
        i = 0;

        while (n > 0) {
                int nn = (n > 8) ? 8 : n;

                memset(proc, 0, sizeof(proc));
                for (j=0; j<nn; j++) {
                        proc[j] = 0xee100f10 | (cpdata[i+j].CRn << 16) | (cpdata[i+j].CRm) | (cpdata[i+j].op2 << 5) | (j << 12);
                }
                proc[15] = 0xe12fff1e;   // bx lr

                if (! execute((unsigned char *)proc, sizeof(proc), r, 0, cpsr, 1000))
                        return false;

                for (j=0; j<nn; j++) {
                        cpdata[i+j].value = r[j];
                }

                i += nn;
                n -= nn;
        }

        n = sizeof(cpdata) / sizeof(struct cpline);

        for (i=0; i<n; i++) {
                List->Cells[0][i+2] = IntToStr(cpdata[i].CRn);
                List->Cells[1][i+2] = IntToStr(cpdata[i].CRm);
                List->Cells[2][i+2] = IntToStr(cpdata[i].op2);

                v = cpdata[i].value;
                List->Cells[3][i+2] = IntToHex((int)v, 8);
                strcpy(buf, "                                         ");
                for (j=0; j<32; j++) {
                        buf[(j*5)/4+1] = ((v << j) & 0x80000000) == 0 ? '0' : '1';
                }
                List->Cells[4][i+2] = buf;
                List->Cells[5][i+2] = cpdata[i].descr;
        }

        return true;

}


void __fastcall TCP15Form::Invoke() {

        int n;

        n = sizeof(cpdata) / sizeof(struct cpline);

        List->RowCount = n + 2;
        List->ColCount = 6;
        List->Cells[0][1] = "CRn";
        List->Cells[1][1] = "CRm";
        List->Cells[2][1] = "op2";
        List->Cells[3][1] = " Hex";
        List->Cells[4][0] = " 3322 2222 2222 1111 1111 11            ";
        List->Cells[4][1] = " 1098 7654 3210 9876 5432 1098 7654 3210";
        List->Cells[5][1] = " Description";

        List->ColWidths[0] = 28;
        List->ColWidths[1] = 28;
        List->ColWidths[2] = 28;
        List->ColWidths[3] = 60;
        List->ColWidths[4] = 224;
        List->ColWidths[5] = 174;
        CP15Form->ClientWidth = 551;
        CP15Form->ClientHeight = (List->DefaultRowHeight) * (n + 2) + 5;
        if (reload()) CP15Form->Show();

}

void __fastcall TCP15Form::Refresh1Click(TObject *Sender)
{
        if (reload()) {
                List->Repaint();
        } else {
                CP15Form->Hide();
        }
}
//---------------------------------------------------------------------------
