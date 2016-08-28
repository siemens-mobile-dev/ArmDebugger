//---------------------------------------------------------------------------

#include <vcl.h>
#include <stdio.h>
#pragma hdrstop

#include "DForm.h"
#include "SearchResults.h"
#include "SearchProgressWin.h"
#include "Debug.h"
#include "Pages.h"
#include "Utility.h"
#include "Globals.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TSearchResForm *SearchResForm;

static char HEXDIGITS[16] = "0123456789ABCDEF";

static unsigned int RegionStart;
static unsigned int RegionLength;
static unsigned char ucWhat[256];
static unsigned char ucMask[256];
static int Size;
static int IsHex;

static resdata *AList;
static int ACount;

//---------------------------------------------------------------------------
__fastcall TSearchResForm::TSearchResForm(TComponent* Owner)
        : TForm(Owner)
{
        Grid->ColWidths[1] = 250;
}
//---------------------------------------------------------------------------
void TSearchResForm::Invoke(unsigned int r_s, unsigned int r_l,
                                unsigned char *what, unsigned char *mask,
                                int size,
                                unsigned int *alist, int acount,
                                int ishex) {

        int i;

        Dismiss();

        RegionStart = r_s;
        RegionLength = r_l;
        memcpy(ucWhat, what, size);
        memcpy(ucMask, mask, size);
        Size = size;
        IsHex = ishex;

        AList = (resdata *)malloc(acount * sizeof(resdata));
        ACount = acount;

        for (i=0; i<acount; i++) {
                AList[i].addr = alist[i];
                AList[i].data = NULL;
        }

        Grid->Visible = false;
        Show();
        SetFocus();
        Timer1->Enabled = true;

}

void __fastcall TSearchResForm::Timer1Timer(TObject *Sender)
{
        Timer1->Enabled = false;
        ReloadMemory();
        Grid->Visible = true;
        Grid->SetFocus();
}

bool isAscii(unsigned char c) {

        return (c >= 32 && c <= 127);

}

bool isUcs(wchar_t c) {

        return (c >= 32 && c <= 0x06ff);

}

wchar_t *display(unsigned char *s, int l) {

        static wchar_t text[1024];
        int i, p;

        if (IsHex == 2) {

                for (i=0; i<l; i++) {
                        p = i*3;
                        text[p] = HEXDIGITS[s[i] >> 4];
                        text[p+1] = HEXDIGITS[s[i] & 0x0f];
                        text[p+2] = ' ';
                }
                text[l*3] = 0;

        } else if (IsHex == 0) {

                for (i=0; i<l; i++) text[i] = s[i];
                text[l] = 0;

        } else {

                for (i=0; i<l/2; i++) text[i] = s[i*2] | (s[i*2+1] << 8);
                text[l/2] = 0;

        }

        return text;

}

bool matches(unsigned char *p) {

        int i;

        for (i=0; i<Size; i++) {
                if ((p[i] & ucMask[i]) != ucWhat[i]) return false;
        }
        return true;

}

void TSearchResForm::ReloadMemory() {

        unsigned int addr, len, pl, sl, j, jj;
        int i;
        unsigned char buf[1024], *data;

        Grid->Cursor = crHourGlass;

        for (i=0; i<ACount; i++) {
                if (AList[i].data != NULL) free(AList[i].data);
                AList[i].data = NULL;
        }

        for (i=0; i<ACount; i++) {

                pl = AList[i].addr - RegionStart;
                if (IsHex == 2) {
                        if (pl > 4) pl = 4;
                        sl = 4;
                } else {
                        pl &= ~1;
                        if (pl > 10) pl = 10;
                        sl = 10;
                }
                addr = AList[i].addr - pl;
                len = pl + Size + sl;

                if (! readMemory(addr, buf, len)) {
                        WarningMessage("Error reading memory");
                        break;
                }

                if (IsHex == 2) {
                        data = AList[i].data = (unsigned char *)malloc(2+len);
                        data[0] = len;
                        data[1] = pl;
                        memcpy(data+2, buf, len);
                } else if (IsHex == 0) {
                        j = pl;
                        while (j > 0) { if (!isAscii(buf[j-1])) break; j--; }
                        jj = pl+Size;
                        while (jj < len) { if (!isAscii(buf[jj])) break; jj++; }
                        len = jj-j;
                        data = AList[i].data = (unsigned char *)malloc(2+len);
                        data[0] = len;
                        data[1] = pl-j;
                        memcpy(data+2, buf+j, len);
                } else {
                        j = pl;
                        while (j > 0) { if (!isUcs(buf[j-2] | (buf[j-1] << 8))) break; j-=2; }
                        jj = pl+Size;
                        while (jj < len) { if (!isUcs(buf[jj] | (buf[jj+1] << 8))) break; jj+=2; }
                        len = jj-j;
                        data = AList[i].data = (unsigned char *)malloc(2+len);
                        data[0] = len;
                        data[1] = pl-j;
                        memcpy(data+2, buf+j, len);
                }
        }

        Grid->RowCount = (ACount == 0) ? 2 : ACount+1;

        Grid->Cursor = crDefault;
        Grid->Repaint();
}
//---------------------------------------------------------------------------
void TSearchResForm::Dismiss() {

        int i;

        if (AList != NULL) {
                for (i=0; i<ACount; i++) {
                        if (AList[i].data != NULL) free(AList[i].data);
                }
                free(AList);
        }
        AList = NULL;
        ACount = 0;
        Hide();

}
//---------------------------------------------------------------------------

