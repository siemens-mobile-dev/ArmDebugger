//---------------------------------------------------------------------------

#include <vcl.h>
#include <stdio.h>
#pragma hdrstop

#include "PatchesWindow.h"
#include "PatchWindow.h"
#include "Config.h"
#include "Debug.h"
#include "Wire.h"
#include "Pages.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TPatchesForm *PatchesForm;
patch **pp;
int np;
//---------------------------------------------------------------------------
__fastcall TPatchesForm::TPatchesForm(TComponent* Owner)
        : TForm(Owner)
{
        Enable1->ShortCut = ShortCut(VK_ADD, TShiftState());
        Disable1->ShortCut = ShortCut(VK_SUBTRACT, TShiftState());
}
//---------------------------------------------------------------------------
void TPatchesForm::reloadList() {

        int i, j, l, n;
        char buf1[256], buf2[256];

        n = np = getPatchCount();
        pp = getPatchList();

        List->RowCount = (n > 0) ? n + 1 : 2;
        List->ColCount = 6;
        List->Cells[0][0] = "St";
        List->Cells[1][0] = "Order";
        List->Cells[2][0] = "Address";
        List->Cells[3][0] = "Length";
        List->Cells[4][0] = "Old data";
        List->Cells[5][0] = "New data";
        List->Cells[0][1] = "";
        List->Cells[1][1] = "";
        List->Cells[2][1] = "";
        List->Cells[3][1] = "";
        List->Cells[4][1] = "";
        List->Cells[5][1] = "";

        List->ColWidths[0] = 15;
        List->ColWidths[1] = 40;
        List->ColWidths[2] = 60;
        List->ColWidths[3] = 60;
        List->ColWidths[4] = 210;
        List->ColWidths[5] = 210;
        ClientWidth = 565;
        ClientHeight = 350;

        for (i=0; i<n; i++) {
                List->Cells[0][i+1] = pp[i]->disabled ? "X" : "";
                List->Cells[1][i+1] = IntToStr(i+1);
                List->Cells[2][i+1] = IntToHex((int)(pp[i]->address), 8);
                List->Cells[3][i+1] = IntToStr(pp[i]->length);
                l = (pp[i]->length > 64) ? 64 : pp[i]->length;
                for (j=0; j<l; j++) {
                        sprintf(buf1+j*3, "%02X ", pp[i]->olddata[j]);
                        sprintf(buf2+j*3, "%02X ", pp[i]->newdata[j]);
                }
                if (l < pp[i]->length) {
                        strcat(buf1, "...");
                        strcat(buf2, "...");
                }
                List->Cells[4][i+1] = buf1;
                List->Cells[5][i+1] = buf2;
        }

        Repaint();

}
//---------------------------------------------------------------------------
void __fastcall TPatchesForm::Invoke() {

        reloadList();
        Visible = true;
        BringToFront();
        SetFocus();

}


void __fastcall TPatchesForm::Close1Click(TObject *Sender)
{
        Visible = false;
}
//---------------------------------------------------------------------------

void __fastcall TPatchesForm::Goto1Click(TObject *Sender)
{
        if (np == 0) return;
        patch *p = pp[List->Row-1];
        rpSetAddress(RPC, p->address, true);
}
//---------------------------------------------------------------------------

void __fastcall TPatchesForm::ListDblClick(TObject *Sender)
{
        if (np == 0) return;
        patch *p = pp[List->Row-1];
        rpSetAddress(RPC, p->address, true);
}
//---------------------------------------------------------------------------

void __fastcall TPatchesForm::Edit1Click(TObject *Sender)
{
        if (np == 0) return;
        patch *p = pp[List->Row-1];
        if (PatchForm->Invoke(p->address, p->newdata, p->length) == mrOk) {
                undoPatch(p->address);
                if (is_online()) setMemory(PatchForm->Address, PatchForm->Data, PatchForm->Length, SM_ISOLATED);
                reloadList();
        }
}
//---------------------------------------------------------------------------

void __fastcall TPatchesForm::Undo1Click(TObject *Sender)
{
        int i, n, top, bottom;
        unsigned int *addr;

        if (np == 0) return;
        top = List->Selection.Top;
        bottom = List->Selection.Bottom;
        n = bottom-top+1;

        addr = (unsigned int *)malloc(n * sizeof(unsigned int *));
        for (i=0; i<n; i++) {
                patch *p = pp[top+i-1];
                addr[i] = p->address;
        }
        for (i=0; i<n; i++) {
                undoPatch(addr[i]);
        }
        free(addr);
        reloadList();
        reload_dumps();
}
//---------------------------------------------------------------------------

