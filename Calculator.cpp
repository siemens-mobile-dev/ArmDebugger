//---------------------------------------------------------------------------

#include <vcl.h>
#include <stdio.h>
#include <stdlib.h>
#pragma hdrstop

#include "Calculator.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TCalcForm *CalcForm;
static bool locked = false;

static unsigned int A = 0;
static unsigned int B = 0;
static int sign = 0;
static int vchange = 0;
//---------------------------------------------------------------------------
__fastcall TCalcForm::TCalcForm(TComponent* Owner)
        : TForm(Owner)
{
        TShiftState sss = TShiftState();
        TShiftState sssh = TShiftState() << ssShift;

        mPlus->ShortCut = ShortCut(VK_ADD, sss);
        mPlus2->ShortCut = ShortCut(187, sssh);
        mMinus->ShortCut = ShortCut(VK_SUBTRACT, sss);
        mMinus2->ShortCut = ShortCut(189, sss);
        mMult->ShortCut = ShortCut(VK_MULTIPLY, sss);
        mMul2->ShortCut = ShortCut('8', sssh);
        mDiv->ShortCut = ShortCut(VK_DIVIDE, sss);
        mDiv2->ShortCut = ShortCut(191, sss);
        mEqual->ShortCut = ShortCut(VK_RETURN, sss);
        mEqual2->ShortCut = ShortCut(187, sss);
        mAnd->ShortCut = ShortCut('7', sssh);
        mOr->ShortCut = ShortCut(220, sssh);
        mXor->ShortCut = ShortCut('6', sssh);
        mNot->ShortCut = ShortCut(192, sssh);
        mLSH->ShortCut = ShortCut(188, sssh);
        mRSH->ShortCut = ShortCut(190, sssh);
}

//---------------------------------------------------------------------------
void TCalcForm::Invoke() {
        Visible = true;
        BringToFront();
        SetFocus();
}

//---------------------------------------------------------------------------
char *filter(TEdit *e, int type) {

        static char buf[256];
        char c;
        int r;

        AnsiString as = e->Text;
        int s = e->SelStart;
        strcpy(buf, as.c_str());

        switch(type) {
                case 0:
                case 1:
                        if (s == 0) break;
                        c = buf[s-1];
                        r = (type == 0) ? isdigit(c) : isxdigit(c);
                        if (! r) {
                                e->SelStart = s-1;
                                e->SelLength = 1;
                                e->ClearSelection();
                        } else {
                                vchange = 1;
                        }
                        break;

                case 2:
                        if (s > 0) {
                                c = buf[s-1];
                                e->SelStart = --s;
                                if (c != '0' && c != '1') {
                                } else if (s > 31) {
                                        if (buf[0] != '1') {
                                                e->SelStart = 0;
                                        }
                                } else {
                                        e->SelStart = ++s;
                                        vchange = 1;
                                }
                                e->SelLength = 1;
                                e->ClearSelection();
                                e->SelStart = s;
                        }
                        as = e->Text;
                        strcpy(buf, as.c_str());
                        int l = strlen(buf);
                        while (l < 32) {
                                e->SelStart = 0;
                                e->SetSelTextBuf("0");
                                l++;
                                s++;
                        }
                        e->SelStart = s;
                        break;
        }

        as = e->Text;
        strcpy(buf, as.c_str());
        return buf;
}

void set_A(unsigned int n, TEdit *cur) {

        char buf[64];

        A = n;
        sprintf(buf, "%u", n);
        if (cur != CalcForm->eDec) CalcForm->eDec->Text = buf;
        sprintf(buf, "%X", n);
        if (cur != CalcForm->eHex) CalcForm->eHex->Text = buf;
        for (int i=0; i<32; i++) buf[i] = ((n >> (31-i)) & 1) | 0x30;
        buf[32] = 0;
        if (cur != CalcForm->eBin) CalcForm->eBin->Text = buf;

        if (cur == NULL) {
                TEdit *te = (TEdit *)(CalcForm->ActiveControl);
                te->SelectAll();
        }
}

void set_sign(int n) {

        char buf[4];

        sign = n;
        if (n == 0) n = ' ';
        buf[0] = n;
        buf[1] = buf[2] = 0;
        if (n == '&') buf[1] = n;
        CalcForm->lSign->Caption = buf;

        TEdit *te = (TEdit *)(CalcForm->ActiveControl);
        te->SelectAll();
}
//---------------------------------------------------------------------------

void __fastcall TCalcForm::eDecChange(TObject *Sender)
{
        if (locked) return;
        locked = true;

        char *s = filter(eDec, 0);
        set_A(strtoul(s, NULL, 10), eDec);

        locked = false;
}
//---------------------------------------------------------------------------

