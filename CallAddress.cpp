//---------------------------------------------------------------------------

#include <vcl.h>
#include <stdio.h>
#include "DForm.h"
#include "Debug.h"
#include "Config.h"
#include "Pages.h"
#include "EventLog.h"
#include "Wire.h"
#include "Utility.h"
#pragma hdrstop

#include "CallAddress.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TCallForm *CallForm;

callrec *crList = NULL;
int crCount = 0;

//---------------------------------------------------------------------------
__fastcall TCallForm::TCallForm(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void addCallRecord(unsigned int addr, bool thumb, unsigned int r[8]) {

        callrec *cr;
        int i, j;

        for (i=0; i<crCount; i++) {
                cr = &(crList[i]);
                if (cr->addr != addr) continue;
                if (cr->thumb != thumb) continue;
                for (j=0; j<6; j++) {
                        if (cr->r[j] != r[j]) break;
                }
                if (j < 6) continue;
                return;
        }

        i = crCount++;
        crList = (callrec *)realloc(crList, crCount * sizeof(callrec));
        cr = &(crList[i]);
        cr->addr = addr;
        cr->thumb = thumb;
        for (j=0; j<6; j++) {
                cr->r[j] = r[j];
        }

        CallForm->LastCalled->Count = crCount;
        CallForm->LastCalled->ItemIndex = i;
        CallForm->Repaint();

}

void __fastcall TCallForm::bCallClick(TObject *Sender)
{
        unsigned int addr, eaddr, r[16], cpsr, buf[8];
        char tbuf[128];
        char *p;
        bool res;

        if (! is_online()) {
                WarningMessage("Not connected");
                return;
        }

        eRes->Text = "";
        eRes->Repaint();

        addr = strtoul(eAddr->Text.c_str(), &p, 16);
        eaddr = addr;
        if (cbThumb->Checked) eaddr = (eaddr & ~1) + 1;
        r[0] = strtoul(eR0->Text.c_str(), &p, 16);
        r[1] = strtoul(eR1->Text.c_str(), &p, 16);
        r[2] = strtoul(eR2->Text.c_str(), &p, 16);
        r[3] = strtoul(eR3->Text.c_str(), &p, 16);
        r[4] = strtoul(eSP0->Text.c_str(), &p, 16);
        r[5] = strtoul(eSP4->Text.c_str(), &p, 16);

        addCallRecord(addr, cbThumb->Checked, r);

        buf[0] = 0xe24dd008;    // sub sp, sp, #8
        buf[1] = 0xe58d4000;    // str r4, [sp]
        buf[2] = 0xe58d5004;    // str r5, [sp,#4]
        buf[3] = 0xe59ff000;    // ldr pc, [pc]
        buf[5] = cbThumb->Checked ? (addr | 1) : addr;
        if (! ExtraFeatures) {
                res = execute((unsigned char *)buf, 48, r, 6, cpsr, ExecTimeout);
        } else {
                res = execute2((unsigned char *)buf, 48, r, 6, cpsr, ExecTimeout);
        }

//        res = armCall(eaddr, r, 8, cpsr, ExecTimeout);

        if (res) {

                eRes->Font->Color = 0x008000;
                eRes->Text = IntToHex((int)r[0], 8);

                sprintf(tbuf, "Call %08X: R0=%08X R1=%08X R2=%08X R3=%08X", addr, r[0], r[1], r[2], r[3]);
                newEvent('>', tbuf);
                setRegisters(r, cpsr, 16, NULL);

        } else {

                eRes->Font->Color = 0x0000ff;
                eRes->Text = "Error";

        }

        reload_dumps();

}
//---------------------------------------------------------------------------
void __fastcall TCallForm::LastCalledData(TWinControl *Control, int Index,
      AnsiString &Data)
{
        char buf[256];

        unsigned int a = crList[Index].addr;
        unsigned int r0 = crList[Index].r[0];
        unsigned int r1 = crList[Index].r[1];
        unsigned char *s = nameByAddress(a);
        if (s == NULL) {
                sprintf(buf, "%08X (%X, %X...)", a, r0, r1);
        } else {
                sprintf(buf, "%s (%X, %X...)", s, r0, r1);
        }

        Data = buf;
}
//---------------------------------------------------------------------------

void __fastcall TCallForm::LastCalledClick(TObject *Sender)
{
        if (LastCalled->ItemIndex == -1) return;
        callrec *cr = &(crList[LastCalled->ItemIndex]);
        eAddr->Text = IntToHex((int)cr->addr, 8);
        cbThumb->Checked = cr->thumb;
        eR0->Text = IntToHex((int)cr->r[0], 1);
        eR1->Text = IntToHex((int)cr->r[1], 1);
        eR2->Text = IntToHex((int)cr->r[2], 1);
        eR3->Text = IntToHex((int)cr->r[3], 1);
        eSP0->Text = IntToHex((int)cr->r[4], 1);
        eSP4->Text = IntToHex((int)cr->r[5], 1);
        eRes->Text = "";

}

//---------------------------------------------------------------------------

void __fastcall TCallForm::LastCalledDblClick(TObject *Sender)
{
        bCallClick(Sender);
}
//---------------------------------------------------------------------------

void __fastcall TCallForm::LastCalledMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
        if (Button != mbLeft) return;
        LastCalledClick(Sender);
}
//---------------------------------------------------------------------------

void __fastcall TCallForm::FormKeyPress(TObject *Sender, char &Key)
{
        if (Key == 0x1b) CallForm->Visible = false;
}
//---------------------------------------------------------------------------