void saveToVkp(struct patch *p, FILE *F) {

        unsigned int a;
        unsigned char *olddata, *newdata;
        char buf1[64], buf2[64];
        int i, l, n;

        a = p->address;
        l = p->length;
        olddata = p->olddata;
        newdata = p->newdata;
        while (l > 0) {
                n = 16 - (a & 0xf);
                if (n > l) n = l;
                for (i=0; i<n; i++) {
                        sprintf(buf1+i*2, "%02X", olddata[i]);
                        sprintf(buf2+i*2, "%02X", newdata[i]);
                }
                fprintf(F, "%08X: %s %s\n", a, buf1, buf2);
                a += n;
                olddata += n;
                newdata += n;
                l -= n;
        }
}

void __fastcall TPatchesForm::Saveasvkp1Click(TObject *Sender)
{
        int i, top, bottom;
        AnsiString as;
        char *name;
        FILE *F;

        if (np == 0) return;
        if (! SaveVkpDlg->Execute()) {
                return;
        }

        top = List->Selection.Top;
        bottom = List->Selection.Bottom;

        as = SaveVkpDlg->FileName;
        name = as.c_str();
        F = fopen(name, "w");
        if (F == NULL) {
                MessageBox(NULL, "Cannot open vkp file", "Error", MB_OK|MB_ICONERROR);
                return;
        }

        for (i=top; i<=bottom; i++) {
                patch *p = pp[i-1];
                saveToVkp(p, F);
        }

        fclose(F);
}
//---------------------------------------------------------------------------

void __fastcall TPatchesForm::Saveallasvkp1Click(TObject *Sender)
{
        AnsiString as;
        char *name;
        FILE *F;
        int i;

        if (np == 0) return;
        if (! SaveVkpDlg->Execute()) {
                return;
        }

        as = SaveVkpDlg->FileName;
        name = as.c_str();
        F = fopen(name, "w");
        if (F == NULL) {
                MessageBox(NULL, "Cannot open vkp file", "Error", MB_OK|MB_ICONERROR);
                return;
        }

        for (i=0; i<np; i++) {
                saveToVkp(pp[i], F);
        }

        fclose(F);
}
//---------------------------------------------------------------------------


void __fastcall TPatchesForm::Decreaseorder1Click(TObject *Sender)
{
        patch *x;
        int n;

        n = List->Row;
        if (n <= 1) return;

        x = pp[n-1];
        pp[n-1] = pp[n-2];
        pp[n-2] = x;
        List->Row = List->Row - 1;
        reloadList();
}
//---------------------------------------------------------------------------

void __fastcall TPatchesForm::Increaseorder1Click(TObject *Sender)
{
        patch *x;
        int n;

        n = List->Row;
        if (n >= np) return;

        x = pp[n];
        pp[n] = pp[n-1];
        pp[n-1] = x;
        List->Row = List->Row + 1;
        reloadList();
}
//---------------------------------------------------------------------------

void __fastcall TPatchesForm::Enable1Click(TObject *Sender)
{
        int i, n, top, bottom;
        unsigned int *addr;

        if (np == 0) return;
        top = List->Selection.Top;
        bottom = List->Selection.Bottom;
        n = bottom-top+1;

        for (i=0; i<n; i++) {
                patch *p = pp[top+i-1];
                enablePatch(p);
        }
        reloadList();
        reload_dumps();

}
//---------------------------------------------------------------------------

void __fastcall TPatchesForm::Disable1Click(TObject *Sender)
{
        int i, n, top, bottom;
        unsigned int *addr;

        if (np == 0) return;
        top = List->Selection.Top;
        bottom = List->Selection.Bottom;
        n = bottom-top+1;

        for (i=0; i<n; i++) {
                patch *p = pp[top+i-1];
                disablePatch(p);
        }
        reloadList();
        reload_dumps();

}
//---------------------------------------------------------------------------

void __fastcall TPatchesForm::EnableAll1Click(TObject *Sender)
{
        int i;

        for (i=0; i<np; i++) {
                enablePatch(pp[i]);
        }
        reloadList();
        reload_dumps();
}
//---------------------------------------------------------------------------

void __fastcall TPatchesForm::DisableAll1Click(TObject *Sender)
{
        int i;

        for (i=0; i<np; i++) {
                disablePatch(pp[i]);
        }
        reloadList();
        reload_dumps();
}
//---------------------------------------------------------------------------