void __fastcall TSearchResForm::FormCloseQuery(TObject *Sender,
      bool &CanClose)
{
        Dismiss();
}
//---------------------------------------------------------------------------

void __fastcall TSearchResForm::GridDrawCell(TObject *Sender, int acol,
      int arow, TRect &rect, TGridDrawState state)
{
        TCanvas *can;
        TColor bg;
        resdata *al;
        unsigned int addr, len, pl;
        unsigned char *data;
        wchar_t *text;
        char tbuf[16];
        int p;

        can = Grid->Canvas;
        if (arow == 0) {
                can->TextOut(rect.Left+2, rect.Top+1, (acol == 0) ? "Address" : "Data");
                return;
        }

        can->Brush->Color = bg = state.Contains(gdSelected) ? SEL_COLOR : (TColor)0xffffff;
        can->Font->Color = (TColor)0;
        can->FillRect(rect);
        if (arow > ACount) return;

        if (acol == 0) {
                addr = AList[arow-1].addr;
                sprintf(tbuf, "%08X", addr);
                can->TextOut(rect.Left+2, rect.Top+1, tbuf);
        } else {
                al = &(AList[arow-1]);
                if (al == NULL) return;
                p = rect.Left+2;
                len = al->data[0];
                pl = al->data[1];
                data = al->data+2;
                text = display(data, pl);
                can->TextOut(p, rect.top+1, text);
                p += can->TextWidth(text);
                if (matches(data+pl)) can->Brush->Color = (TColor)0x80ffff;
                text = display(data+pl, Size);
                can->TextOut(p, rect.top+1, text);
                p += can->TextWidth(text);
                can->Brush->Color = bg;
                text = display(data+pl+Size, len-(pl+Size));
                can->TextOut(p, rect.top+1, text);
        }

}
//---------------------------------------------------------------------------

void __fastcall TSearchResForm::Refresh1Click(TObject *Sender)
{
        ReloadMemory();
}
//---------------------------------------------------------------------------

void __fastcall TSearchResForm::View1Click(TObject *Sender)
{
        int arow = Grid->Row;
        if (arow == 0 || arow > ACount) return;
        rpSetAddress(RPC, AList[arow-1].addr, true);
}
//---------------------------------------------------------------------------

void __fastcall TSearchResForm::ViewasCode1Click(TObject *Sender)
{
        int arow = Grid->Row;
        if (arow == 0 || arow > ACount) return;
        cpSetAddress(AList[arow-1].addr, true);
}
//---------------------------------------------------------------------------

void __fastcall TSearchResForm::GridDblClick(TObject *Sender)
{
        if (ctrlState() == 0) {
                View1Click(Sender);
        } else {
                ViewasCode1Click(Sender);
        }
}
//---------------------------------------------------------------------------

void __fastcall TSearchResForm::RestartSearch1Click(TObject *Sender)
{
        SearchProgress->InvokeAgain();
        unsigned int *alist = SearchProgress->getResults();
        int acount = SearchProgress->getResultsCount();
        Invoke(RegionStart, RegionLength, ucWhat, ucMask, Size, alist, acount, IsHex);
        SearchProgress->freeResults();

}
//---------------------------------------------------------------------------

void __fastcall TSearchResForm::Removefromlist1Click(TObject *Sender)
{
        int i, n;

        int arow = Grid->Row;
        if (arow == 0 || arow > ACount) return;
        n = arow - 1;
        free(AList[n].data);
        for (i=n+1; i<ACount; i++) {
                AList[i-1].addr = AList[i].addr;
                AList[i-1].data = AList[i].data;
        }
        ACount--;
        Grid->RowCount = (ACount == 0) ? 2 : ACount+1;
        Grid->Repaint();

}
//---------------------------------------------------------------------------

