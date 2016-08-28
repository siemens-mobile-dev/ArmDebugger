//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "GotoWindow.h"
#include "Debug.h"
#include "Globals.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TGotoForm *GotoForm;
char **AdrSaveList = NULL;
//---------------------------------------------------------------------------
__fastcall TGotoForm::TGotoForm(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------
bool TGotoForm::Invoke(char *title, char *buf, unsigned int *res) {

        AnsiString as;
        unsigned int addr;
        char *p, *pp, **sp;

        if (AdrSaveList != NULL) {
                for (sp = AdrSaveList; *sp != NULL; sp++) {
                        GotoForm->AddressEdit->Items->Add(*sp);
                }
                AdrSaveList = NULL;
        }

        Caption = title;
        ActiveControl = AddressEdit;
        if (buf != NULL) {
                AddressEdit->Text = buf;
        }

        if (ShowModal() == mrOk) {
                as = GotoForm->AddressEdit->Text;
                p = as.c_str();
                addr = addressForText(p);
                if (addr == NONE) return false;
                *res = addr;
                return true;
        } else {
                return false;
        }

}
//---------------------------------------------------------------------------
void __fastcall TGotoForm::FormShow(TObject *Sender)
{
        AddressEdit->SelStart = 2;
        AddressEdit->SelLength = 0;

}
//---------------------------------------------------------------------------

void __fastcall TGotoForm::FormHide(TObject *Sender)
{
        AnsiString as = AddressEdit->Text;
        int n = AddressEdit->Items->Count;
        for (int i=0; i<n; i++) {
                if (as == AddressEdit->Items->Strings[i]) return;
        }
        AddressEdit->Items->Insert(0, AddressEdit->Text);
}
//---------------------------------------------------------------------------


