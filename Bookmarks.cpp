//---------------------------------------------------------------------------

#include <vcl.h>
#include <stdio.h>
#pragma hdrstop

#include "DForm.h"
#include "GotoWindow.h"
#include "Debug.h"
#include "Pages.h"
#include "EditAsc.h"
#include "Utility.h"
#include "Bookmarks.h"
#include "Globals.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TBkForm *BkForm;

static bool locked = false;
static bool ctrl = false;
static int ncols, nrows;

sb_entry *sb_list = NULL;
int sb_count;

//---------------------------------------------------------------------------
__fastcall TBkForm::TBkForm(TComponent* Owner)
        : TForm(Owner)
{
}

void realignRows() {

        locked = true;
        int w;

        if (BkForm == NULL) return;
        w = BkForm->Grid->Width;
        ncols = w / 115; if (ncols == 0) ncols = 1;
        nrows = (sb_count == 0) ? 1 : (sb_count-1) / ncols + 1;
        BkForm->Grid->RowCount = nrows;
        BkForm->Grid->ColCount = ncols;
        BkForm->Grid->DefaultColWidth = w / ncols;

        locked = false;
        BkForm->Grid->Repaint();

}

void updateSidebar() {

        char buf[64], *p;
        sb_entry *sbe, *sbe1, *sbe2, tsbe;
        int i, j;

        for (i=0; i<sb_count; i++) {
                sbe = &(sb_list[i]);
                if (sbe->name == NULL || sbe->type == ST_CODE || sbe->type == ST_DATA) {
                        if (sbe->name != NULL) free(sbe->name);
                        switch (sbe->type) {
                                case ST_SOURCE:
                                case ST_BINARY:
                                        p = strrchr(sbe->text, '\\');
                                        if (p == NULL) p = sbe->text; else p++;
                                        strncpy(buf, p, 63);
                                        break;
                                case ST_TSEARCH:
                                        strncpy(buf, sbe->text, 63);
                                        break;
                                case ST_HSEARCH:
                                        p = buf;
                                        for (j=0; j<sbe->len;j++) {
                                                if (j > 30) break;
                                                sprintf(p, "%02X", sbe->data[j]);
                                                p += 2;
                                        }
                                        *p = '\0';
                                        break;
                                case ST_CODE:
                                case ST_DATA:
                                case ST_PATCH:
                                        p = nameByAddress(sbe->address);
                                        if (p != NULL) {
                                                strncpy(buf, p, 63);
                                        } else {
                                                sprintf(buf, "%08X", sbe->address);
                                        }
                                        break;
                        }
                        buf[64] = '\0';
                        sbe->name = strdup(buf);
                }
        }

        // sorting items
        for (i=sb_count-1; i>0; i--) {
                for (j=0; j<i; j++) {
                        sbe1 = &(sb_list[j]);
                        sbe2 = &(sb_list[j+1]);
                        if (sbe1->type > sbe2->type || (sbe1->type == sbe2->type && stricmp(sbe1->name, sbe2->name) > 0)) {
                                memmove(&tsbe, sbe1, sizeof(sb_entry));
                                memmove(sbe1, sbe2, sizeof(sb_entry));
                                memmove(sbe2, &tsbe, sizeof(sb_entry));
                        }
                }
        }

        realignRows();

}

void emptySidebar() {

        if (sb_list != NULL) free(sb_list);
        sb_list = NULL;
        sb_count = 0;

}

bool entryExists(int type, unsigned int addr, char *text, unsigned char *data, int len) {

        sb_entry *sbe;
        int i;

        for (i=0; i<sb_count; i++) {
                sbe = &(sb_list[i]);
                if (sbe->type != type) continue;
                if (sbe->address != addr) continue;
                if (text != NULL && stricmp(sbe->text, text) != 0) continue;
                if (sbe->len != 0 && sbe->data != NULL && (sbe->len != len || memcmp(sbe->data, data, len) != 0)) continue;
                return true;
        }
        return false;

}

void addSideEntryFast(int type, unsigned int addr, char *text, unsigned char *data, int len) {

        sb_entry *sbe;

        if (entryExists(type, addr, text, data, len)) {
                WarningMessage("Entry already exists");
                return;
        }

        sb_count++;
        sb_list = (sb_entry *)realloc(sb_list, sb_count * sizeof(sb_entry));
        sbe = &(sb_list[sb_count-1]);
        sbe->name = NULL;
        sbe->type = type;
        sbe->address = addr;
        sbe->text = (text == NULL) ? NULL : strdup(text);
        if (data != NULL && len != 0) {
                sbe->data = (unsigned char *)malloc(len);
                memcpy(sbe->data, data, len);
        } else {
                sbe->data = NULL;
        }
        sbe->len = len;

}

