//---------------------------------------------------------------------------

#include <vcl.h>
#include <stdio.h>
#pragma hdrstop

#include "PatchWindow.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TPatchForm *PatchForm;

static bool inEdit;
static unsigned char editValue;

//---------------------------------------------------------------------------
__fastcall TPatchForm::TPatchForm(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------

int __fastcall TPatchForm::Invoke(unsigned int addr, unsigned char *data, int len) {

        char tbuf[256];

        Address = addr;
        inEdit = false;
        if (data == NULL) {
                Length = 0;
                Dump->RowCount = 1;
        } else {
                memcpy(Data, data, len);
                Length = len;
        }

        Dump->ColWidths[3] = 22;
        Dump->ColWidths[7] = 22;
        Dump->ColWidths[11] = 22;

        sprintf(tbuf, "Enter patch data at %08X:", addr);
        Label1->Caption = tbuf;
        Dump->Col = 0;
        Dump->Row = 0;
        return ShowModal();

}

int __fastcall TPatchForm::Invoke(unsigned int addr) {

        return Invoke(addr, NULL, 0);

}

//---------------------------------------------------------------------------

void __fastcall TPatchForm::DumpDrawCell(TObject *Sender, int ACol,
      int ARow, TRect &Rect, TGridDrawState State)
{
        static char *digits = "0123456789ABCDEF";

        TCanvas *can;
        char buf[4];
        unsigned char c;
        unsigned int offset;

        can = Dump->Canvas;
        can->Brush->Color = State.Contains(gdSelected) ? (TColor)0xe8e8e8 : (TColor)0xffffff;
        can->Font->Color = (TColor)0;
        can->FillRect(Rect);

        offset = ARow * 16 + ACol;

        if (inEdit && State.Contains(gdSelected)) {
                buf[0] = digits[editValue];
                buf[1] = '\0';
                can->TextOut(Rect.left+1, Rect.top, buf);
                return;
        }

        if (offset >= Length) return;

        c = Data[offset];
        buf[0] = digits[c >> 4];
        buf[1] = digits[c & 0x0f];
        buf[2] = '\0';
        can->TextOut(Rect.left+1, Rect.top, buf);

}

//---------------------------------------------------------------------------


void __fastcall TPatchForm::DumpSelectCell(TObject *Sender, int ACol,
      int ARow, bool &CanSelect)
{
        unsigned int offset = ARow * 16 + ACol;
        inEdit = false;
        if (offset > Length) {
                CanSelect = false;
                Dump->Col = 0;
                Dump->Row = Length / 16;
                Dump->Col = Length % 16;
        }
}
//---------------------------------------------------------------------------
void __fastcall TPatchForm::DumpKeyPress(TObject *Sender, char &Key)
{
        unsigned char buf[16];
        unsigned int addr, eaddr, offset;
        char key = Key;

        offset = Dump->Row * 16 + Dump->Col;

        if (key == 0x08) {
                if (inEdit) {
                        inEdit = false;
                } else {
                        if (Dump->Col == 0 && Dump->Row > 0) {
                                Dump->Row--;
                                Dump->Col = 15;
                        } else if (Dump->Col > 0) {
                                Dump->Col--;
                        }

                        if (offset == Length && Length > 0) {
                                Length--;
                                Dump->RowCount = Length / 16 + 1;
                        }
                }
        }

        if (key >= 'a' && key <= 'f') key -= 0x20;
        if ((key >= '0' && key <= '9') || (key >= 'A' && key <= 'F')) {
                key -= 0x30;
                if (key > 9) key -= 7;

                // hex editing
                if (! inEdit) {
                        inEdit = true;
                        editValue = key;
                        Dump->Repaint();
                } else {
                        inEdit = false;

                        Data[offset] = (editValue << 4) + key;
                        if (offset == Length) {
                                Length++;
                                Dump->RowCount = Length / 16 + 1;
                        }

                        if (Dump->Col < 15) {
                                Dump->Col++;
                        } else {
                                Dump->Col = 0;
                                Dump->Row++;
                        }
                }
        }

}
//---------------------------------------------------------------------------

