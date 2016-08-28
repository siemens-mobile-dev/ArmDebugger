//---------------------------------------------------------------------------

#include <vcl.h>
#include <stdio.h>
#include "Debug.h"
#include "Pages.h"
#include "Utility.h"
#include "parser.h"
#include "Globals.h"
#pragma hdrstop

#include "FlashMap.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

#define B_OUTSIDE 0
#define B_EMPTY 1
#define B_CODE 2
#define B_ARM 3
#define B_ARMS 4
#define B_THUMB 5
#define B_THUMBS 6
#define B_FFS 7
#define B_EEFULL 8
#define B_EELITE 9
#define B_STRING 10

#define COLOR_OUTSIDE 0xf1f1f1
#define COLOR_EMPTY 0xe8e8e8
#define COLOR_CODE 0x90f1a0
#define COLOR_ARMS 0x0000e9
#define COLOR_ARM 0xc8d0e9
#define COLOR_THUMBS 0xf10000
#define COLOR_THUMB 0xf1d0b8
#define COLOR_FFS 0x70d0f1
#define COLOR_EEPROM 0xf100f1
#define COLOR_STRING 0xa0f1f1

#define CODE 0
#define FFS 1
#define EEFULL 2
#define EELITE 3

#define BORDER 2

TMapForm *MapForm;

static int blocksize = 16;
static unsigned char *binfo[2];
static int w, h;

static unsigned char *ff = NULL;
static unsigned int len = 0;
static int ffnum;

static unsigned int caddr = NONE;
static bool ctrl = false;

bool openFlash(int num) {

        FILE *F;
        char *name;

        if (MapForm->OpenFlashDlg->Execute()) {

                name = MapForm->OpenFlashDlg->FileName.c_str();
                F = fopen(name, "rb");
                if (F == NULL) {
                        return WarningMessage("Cannot open file");
                }
                fseek(F, 0, SEEK_END);
                len = (int)ftell(F);
                fseek(F, 0, SEEK_SET);
                if (len == 0) {
                        fclose(F);
                        return WarningMessage("File is empty");
                }
                ffnum = num;
                ff = (unsigned char *)malloc(len);
                fread(ff, len, 1, F);
                fclose(F);

                MapForm->Image->Visible = false;
                MapForm->Info->Caption = " Please wait...";
                MapForm->Timer1->Enabled = true;

                return true;

        } else {

                return false;

        }

}

