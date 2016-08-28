//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Search.h"
#include "DForm.h"
#include "Pages.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TSearchForm *SearchForm;
//---------------------------------------------------------------------------
__fastcall TSearchForm::TSearchForm(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------
char *strfind(char *s1, char *s2, bool icase) {

        register char *p;
        register int len;

        if (icase) {
                p = s1;
                len = strlen(s2);
                while (*p) {
                        if (strnicmp(p, s2, len) == 0) {
                                return p;
                        }
                        p++;
                }
                return NULL;
        } else {
                return strstr(s1, s2);
        }
}


void __fastcall TSearchForm::Open() {

        Visible = true;
        BringToFront();
        What->SetFocus();

}
//---------------------------------------------------------------------------
void __fastcall TSearchForm::NextSearch(TObject *Sender)
{

        int i, x, y;
        size_t j;
        char *q, *s, *p;
        AnsiString as, as1;
        TPoint cp;
        bool icase;

        TRichEdit *page = APC->page;

        as = What->Text.c_str();
        q = as.c_str();

        icase = ! cbCase->Checked;

        x = page->CaretPos.x;
        y = page->CaretPos.y;
        int nlines = page->Lines->Count;

        if (rbForward->Checked) {

                for (i=y; i<nlines; i++) {

                        as1 = page->Lines->Strings[i];
                        s = as1.c_str();
                        j = (i == y) ? x+1 : 0;
                        if ((p = strfind(s+j, q, icase)) != NULL) {
                                x = p - s;
                                cp.x = x;
                                cp.y = i;
                                APC->page->CaretPos = cp;
                                APC->page->SelLength = strlen(q);
                                APC->page->SetFocus();
                                return;
                        }
                }

        } else {

                for (i=y; i>=0; i--) {

                        s = page->Lines->Strings[i].c_str();
                        j = (i == y) ? x : 99999;
                        if ((p = strfind(s, q, icase)) != NULL && (p-s)+strlen(q) < j) {
                                x = p - s;
                                cp.x = x;
                                cp.y = i;
                                APC->page->CaretPos = cp;
                                APC->page->SelLength = strlen(q);
                                APC->page->SetFocus();
                                return;
                        }
                }

        }
}
//---------------------------------------------------------------------------
void __fastcall TSearchForm::Button2Click(TObject *Sender)
{
        Hide();
}
//---------------------------------------------------------------------------
