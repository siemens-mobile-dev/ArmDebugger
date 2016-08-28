//---------------------------------------------------------------------------

#include <vcl.h>
#include <stdio.h>
#include "Pages.h"
#include "Debug.h"
#include "NewWatch.h"
#include "EditAsc.h"
#include "Watches.h"

#pragma hdrstop
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TWatchesForm *WatchesForm;
//---------------------------------------------------------------------------
__fastcall TWatchesForm::TWatchesForm(TComponent* Owner)
        : TForm(Owner)
{
}

//---------------------------------------------------------------------------

void __fastcall TWatchesForm::FormCreate(TObject *Sender)
{
        WP.page = WatchesForm->List;
        WP.Visible = false;
        WP.Sticker = WatchesForm->MonStickerW;
        WP.page->ColWidths[1] = 1500;

}

//---------------------------------------------------------------------------

void TWatchesForm::Invoke() {

        Visible = true;
        BringToFront();
        SetFocus();

}

//---------------------------------------------------------------------------

void __fastcall TWatchesForm::FormHide(TObject *Sender)
{
        WP.Visible = false;
}
//---------------------------------------------------------------------------

void __fastcall TWatchesForm::FormShow(TObject *Sender)
{
        WP.Visible = true;
        watches_reload();
}
//---------------------------------------------------------------------------

void __fastcall TWatchesForm::Refresh1Click(TObject *Sender)
{
        watches_reload();
}
//---------------------------------------------------------------------------

void __fastcall TWatchesForm::Monitor1Click(TObject *Sender)
{
        if (WP.Monitor) {
                WP.Monitor = false;
                MonStickerW->Visible = false;
        } else {
                WP.Monitor = true;
                MonStickerW->Visible = true;
        }
}

//---------------------------------------------------------------------------

void __fastcall TWatchesForm::View1Click(TObject *Sender)
{
        int arow = WP.page->Row;
        if (arow >= WP.count) return;
        watchitem *wi = &(WP.items[arow]);
        rpSetAddress(RPC, wi->Address, true);
}

//---------------------------------------------------------------------------

void __fastcall TWatchesForm::Hide1Click(TObject *Sender)
{
        Visible = false;
}

//---------------------------------------------------------------------------

void __fastcall TWatchesForm::Stayontop1Click(TObject *Sender)
{
        FormStyle = (FormStyle == fsStayOnTop) ? fsNormal : fsStayOnTop;
}

//---------------------------------------------------------------------------

void __fastcall TWatchesForm::Newwatch1Click(TObject *Sender)
{
        NewWatchForm->Invoke(NULL);
}

//---------------------------------------------------------------------------

void __fastcall TWatchesForm::ListDrawCell(TObject *Sender, int ACol,
      int ARow, TRect &Rect, TGridDrawState State)
{
        drawWPcell(&WP, ACol, ARow, Rect, State);
}

//---------------------------------------------------------------------------

void __fastcall TWatchesForm::ChangeWatch1Click(TObject *Sender)
{
        if (WP.count == 0) return;
        int n = WP.page->Row;
        NewWatchForm->Invoke(&(WP.items[n]));
}

//---------------------------------------------------------------------------

void __fastcall TWatchesForm::Delete1Click(TObject *Sender)
{
        if (WP.count == 0) return;
        int n = WP.page->Row;
        int w = sizeof(struct watchitem_t);
        WP.count--;
        if (n < WP.count) memmove(&(WP.items[n]), &(WP.items[n+1]), (WP.count-n)*w);
        WP.page->RowCount = WP.count;
        watches_reload();

}

//---------------------------------------------------------------------------

void __fastcall TWatchesForm::Edit1Click(TObject *Sender)
{
        unsigned char buf[256];
        unsigned char xdata[16];
        unsigned char xb;
        unsigned short xs;
        unsigned int xi;

        if (WP.count == 0) return;
        int n = WP.page->Row;
        watchitem *wi = &(WP.items[n]);
        unsigned int a = wi->realAddress;
        switch (wi->Type) {
                case 0:
                        xb = *((unsigned char *)(wi->data));
                        sprintf(buf, "%02X", xb);
                        if (EditAscForm->Invoke("Edit value", buf, true, 2) != NULL) {
                                *xdata = strtoul(buf, NULL, 16);
                                setMemory(a, xdata, 1, SM_ISOLATED);
                        }
                        break;
                case 1:
                        xs = *((unsigned short *)(wi->data));
                        sprintf(buf, "%04X", xs);
                        if (EditAscForm->Invoke("Edit value", buf, true, 4) != NULL) {
                                *((unsigned short *)xdata) = strtoul(buf, NULL, 16);
                                setMemory(a, xdata, 2, SM_ISOLATED);
                        }
                        break;
                case 2:
                        xi = *((unsigned int *)(wi->data));
                        sprintf(buf, "%08X", xi);
                        if (EditAscForm->Invoke("Edit value", buf, true, 8) != NULL) {
                                *((unsigned int *)xdata) = strtoul(buf, NULL, 16);
                                setMemory(a, xdata, 4, SM_ISOLATED);
                        }
                        break;
                case 3:
                        int sb = sizeof(buf);
                        readMemory(wi->realAddress, buf, sb);
                        buf[sb-1] = 0;
                        if (EditAscForm->Invoke("Edit value", buf, false, sb) != NULL) {
                                setMemory(a, buf, strlen(buf)+1, SM_STICKY);
                        }
                        break;
        }
        reload_dumps();
}

//---------------------------------------------------------------------------

