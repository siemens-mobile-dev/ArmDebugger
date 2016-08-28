//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "About.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TAboutForm *AboutForm;
char *URL = "http://chaos.allsiemens.com";
//---------------------------------------------------------------------------
__fastcall TAboutForm::TAboutForm(TComponent* Owner)
        : TForm(Owner)
{
        Label4->Caption = URL;
}
//---------------------------------------------------------------------------
void __fastcall TAboutForm::Label4Click(TObject *Sender)
{
        ShellExecute(0, NULL, URL, NULL, NULL, SW_SHOWNORMAL);
}
//---------------------------------------------------------------------------

