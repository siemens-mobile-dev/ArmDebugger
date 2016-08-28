//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Debug.h"
#include "Pages.h"
#include "Utility.h"
#include "NewWatch.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TNewWatchForm *NewWatchForm;
//---------------------------------------------------------------------------
__fastcall TNewWatchForm::TNewWatchForm(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------

bool TNewWatchForm::validate() {

        AnsiString as;
        unsigned int a, l, t;

        as = eAddress->Text;
        a = strtoul(as.c_str(), NULL, 16);
        if (a == 0 || ! inSpace(a)) {
                return WarningMessage("Address not valid");
        }
        as = eLength->Text;
        l = strtoul(as.c_str(), NULL, 10);
        if (l == 0) {
                return WarningMessage("Length/count not valid");
        }
        t = rType->ItemIndex;
        if ((t == 0 && l > 64) || (t == 1 && l > 32) || (t == 2 && l > 16) ||
            (t == 3 && l > 64) || (t == 4 && l > 32))
        {
                return WarningMessage("Length too large (max 64 bytes)");
        }
        return true;
}

void ui_seteu(TEdit *te, TLabel *l, bool en) {

        te->Visible = en;
        l->Visible = en;

}

void TNewWatchForm::ui_sync() {
        int t = cbRef->ItemIndex;
        ui_seteu(eOff1, lOff1, t > 0);
        ui_seteu(eOff2, lOff2, t > 1);
        ui_seteu(eOff3, lOff3, t > 2);
        ui_seteu(eOff4, lOff4, t > 3);
}

void TNewWatchForm::Invoke(watchitem *wi) {

        AnsiString as;
        int t, l;

        if (wi != NULL) {
                rType->ItemIndex = t = wi->Type;
                l = wi->Length;
                if (t == WTYPE_HALFWORD) l /= 2;
                if (t == WTYPE_WORD) l /= 4;
                if (t == WTYPE_UNICODE) l /= 2;
                eLength->Text = IntToStr(l);
                cbRef->ItemIndex = wi->Ref;
                eAddress->Text = IntToHex((int)wi->Address, 8);
                eOff1->Text = IntToHex((int)wi->Offset[0], 4);
                eOff2->Text = IntToHex((int)wi->Offset[1], 4);
                eOff3->Text = IntToHex((int)wi->Offset[2], 4);
                eOff4->Text = IntToHex((int)wi->Offset[3], 4);
        } else {
                rType->ItemIndex = 2;
                eLength->Text = "1";
                cbRef->ItemIndex = 0;
                eAddress->Text = "";
                eOff1->Text = "";
                eOff2->Text = "";
                eOff3->Text = "";
                eOff4->Text = "";
        }

        ActiveControl = eAddress;
        while (true) {
                ui_sync();
                if (ShowModal() != mrOk) return;
                if (validate()) break;
        }

        if (wi == NULL) {
                WP.count++;
                WP.items = (struct watchitem_t *)realloc(WP.items, WP.count * sizeof(struct watchitem_t));
                wi = &(WP.items[WP.count-1]);
                wi->first = true;
                MainForm->WatchesList->RowCount = WP.count;
        }
        t = wi->Type = rType->ItemIndex;
        as = eAddress->Text;
        wi->Address = strtoul(as.c_str(), NULL, 16);
        as = eLength->Text;
        l = strtoul(as.c_str(), NULL, 10);
        if (t == WTYPE_HALFWORD) l *= 2;
        if (t == WTYPE_WORD) l *= 4;
        if (t == WTYPE_UNICODE) l *= 2;
        if (l > 64) l = 64;
        wi->Length = l;
        as = eOff1->Text;
        wi->Offset[0] = strtoul(as.c_str(), NULL, 16);
        as = eOff2->Text;
        wi->Offset[1] = strtoul(as.c_str(), NULL, 16);
        as = eOff3->Text;
        wi->Offset[2] = strtoul(as.c_str(), NULL, 16);
        as = eOff4->Text;
        wi->Offset[3] = strtoul(as.c_str(), NULL, 16);
        wi->Ref = cbRef->ItemIndex;

        watches_reload();
}


void __fastcall TNewWatchForm::cbRefChange(TObject *Sender)
{
        ui_sync();
}
//---------------------------------------------------------------------------

void __fastcall TNewWatchForm::rTypeClick(TObject *Sender)
{
        switch (rType->ItemIndex) {
                case 0: eLength->Text = "1"; break;
                case 1: eLength->Text = "1"; break;
                case 2: eLength->Text = "1"; break;
                case 3: eLength->Text = "64"; break;
                case 4: eLength->Text = "32"; break;
        }
}
//---------------------------------------------------------------------------

