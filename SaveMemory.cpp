//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "SaveMemory.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TSaveMem *SaveMem;
//---------------------------------------------------------------------------
__fastcall TSaveMem::TSaveMem(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------

void __fastcall TSaveMem::Button3Click(TObject *Sender)
{
        if (SaveMemoryDlg->Execute()) {
                eFilename->Text = SaveMemoryDlg->FileName;
                Button1->Enabled = (eFilename->Text.Length() != 0);
        }

}
//---------------------------------------------------------------------------


void __fastcall TSaveMem::FormShow(TObject *Sender)
{
        eFilename->Text = "";
        eFilename->SetFocus();
        Button1->Enabled = false;
}
//---------------------------------------------------------------------------
void __fastcall TSaveMem::eFilenameChange(TObject *Sender)
{
        Button1->Enabled = (eFilename->Text.Length() != 0);
}
//---------------------------------------------------------------------------
