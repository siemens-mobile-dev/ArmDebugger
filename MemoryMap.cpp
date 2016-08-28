//---------------------------------------------------------------------------

#include <vcl.h>
#include "Debug.h"
#include "Wire.h"
#include "Pages.h"
#pragma hdrstop

#include "MemoryMap.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TMMapView *MMapView;

static char *TYPES[] = { "Unused", "Coarse", "Section", "Fine" };
static char *TYPES2[] = { "Unused", "Large", "Small", "Tiny" };
static char *PERMS[] = { "S", "NA", "RO", "RW" };
static char *CB[] = { "00", "01", "10", "11" };

static unsigned int L1TT[4096];
static unsigned int L2TT[4096];
static int items[4096];
static int nitems;
bool L2ready = false;

//---------------------------------------------------------------------------
__fastcall TMMapView::TMMapView(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void TMMapView::Invoke() {

        if (! is_online()) return;

        pReading->Visible = true;
        pReading->BringToFront();
        Visible = true;
        BringToFront();
        Timer1->Enabled = true;
}

void __fastcall TMMapView::Timer1Timer(TObject *Sender)
{

        unsigned int addr, d, type, physical;
        int i;
        char *s;
        AnsiString asz = "";

        Timer1->Enabled = false;

        if (! readMemory(TransTableAddr, (unsigned char *)L1TT, 16384)) {
                Visible = false;
                return;
        }

        L2ready = false;
        nitems = 0;
        for (i=0; i<4096; i++) {
                if ((L1TT[i] & 3) != 0) {
                        items[nitems++] = i;
                }
        }

        L1grid->ColCount = 7;
        L1grid->RowCount = nitems+1;
        L1grid->ColWidths[0] = 70;
        L1grid->ColWidths[1] = 75;
        L1grid->ColWidths[2] = 70;
        L1grid->ColWidths[3] = 55;
        L1grid->ColWidths[4] = 30;
        L1grid->ColWidths[5] = 30;
        L1grid->ColWidths[6] = 150;
        L1grid->Cells[0][0] = " MVA";
        L1grid->Cells[1][0] = " Type";
        L1grid->Cells[2][0] = " Physical";
        L1grid->Cells[3][0] = " Domain";
        L1grid->Cells[4][0] = " AP";
        L1grid->Cells[5][0] = " CB";
        L1grid->Cells[6][0] = " Description";


        L2grid->ColCount = 8;
        L2grid->RowCount = 2;
        L2grid->ColWidths[0] = 70;
        L2grid->ColWidths[1] = 75;
        L2grid->ColWidths[2] = 70;
        L2grid->ColWidths[3] = 30;
        L2grid->ColWidths[4] = 30;
        L2grid->ColWidths[5] = 30;
        L2grid->ColWidths[6] = 30;
        L2grid->ColWidths[7] = 150;
        L2grid->Cells[0][0] = " MVA";
        L2grid->Cells[1][0] = " Type";
        L2grid->Cells[2][0] = " Physical";
        L2grid->Cells[3][0] = " AP0";
        L2grid->Cells[4][0] = " AP1";
        L2grid->Cells[5][0] = " AP2";
        L2grid->Cells[6][0] = " AP3";
        L2grid->Cells[7][0] = " CB";

        for (i=0; i<nitems; i++) {

                d = L1TT[items[i]];
                type = d & 3;
                addr = items[i] * 1024 * 1024;
                physical = d & ~0xfffff;
                if (type == 1) physical = d & ~0x3ff;
                if (type == 3) physical = d & ~0xfff;


                L1grid->Cells[0][i+1] = IntToHex((int)addr, 8);
                L1grid->Cells[1][i+1] = TYPES[type];
                L1grid->Cells[2][i+1] = (type != 0) ? IntToHex((int)physical, 8) : asz;
                L1grid->Cells[3][i+1] = (type != 0) ? IntToStr((d >> 5) & 15) : asz;
                L1grid->Cells[4][i+1] = (type == 2) ? PERMS[(d >> 10) & 3] : "";
                L1grid->Cells[5][i+1] = (type == 2) ? CB[(d >> 2) & 3] : "";

                if (FlashSize != 0 && addr >= FlashAddress && addr <= FlashAddress+FlashSize-1) {
                        s = (physical == addr) ? "Flash" : "Flash (relocated)";
                } else if (BootROMsize != 0 && addr >= BootROMaddress && addr <= BootROMaddress+BootROMsize-1) {
                        s = "Boot ROM";
                } else if (IntRAM1size != 0 && addr >= IntRAM1address && addr <= IntRAM1address+IntRAM1size-1) {
                        s = "Internal RAM";
                } else if (IntRAM2size != 0 && addr >= IntRAM2address && addr <= IntRAM2address+IntRAM2size-1) {
                        s = "Internal RAM";
                } else if (IntRAM3size != 0 && addr >= IntRAM3address && addr <= IntRAM3address+IntRAM3size-1) {
                        s = "Internal RAM";
                } else if (ExtRAMsize != 0 && addr >= ExtRAMaddress && addr <= ExtRAMaddress+ExtRAMsize-1) {
                        s = "External RAM";
                } else if (IOsize != 0 && addr >= IOaddress && addr <= IOaddress+IOsize-1) {
                        s = "I/O ports";
                } else if (FlashSize != 0 && addr >= ShadowFlashAddress && addr <= ShadowFlashAddress+FlashSize-1) {
                        s = "Shadow flash";
                } else if (ExtRAMsize != 0 && addr >= ShadowRAMaddress && addr <= ShadowRAMaddress+ExtRAMsize-1) {
                        s = "Shadow ERAM";
                } else {
                        s = "";
                }
                L1grid->Cells[6][i+1] = s;
        }

        pReading->Visible = false;
        Repaint();
}


//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

void __fastcall TMMapView::L1gridSelectCell(TObject *Sender, int ACol,
      int ARow, bool &CanSelect)
{
        static int lastcell = -1;
        unsigned int type;
        int i;

        i = ARow - 1;
        type = L1TT[items[i]] & 3;
        if (i == lastcell) return;
        lastcell = i;

        L2grid->Visible = false;
        Panel2->Caption = "";
        Timer2->Enabled = false;
        if (type == 1 || type == 3) {
                Timer2->Interval = 500;
                Timer2->Enabled = true;
        }

}
//---------------------------------------------------------------------------

void __fastcall TMMapView::Timer2Timer(TObject *Sender)
{
        unsigned int addr, d, type, physical;
        unsigned int addr2, d2, type2, physical2;
        int i, n;
        AnsiString asz = "";

        Timer2->Enabled = false;

        Panel2->Caption = "";

        i = L1grid->Row - 1;
        addr = items[i] * 1024 * 1024;
        d = L1TT[items[i]];
        type = d & 3;
        if (type == 0 || type == 2) return;

        Panel2->Caption = "Reading ...";
        Panel2->Repaint();

        if (type == 1) physical = d & ~0x3ff;
        if (type == 3) physical = d & ~0xfff;
        if (ExtRAMsize > 0 && physical >= ExtRAMaddress && physical <= ExtRAMaddress+ExtRAMsize-1) {
                physical = physical+ShadowRAMaddress-ExtRAMaddress;
        }

        n = (type == 1) ? 256 : 1024;
        if (! readMemory(physical, (unsigned char *)L2TT, n*4)) {
                Visible = false;
                return;
        }

        L2grid->RowCount = n+1;

        for (i=0; i<n; i++) {

                d2 = L2TT[i];
                type2 = d2 & 3;
                addr2 = addr + i * ((type == 1) ? 4096 : 1024);
                if (type2 == 1) physical2 = d2 & ~0xffff;
                if (type2 == 2) physical2 = d2 & ~0xfff;
                if (type2 == 3) physical2 = d2 & ~0x3ff;

                L2grid->Cells[0][i+1] = IntToHex((int)addr2, 8);
                L2grid->Cells[1][i+1] = TYPES2[type2];
                L2grid->Cells[2][i+1] = (type2 != 0) ? IntToHex((int)physical2, 8) : asz;
                L2grid->Cells[3][i+1] = (type2 != 0) ? PERMS[(d2 >> 4) & 3] : "";
                L2grid->Cells[4][i+1] = (type2 != 0 && type2 != 3) ? PERMS[(d2 >> 6) & 3] : "";
                L2grid->Cells[5][i+1] = (type2 != 0 && type2 != 3) ? PERMS[(d2 >> 8) & 3] : "";
                L2grid->Cells[6][i+1] = (type2 != 0 && type2 != 3) ? PERMS[(d2 >> 10) & 3] : "";
                L2grid->Cells[7][i+1] = (type2 != 0) ? CB[(d2 >> 2) & 3] : "";

        }

        L2grid->Visible = true;

}
//---------------------------------------------------------------------------



void __fastcall TMMapView::L1gridClick(TObject *Sender)
{
        unsigned int addr;
        int i;

        i = L1grid->Row - 1;
        addr = items[i] * 1024 * 1024;
        rpSetAddress(RPC, addr, true);
}
//---------------------------------------------------------------------------

void __fastcall TMMapView::L2gridClick(TObject *Sender)
{
        unsigned int addr, d, type;
        int i;

        i = L1grid->Row - 1;
        addr = items[i] * 1024 * 1024;
        d = L1TT[items[i]];
        type = d & 3;
        if (type == 0 || type == 2) return;
        i = L2grid->Row - 1;
        addr = addr + i * ((type == 1) ? 4096 : 1024);
        rpSetAddress(RPC, addr, true);
}
//---------------------------------------------------------------------------