void __fastcall TCalcForm::eHexChange(TObject *Sender)
{
        if (locked) return;
        locked = true;

        char *s = filter(eHex, 1);
        set_A(strtoul(s, NULL, 16), eHex);

        locked = false;
}
//---------------------------------------------------------------------------

void __fastcall TCalcForm::eBinChange(TObject *Sender)
{
        if (locked) return;
        locked = true;

        char *s = filter(eBin, 2);
        set_A(strtoul(s, NULL, 2), eBin);

        locked = false;
}
//---------------------------------------------------------------------------

void calc() {

        if (sign == 0) return;
        switch (sign) {
                case '+':
                        set_A(B+A, NULL);
                        break;
                case '-':
                        set_A(B-A, NULL);
                        break;
                case '*':
                        set_A(B*A, NULL);
                        break;
                case '/':
                        if (A == 0) {
                                MessageBox(NULL, "Division by zero", "Error", MB_OK|MB_ICONWARNING);
                                set_sign(0);
                                break;
                        }
                        set_A(B/A, NULL);
                        break;
                case '&':
                        set_A(B&A, NULL);
                        break;
                case '|':
                        set_A(B|A, NULL);
                        break;
                case '^':
                        set_A(B^A, NULL);
                        break;
                case '<':
                        set_A(B<<A, NULL);
                        break;
                case '>':
                        set_A(B>>A, NULL);
                        break;
        }
        set_sign(0);

}

void op(int n) {

        if (vchange) calc();
        B = A;
        set_sign(n);
        vchange = 0;

}

//---------------------------------------------------------------------------

void __fastcall TCalcForm::Clear1Click(TObject *Sender)
{
        set_A(0, NULL);
        B = 0;
        set_sign(0);
}
//---------------------------------------------------------------------------

void __fastcall TCalcForm::mPlusClick(TObject *Sender)
{
        op('+');
}
//---------------------------------------------------------------------------

void __fastcall TCalcForm::mMinusClick(TObject *Sender)
{
        op('-');
}
//---------------------------------------------------------------------------

void __fastcall TCalcForm::mEqualClick(TObject *Sender)
{
        calc();
}
//---------------------------------------------------------------------------

void __fastcall TCalcForm::mDivClick(TObject *Sender)
{
        op('/');
}
//---------------------------------------------------------------------------

void __fastcall TCalcForm::mMultClick(TObject *Sender)
{
        op('*');
}
//---------------------------------------------------------------------------

void __fastcall TCalcForm::mAndClick(TObject *Sender)
{
        op('&');        
}
//---------------------------------------------------------------------------

void __fastcall TCalcForm::mOrClick(TObject *Sender)
{
        op('|');
}
//---------------------------------------------------------------------------

void __fastcall TCalcForm::mXorClick(TObject *Sender)
{
        op('^');
}
//---------------------------------------------------------------------------

void __fastcall TCalcForm::mLSHClick(TObject *Sender)
{
        op('<');
}
//---------------------------------------------------------------------------

void __fastcall TCalcForm::mRSHClick(TObject *Sender)
{
        op('>');
}
//---------------------------------------------------------------------------

void __fastcall TCalcForm::mNotClick(TObject *Sender)
{
        set_A(~A, NULL);
        set_sign(0);
}
//---------------------------------------------------------------------------

void __fastcall TCalcForm::Copy1Click(TObject *Sender)
{
        HGLOBAL H;

        TEdit *te = (TEdit *)(CalcForm->ActiveControl);
        AnsiString as = te->Text;
        if (! OpenClipboard(NULL)) return;
        H = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, 64);
        if (H != NULL) {
                char *p = (char *)GlobalLock(H);
                strcpy(p, as.c_str());
                GlobalUnlock(H);
                EmptyClipboard();
                if (! SetClipboardData(CF_TEXT, H)) {
                        GlobalFree(H);
                }
        }
        CloseClipboard();
}
//---------------------------------------------------------------------------

void __fastcall TCalcForm::Paste1Click(TObject *Sender)
{
        HGLOBAL H;

        if (! OpenClipboard(NULL)) return;
        H = GetClipboardData(CF_TEXT);
        if (H == NULL) {
                CloseClipboard();
                return;
        }
        char *p = (char *)GlobalLock(H);
        if (p == NULL) {
                CloseClipboard();
                return;
        }
        TEdit *te = (TEdit *)(CalcForm->ActiveControl);
        te->Text = p;
        GlobalUnlock(H);
        CloseClipboard();
}
//---------------------------------------------------------------------------


void __fastcall TCalcForm::Stayontop1Click(TObject *Sender)
{
        FormStyle = (FormStyle == fsStayOnTop) ? fsNormal : fsStayOnTop;
}
//---------------------------------------------------------------------------

