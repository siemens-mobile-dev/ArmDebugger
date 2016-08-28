//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "SaveProgressWin.h"
#include "DForm.h"
#include "Wire.h"
#include "Debug.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TSaveProgress *SaveProgress;

static FILE *F = NULL;
static unsigned int Address = 0;
static unsigned int InitialLength = 0;
static unsigned int Length = 0;
static unsigned int Pos = 0;
static bool CancelQuery = false;

//---------------------------------------------------------------------------
__fastcall TSaveProgress::TSaveProgress(TComponent* Owner)
        : TForm(Owner)
{
}

void TSaveProgress::initiate(FILE *f, int addr, int len) {

        F = f;
        Address = addr;
        InitialLength = len;
        Length = InitialLength = len;
        Pos = 0;
        CancelQuery = false;

        Label->Caption = " ";
        PBar->Max = InitialLength;
        PBar->Position = 0;
        Timer1->Enabled = true;
        MainForm->Enabled = false;
        Show();

}
//---------------------------------------------------------------------------
void __fastcall TSaveProgress::Button1Click(TObject *Sender)
{
        CancelQuery = true;
}
//---------------------------------------------------------------------------
void __fastcall TSaveProgress::FormCloseQuery(TObject *Sender,
      bool &CanClose)
{
        CancelQuery = true;
        CanClose = false;
}
//---------------------------------------------------------------------------
void __fastcall TSaveProgress::Timer1Timer(TObject *Sender)
{
        unsigned int l;
        static char buf[16384];
        AnsiString s;

        if (CancelQuery) {
                if (F) fclose(F);
                F = NULL;
                Timer1->Enabled = false;
                MainForm->Enabled = true;
                Hide();
                return;
        }

        l = (Length > 1024) ? 1024 : Length;
        if (! readMemory(Address, buf, l)) {
                Label->Caption = "Lost connection";
                Timer1->Interval = 1000;
                return;
        }

        fwrite(buf, 1, l, F);
        Length -= l;
        Pos += l;
        Address += l;
        PBar->Position = Pos;
        s.printf("%u / %u bytes saved", Pos, InitialLength);
        Label->Caption = s;
        Timer1->Interval = 1;

        if (Length == 0) {
                CancelQuery = true;
        }

}
//---------------------------------------------------------------------------