//---------------------------------------------------------------------------
__fastcall TMapForm::TMapForm(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void TMapForm::Invoke() {

        if (binfo[0] == NULL) {
                if (!openFlash(0)) return;
        }

        Visible = true;
        BringToFront();
        SetFocus();

}

unsigned int getAddress(int X, int Y) {

        int x, y;

        if (X >= BORDER && X < BORDER+256) {
                x = X - BORDER;
        } else if (X >= BORDER*3+256 && X < BORDER*3+256+256) {
                x = X - BORDER*3+256;
        } else {
                return NONE;
        }
        if (Y < BORDER || Y >= BORDER+h+(h>>5)) return NONE;
        y = Y - BORDER;

        return (((y * 32) / 33) * 256 + x) * blocksize + FlashAddress;

}

//---------------------------------------------------------------------------
void __fastcall TMapForm::ImageMouseMove(TObject *Sender,
      TShiftState Shift, int X, int Y)
{
        char buf[256];

        caddr = getAddress(X, Y);
        if (caddr == NONE) return;

        sprintf(buf, " %08X", caddr);
        Info->Caption = buf;
}
void __fastcall TMapForm::ImageMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
        ctrl = Shift.Contains(ssCtrl);
        ImageMouseMove(Sender, Shift, X, Y);
}
//---------------------------------------------------------------------------
void __fastcall TMapForm::Timer1Timer(TObject *Sender)
{
        TCanvas *can;
        int x, y, xx, nblocks, sectype, i, j, b;
        unsigned short sh;
        unsigned int a, t;
        unsigned char *cinfo;
        int cl;

        flashinfo fli;

        Timer1->Enabled = false;

        analyze(ff, len, FlashAddress, &fli);

        nblocks = len / blocksize;
        if (binfo[ffnum] != NULL) free(binfo[ffnum]);
        cinfo = binfo[ffnum] = (unsigned char *)malloc(nblocks);
        w = 256;
        h = (nblocks + 255) / 256;
        MapForm->Image->Width = w * 2 + BORDER * 4;
        MapForm->Image->Height = h + (h >> 5) + BORDER * 2;

        for (y=0; y<h; y++) {
                for (x=0; x<w; x++) {

                        a = (y*w+x) * blocksize;
                        b = y*w+x;

                        if (a >= len) {
                                cinfo[b] = B_OUTSIDE;
                                continue;
                        }

                        cinfo[b] = B_EMPTY;
                        for (i=0; i<blocksize; i++) {
                                if (ff[a+i] != 0xff) {
                                        cinfo[b] = B_CODE;
                                        break;
                                }
                        }

                        if (cinfo[b] != B_EMPTY) {
                                cinfo[b] = B_EMPTY;
                                for (i=0; i<blocksize; i++) {
                                        if (ff[a+i] != 0) {
                                                cinfo[b] = B_CODE;
                                                break;
                                        }
                                }
                        }

                        // ascii string
                        for (i=0, j=0; i<blocksize; i++) {
                                if (ff[a+i] == 0) {
                                        j++;
                                } else if (ff[a+i] < 0x20 || ff[a+i] > 0x7e) {
                                        j=9999;
                                        break;
                                }
                        }
                        if ((j * 100) / blocksize < 20) cinfo[b] = B_STRING;

                        // unicode string
                        for (i=0, j=0; i<blocksize; i+=2) {
                                sh = ff[a+i] + (ff[a+i+1] << 8);
                                if (sh == 0) {
                                        j++;
                                } else if (sh < 0x20 || sh > 0x7e) {
                                        j=9999;
                                        break;
                                }
                        }
                        if ((j * 100) / blocksize < 20) cinfo[b] = B_STRING;

                        j = a >> 1;
                        if ((fli.fcode[j >> 3] & (1 << (j & 0x7))) != 0) cinfo[b] = B_ARM;
                        if ((fli.fthumb[j >> 3] & (1 << (j & 0x7))) != 0) cinfo[b] = B_THUMB;

                        if ((a & 0x1ffff) == 0) {
                                if (strncmp(&ff[a], "FFS", 3) == 0) {
                                        sectype = FFS;
                                } else if (strncmp(&ff[a], "EEFULL", 6) == 0) {
                                        sectype = EEFULL;
                                } else if (strncmp(&ff[a], "EELITE", 6) == 0) {
                                        sectype = EELITE;
                                } else {
                                        sectype = CODE;
                                }
                        }

                        if (sectype == FFS) cinfo[b] = B_FFS;
                        if (sectype == EEFULL) cinfo[b] = B_EEFULL;
                        if (sectype == EELITE) cinfo[b] = B_EELITE;

                }
        }

        for (i=0; i<fli.count; i++) {
                a = fli.ixf[i].addr;
                t = fli.ixf[i].thumb;
                cinfo[(a-FlashAddress)/blocksize] = t ? B_THUMBS : B_ARMS;
        }

        xx = (ffnum == 0) ? BORDER : BORDER * 2 + w;
        can = Image->Canvas;
        for (y=0; y<h; y++) {
                for (x=0; x<w; x++) {
                        switch (cinfo[y*w+x]) {
                                case B_OUTSIDE: cl = COLOR_OUTSIDE; break;
                                case B_EMPTY: cl = COLOR_EMPTY; break;
                                case B_CODE: cl = COLOR_CODE; break;
                                case B_ARMS: cl = COLOR_ARMS; break;
                                case B_THUMBS: cl = COLOR_THUMBS; break;
                                case B_ARM: cl = COLOR_ARM; break;
                                case B_THUMB: cl = COLOR_THUMB; break;
                                case B_FFS: cl = COLOR_FFS; break;
                                case B_EEFULL: cl = COLOR_EEPROM; break;
                                case B_EELITE: cl = COLOR_EEPROM; break;
                                case B_STRING: cl = COLOR_STRING; break;
                                default: cl = COLOR_OUTSIDE; break;
                        }
                        can->Pixels[x+xx][y+(y>>5)+BORDER] = (TColor)cl;
                }
        }

        free(fli.ixf);
        free(fli.fmap);
        free(fli.fcode);
        free(fli.fthumb);
        free(ff);
        ff = NULL;

        MapForm->Image->Visible = true;
        if (ffnum == 1) MapForm->ClientWidth = w * 2 + 30;
}
//---------------------------------------------------------------------------

void __fastcall TMapForm::OpenFullFlash1Click(TObject *Sender)
{
        openFlash(0);
}
//---------------------------------------------------------------------------

void __fastcall TMapForm::OpenSecondFullFlash1Click(TObject *Sender)
{
        openFlash(1);
}
//---------------------------------------------------------------------------


void __fastcall TMapForm::ImageDblClick(TObject *Sender)
{
        if (caddr == NONE) return;
        if (ctrl) {
                cpSetAddress(caddr, true);
        } else {
                rpSetAddress(RPC, caddr, true);
        }
}
//---------------------------------------------------------------------------

void __fastcall TMapForm::Button1Click(TObject *Sender)
{
        openFlash(0);
}
//---------------------------------------------------------------------------

void __fastcall TMapForm::Button2Click(TObject *Sender)
{
        openFlash(1);
}
//---------------------------------------------------------------------------

void __fastcall TMapForm::SpeedButton3Click(TObject *Sender)
{
        Legend->Visible = ! Legend->Visible;        
}
//---------------------------------------------------------------------------

void __fastcall TMapForm::LegendMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
        Legend->Visible = false;        
}
//---------------------------------------------------------------------------

void __fastcall TMapForm::SBMouseWheelDown(TObject *Sender,
      TShiftState Shift, TPoint &MousePos, bool &Handled)
{
       SB->VertScrollBar->Position += 20;
}
//---------------------------------------------------------------------------

void __fastcall TMapForm::SBMouseWheelUp(TObject *Sender,
      TShiftState Shift, TPoint &MousePos, bool &Handled)
{
       SB->VertScrollBar->Position -= 20;
}
//---------------------------------------------------------------------------

