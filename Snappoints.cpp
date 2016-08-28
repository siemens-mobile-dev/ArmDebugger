//---------------------------------------------------------------------------

#include <vcl.h>
#include <stdio.h>
#pragma hdrstop

#include "Debug.h"
#include "Pages.h"
#include "Wire.h"
#include "Watchpoint.h"
#include "Snappoints.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TSnapsForm *SnapsForm;

int n = 0;
int snum[16];
unsigned int saddr[16];
bool sthumb[16];

//---------------------------------------------------------------------------
__fastcall TSnapsForm::TSnapsForm(TComponent* Owner)
        : TForm(Owner)
{
        SnapGrid->ColWidths[0] = 20;
        SnapGrid->ColWidths[1] = 75;
        SnapGrid->ColWidths[2] = 90;
        SnapGrid->ColWidths[3] = 1000;
        SnapGrid->Cells[0][0] = " N";
        SnapGrid->Cells[1][0] = " Address";
        SnapGrid->Cells[2][0] = " Condition";
        SnapGrid->Cells[3][0] = " Text";
}
//---------------------------------------------------------------------------
void TSnapsForm::Invoke() {

        Visible = true;
        updateSnaps();
        BringToFront();

}

void TSnapsForm::updateSnaps() {

        snap **sn;
        char buf[4096], c, *p, *pp;
        int i;

        if (! Visible) return;

        SnapGrid->RowCount = 2;
        SnapGrid->Cells[0][1] = "";
        SnapGrid->Cells[1][1] = "";
        SnapGrid->Cells[2][1] = "";
        SnapGrid->Cells[3][1] = "";

        n = 0;
        sn = getSnapList();
        for (i=1; i<16; i++) {

                if (sn[i] == NULL) continue;
                n++;
                SnapGrid->RowCount = n+1;
                SnapGrid->Cells[0][n] = i;
                sprintf(buf, " %08X %c", sn[i]->addr, sn[i]->thumb ? 'T' : ' ');
                SnapGrid->Cells[1][n] = buf;
                SnapGrid->Cells[2][n] = sn[i]->condition;
                p = sn[i]->text;
                pp = buf;
                *(pp++) = ' ';
                while (*p && pp-buf < sizeof(buf)-5) {
                        c = *(p++);
                        switch (c) {
                                case '\t': *(pp++) = ' '; break;
                                case '\r': break;
                                case '\n': strcpy(pp, " | "); pp+=3; break;
                                default: *(pp++) = c; break;
                        }
                }
                *pp = 0;
                SnapGrid->Cells[3][n] = buf;
                snum[n] = i;
                saddr[n] = sn[i]->addr;
                sthumb[n] = sn[i]->thumb;
        }
}

void __fastcall TSnapsForm::Goto1Click(TObject *Sender)
{
        int i = SnapGrid->Row;
        if (i > n) return;
        cpSetAddress(saddr[i], true);
}
//---------------------------------------------------------------------------
void __fastcall TSnapsForm::Edit1Click(TObject *Sender)
{
        if (! is_online()) return;
        int i = SnapGrid->Row;
        if (i > n) return;
        WptForm->Invoke(saddr[i], sthumb[i], false);
}
//---------------------------------------------------------------------------

void __fastcall TSnapsForm::Remove1Click(TObject *Sender)
{
        int i = SnapGrid->Row;
        if (i > n) return;
        deleteSnap(snum[i]);
        updateSnaps();
        reload_dumps();
}
//---------------------------------------------------------------------------

void __fastcall TSnapsForm::Removeall1Click(TObject *Sender)
{
        int i;

        for (i=1; i<=n; i++) {
                deleteSnap(snum[i]);
        }
        updateSnaps();
        reload_dumps();
}
//---------------------------------------------------------------------------

void __fastcall TSnapsForm::close1Click(TObject *Sender)
{
        Visible = false;        
}
//---------------------------------------------------------------------------

