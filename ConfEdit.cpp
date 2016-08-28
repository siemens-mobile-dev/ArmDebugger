//---------------------------------------------------------------------------

#include <vcl.h>
#include <stdio.h>
#pragma hdrstop

#include "ConfEdit.h"
#include "Config.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TConfigForm *ConfigForm;
//---------------------------------------------------------------------------
__fastcall TConfigForm::TConfigForm(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void updateFontButtons() {

        char buf[256];

        sprintf(buf, "%s, %i", ConfigForm->EditorFontDlg->Font->Name, ConfigForm->EditorFontDlg->Font->Size);
        ConfigForm->bEditFont->Caption = buf;
        sprintf(buf, "%s, %i", ConfigForm->DebuggerFontDlg->Font->Name, ConfigForm->DebuggerFontDlg->Font->Size);
        ConfigForm->bDbgFont->Caption = buf;

}


void TConfigForm::Invoke() {

        char buf[256];

        cbMethod->ItemIndex = AccessMethod;
        cbSpeed->Text = itoa(Baudrate, buf, 10);
        eMonIntv->Text = itoa(MonitorInterval, buf, 10);
        eExecTm->Text = itoa(ExecTimeout, buf, 10);

        EditorFontDlg->Font = EditorFont;
        DebuggerFontDlg->Font = DebuggerFont;
        updateFontButtons();

        ckEvents->Checked = TraceEvents;
        ckDebug->Checked = TraceDebug;
        ckWire->Checked = TraceWire;
        ckComm->Checked = TraceComm;

        eArmPath->Text = KeilArmPath;
        eAsmArgs->Text = AsmArgs;
        eCompArgs->Text = CompArgs;
        eLinkArgs->Text = LinkArgs;

        if (ShowModal() == mrOk) {

                AccessMethod = cbMethod->ItemIndex;
                Baudrate = atoi(cbSpeed->Text.c_str());
                MonitorInterval = atoi(eMonIntv->Text.c_str());
                ExecTimeout = atoi(eExecTm->Text.c_str());

                EditorFont = EditorFontDlg->Font;
                DebuggerFont = DebuggerFontDlg->Font;

                TraceEvents = ckEvents->Checked ? 1 : 0;
                TraceDebug = ckDebug->Checked ? 1 : 0;
                TraceWire = ckWire->Checked ? 1 : 0;
                TraceComm = ckComm->Checked ? 1 : 0;

                KeilArmPath = strdup(eArmPath->Text.c_str());
                AsmArgs = strdup(eAsmArgs->Text.c_str());
                CompArgs = strdup(eCompArgs->Text.c_str());
                LinkArgs = strdup(eLinkArgs->Text.c_str());

                applyConfig();
                saveConfig();

        }

}

void __fastcall TConfigForm::bEditFontClick(TObject *Sender)
{
        EditorFontDlg->Execute();
        updateFontButtons();
}
//---------------------------------------------------------------------------

void __fastcall TConfigForm::bDbgFontClick(TObject *Sender)
{
        DebuggerFontDlg->Execute();
        updateFontButtons();
}
//---------------------------------------------------------------------------