void addSideEntry(int type, unsigned int addr, char *text, unsigned char *data, int len) {

        addSideEntryFast(type, addr, text, data, len);
        updateSidebar();

}


//---------------------------------------------------------------------------
void __fastcall TBkForm::Stayontop1Click(TObject *Sender)
{
        FormStyle = (FormStyle == fsStayOnTop) ? fsNormal : fsStayOnTop;
}
//---------------------------------------------------------------------------
void __fastcall TBkForm::NewSource1Click(TObject *Sender)
{
        if (OpenSrcDlg->Execute()) {
                char *name = OpenSrcDlg->FileName.c_str();
                addSideEntry(ST_SOURCE, 0, name, NULL, 0);
        }
}
//---------------------------------------------------------------------------
void __fastcall TBkForm::NewBinary1Click(TObject *Sender)
{
        if (OpenBinDlg->Execute()) {
                char *name = OpenBinDlg->FileName.c_str();
                addSideEntry(ST_BINARY, 0, name, NULL, 0);
        }
}
//---------------------------------------------------------------------------
void __fastcall TBkForm::NewCodepoint1Click(TObject *Sender)
{
        unsigned int a;

        if (! GotoForm->Invoke("Code point", "", &a)) return;
        addSideEntry(ST_CODE, a, NULL, NULL, 0);
}
//---------------------------------------------------------------------------
void __fastcall TBkForm::NewDatapoint1Click(TObject *Sender)
{
        unsigned int a;

        if (! GotoForm->Invoke("Data point", "", &a)) return;
        addSideEntry(ST_DATA, a, NULL, NULL, 0);
}
//---------------------------------------------------------------------------

void __fastcall TBkForm::GridDrawCell(TObject *Sender, int ACol, int ARow,
      TRect &Rect, TGridDrawState State)
{
        TCanvas *can;
        sb_entry *sbe;
        int n;
        TColor color;
        bool focus;

        if (locked) return;

        focus = State.Contains(gdFocused);
        n = ACol * nrows + ARow;
        color = (TColor)0x00E9FDFE;
        if (focus && n < sb_count) color = (TColor)0xe0e0e0;
        can = Grid->Canvas;
        can->Brush->Color = color;
        can->FillRect(Rect);

        if (n >= sb_count) return;
        sbe = &(sb_list[n]);
        TypeImages->Draw(can, Rect.Left+1, Rect.Top, sbe->type, true);
        can->Font->Color = focus ? (TColor)0x800000 : clBlack;
        can->Font->Style = TFontStyles();
        can->TextOut(Rect.Left+18, Rect.Top+1, sbe->name);

}
//---------------------------------------------------------------------------


void __fastcall TBkForm::FormResize(TObject *Sender)
{
        realignRows();
}
//---------------------------------------------------------------------------

void __fastcall TBkForm::MenuPopup(TObject *Sender)
{
        int n = Grid->Col * nrows + Grid->Row;
        if (n >= sb_count) {
                Open1->Visible = false;
                Open21->Visible = false;
                Delete1->Visible = false;
                return;
        }
        Delete1->Visible = true;
        switch(sb_list[n].type) {
                case ST_SOURCE:
                        Open1->Caption = "Open";
                        Open1->Visible = true;
                        Open21->Visible = false;
                        break;
                case ST_BINARY:
                        Open1->Caption = "Execute";
                        Open1->Visible = true;
                        Open21->Visible = false;
                        break;
                case ST_TSEARCH:
                case ST_HSEARCH:
                        Open1->Caption = "Search";
                        Open1->Visible = true;
                        Open21->Caption = "Search from start";
                        Open21->Visible = true;
                        break;
                case ST_CODE:
                case ST_DATA:
                        Open1->Caption = "Goto";
                        Open1->Visible = true;
                        Open21->Visible = false;
                        break;
                case ST_PATCH:
                        Open1->Caption = "Apply";
                        Open1->Visible = true;
                        Open21->Caption = "Revert";
                        Open21->Visible = true;
                        break;
        }
}
//---------------------------------------------------------------------------

