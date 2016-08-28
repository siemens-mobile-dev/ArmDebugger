//---------------------------------------------------------------------------

#include <vcl.h>
#include <stdio.h>
#include <stdlib.h>
#pragma hdrstop

#include "DForm.h"
#include "Debug.h"
#include "Pages.h"
#include "SearchProgressWin.h"
#include "SearchResults.h"
#include "Globals.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TSearchProgress *SearchProgress;

static unsigned char ucWhat[256];
static unsigned char ucMask[256];
static unsigned char Size;

static unsigned int RegionStart;
static unsigned int RegionLength;
static unsigned int StartAddress;
static unsigned int Address;
static bool Tabulate;
static unsigned int Result;

static unsigned int *results = NULL;
static int nresults;
static int results_size;

static bool CancelQuery;

//---------------------------------------------------------------------------
__fastcall TSearchProgress::TSearchProgress(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------
unsigned int TSearchProgress::Invoke(unsigned char *what, unsigned char *mask,
                        int len,
                        unsigned int r_s, unsigned int r_l,
                        unsigned int a, bool tabulate) {

        memcpy(ucWhat, what, len);
        memcpy(ucMask, mask, len);
        Size = len;
        RegionStart = r_s;
        RegionLength = r_l;
        StartAddress = a;
        Tabulate = tabulate;
        InvokeAgain();
        return Result;

}

void TSearchProgress::InvokeAgain() {

        if (results != NULL) free(results);
        results = NULL;
        nresults = 0;
        results_size = 0;

        Address = StartAddress;
        Button1->Caption = Tabulate ? "Stop" : "Cancel";
        Label1->Caption = "Searching...";
        CancelQuery = false;
        PBar->Max = RegionLength;
        PBar->Position = Address - RegionStart;
        Timer1->Interval = 1;
        Timer1->Enabled = true;
        Result = NONE;
        ShowModal();

}

unsigned int * TSearchProgress::getResults() {

        return results;

}

int TSearchProgress::getResultsCount() {

        return nresults;

}

void TSearchProgress::freeResults() {

        if (results != NULL) free(results);
        results = NULL;

}

void __fastcall TSearchProgress::Button1Click(TObject *Sender)
{
        CancelQuery = true;
}
//---------------------------------------------------------------------------

void __fastcall TSearchProgress::FormCloseQuery(TObject *Sender,
      bool &CanClose)
{
        if (ModalResult == mrNone) {
                CancelQuery = true;
                CanClose = false;
        }
}
//---------------------------------------------------------------------------

void __fastcall TSearchProgress::Timer1Timer(TObject *Sender)
{
        char tbuf[64];
        unsigned int a, eaddr;
        int nres;

        if (CancelQuery) {
                Timer1->Enabled = false;
                ModalResult = mrCancel;
                return;
        }

        eaddr = Address + 524288*2;
        if (eaddr >= RegionStart+RegionLength-1) eaddr = RegionStart+RegionLength-1;
        if (eaddr < Address) eaddr = NONE;

        nres = 0;
        while (Address < eaddr) {
                a = searchMemory(ucWhat, ucMask, Size, Address, eaddr);
                Address = (a == NONE) ? eaddr : a+1;
                if (a != NONE) {
                        if (Tabulate) {
                                if (results_size < nresults+2) {
                                        results_size += 1024;
                                        results = (unsigned int *)realloc(results, results_size * sizeof(unsigned int));
                                }
                                results[nresults] = a;
                                nresults++;
                                nres++;
                                sprintf(tbuf, "Found: %i", nresults);
                                Label1->Caption = tbuf;
                                if (nres >= 20) break;
                        } else {
                                Result = a;
                                CancelQuery = true;
                                break;
                        }
                }
        }
        PBar->Position = Address - RegionStart;

        if (Address >= RegionStart + RegionLength - 1) {
                CancelQuery = true;
                if (Tabulate) {

                } else {
                        Label1->Caption = "Nothing found";
                        Timer1->Enabled = false;
                        Timer1->Interval = 700;
                        Timer1->Enabled = true;
                }
        }

}
//---------------------------------------------------------------------------

