//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "EditAsc.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TEditAscForm *EditAscForm;
bool isnum;
//---------------------------------------------------------------------------
__fastcall TEditAscForm::TEditAscForm(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------
char *TEditAscForm::Invoke(char *title, char *buf, bool isNumber, int maxlen) {

        AnsiString as;

        Caption = title;
        eLine->MaxLength = maxlen;
        eLine->CharCase = isNumber ? ecUpperCase : ecNormal;
        isnum = isNumber;
        if (buf != NULL) {
                eLine->Text = buf;
        } else {
                eLine->Text = "";
        }
        eLine->SelectAll();

        if (ShowModal() == mrOk) {
                as = eLine->Text;
                strcpy(buf, as.c_str());
                return buf;
        } else {
                return NULL;
        }

}

void __fastcall TEditAscForm::eLineChange(TObject *Sender)
{
        static char last[256];
        int i, l, s;
        AnsiString as = eLine->Text;
        char *p = as.c_str();

        if (! isnum) return;
        if (strcmp(p, last) == 0) return;

        l = strlen(p);
        s = eLine->SelStart;
        for (i=0; i<l; i++) {
                if (! isxdigit(p[i])) {
                        if (s > i) s--;
                        eLine->Text = last;
                        eLine->SelStart = s;
                        return;
                }
        }
        strcpy(last, p);
        for (i=0; i<l; i++) {
                if (last[i] > 'F') last[i] -= 0x20;
        }
        eLine->Text = last;
        eLine->SelStart = s;

}
//---------------------------------------------------------------------------