void __fastcall TBkForm::Delete1Click(TObject *Sender)
{
        int n = Grid->Col * nrows + Grid->Row;
        if (n >= sb_count) return;
        if (n < sb_count-1) memmove(&(sb_list[n]), &(sb_list[n+1]), (sb_count-n-1) * sizeof(sb_entry));
        sb_count--;
        updateSidebar();
}
//---------------------------------------------------------------------------

void __fastcall TBkForm::Grid1DblClick(TObject *Sender)
{
        unsigned char what[256], mask[256];
        unsigned int addr_s, addr_e, res;
        memoryinfo mi;
        int i, n;

        if (locked) return;
        n = Grid->Col * nrows + Grid->Row;
        if (n >= sb_count) return;
        sb_entry *sbe = &(sb_list[n]);

        switch(sbe->type) {
                case ST_SOURCE:
                        openSource(sbe->text);
                        break;
                case ST_BINARY:
                        execBinary(sbe->text, false);
                        break;
                case ST_CODE:
                        cpSetAddress(sbe->address, true);
                        MainForm->Pages->ActivePage = MainForm->SheetCPU;
                        MainForm->vCPU->SetFocus();
                        break;
                case ST_DATA:
                        rpSetAddress(RPC, sbe->address, true);
                        RPC->page->SetFocus();
                        break;
                case ST_TSEARCH:
                case ST_HSEARCH:
                        if (sbe->type == ST_TSEARCH) {
                                n = strlen(sbe->text);
                                memcpy(what, sbe->text, n);
                                for (i=0; i<n; i++) {
                                        if (what[i] == '?') {
                                                mask[i] = what[i] = 0;
                                        } else {
                                                mask[i] = 0xff;
                                        }
                                }
                        } else {
                                n = sbe->len / 2;
                                memcpy(what, sbe->data, n);
                                memcpy(mask, sbe->data+n, n);
                        }
                        addr_s = rpGetAddress(RPC) + 1;
                        getMemoryInfo(addr_s, &mi);
                        if (mi.len == 0) return;
                        if (ctrl) addr_s = mi.addr;
                        addr_e = mi.addr + mi.len;
                        res = searchMemory(what, mask, n, addr_s, addr_e);
                        if (res != NONE) {
                                rpSetAddress(RPC, res, true);
                                RPC->page->SetFocus();
                        }
                        break;

        }

}
//---------------------------------------------------------------------------


void __fastcall TBkForm::NewTextSearch1Click(TObject *Sender)
{
        char buf[34];

        buf[0] = '\0';
        if (EditAscForm->Invoke("Text search", buf, false, 32) != NULL) {
                if (strlen(buf) == 0) return;
                addSideEntry(ST_TSEARCH, 0, buf, NULL, 0);
        }
}
//---------------------------------------------------------------------------




void __fastcall TBkForm::NewSearchhex1Click(TObject *Sender)
{
        char buf[65];
        unsigned char what[32];
        int i, n;
        unsigned char c1, c2;

        buf[0] = '\0';
        if (EditAscForm->Invoke("Hex search", buf, true, 64) != NULL) {
                n = strlen(buf)/2;
                if (n == 0 || n > 64) return;
                for (i=0; i<n; i++) {
                        c1 = buf[i*2] - 0x30; if (c1 > 9) c1 -= 7;
                        c2 = buf[i*2+1] - 0x30; if (c2 > 9) c2 -= 7;
                        what[i] = (c1 << 4) | c2;
                }
                addSideEntry(ST_HSEARCH, 0, NULL, what, n);
        }
}
//---------------------------------------------------------------------------

void __fastcall TBkForm::GridMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
        POINT pt;
        int row, col;

        ctrl = Shift.Contains(ssCtrl);
        Grid->MouseToCell(X, Y, col, row);
        if (col < 0 || row < 0 > col >= Grid->ColCount || row >= Grid->RowCount) return;
        Grid->Row = row;
        Grid->Col = col;

}
//---------------------------------------------------------------------------

void __fastcall TBkForm::Open1Click(TObject *Sender)
{
        ctrl = false;
        Grid1DblClick(Sender);
}
//---------------------------------------------------------------------------

void __fastcall TBkForm::Open21Click(TObject *Sender)
{
        ctrl = true;
        Grid1DblClick(Sender);
}
//---------------------------------------------------------------------------



