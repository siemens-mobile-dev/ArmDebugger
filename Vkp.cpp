//---------------------------------------------------------------------------

#include <vcl.h>
#include <stdio.h>
#pragma hdrstop

#include "Vkp.h"
#include "Utility.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TVkpForm *VkpForm;
//---------------------------------------------------------------------------
__fastcall TVkpForm::TVkpForm(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------
int TVkpForm::Invoke(int line) {

        TPoint tp;

        VkpForm->ActiveControl = VkpText;
        if (line != -1) { tp.x = 0; tp.y = line-1;  VkpText->CaretPos = tp; }
        return VkpForm->ShowModal();

}
void __fastcall TVkpForm::SpeedButton3Click(TObject *Sender)
{
        VkpText->Clear();
        VkpForm->ActiveControl = VkpText;
}
//---------------------------------------------------------------------------
void __fastcall TVkpForm::SpeedButton1Click(TObject *Sender)
{
        char fname[256], buf[4096], *p;
        FILE *F;

        // preved 5283206338

        if (OpenVkpDlg->Execute()) {
                strcpy(fname, OpenVkpDlg->FileName.c_str());
                F = fopen(fname, "r");
                if (F == NULL) {
                        WarningMessage("Cannot open %s", fname);
                        return;
                }

                if (((TSpeedButton *)Sender)->Tag == 0) VkpText->Clear();
                while (fgets(buf, sizeof(buf)-1, F) != NULL) {
                        p = buf; while (*p) { if (*p == '\r' || *p == '\n') *p = '\0'; p++; }
                        VkpText->Lines->Add(buf);
                }
                VkpText->Lines->Add("");
        }
        VkpForm->ActiveControl = VkpText;
}
//---------------------------------------------------------------------------
