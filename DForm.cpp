//---------------------------------------------------------------------------

#include <vcl.h>
#include <stdio.h>
#include <values.h>
#include <process.h>

#include "Globals.h"
#include "DForm.h"
#include "Config.h"
#include "Pages.h"
#include "Debug.h"
#include "SaveMemory.h"
#include "SaveProgressWin.h"
#include "SearchProgressWin.h"
#include "SearchResults.h"
#include "BootWindow.h"
#include "PatchesWindow.h"
#include "Watchpoint.h"
#include "Snappoints.h"
#include "Assembler.h"
#include "EventLog.h"
#include "Search.h"
#include "GotoWindow.h"
#include "ConfEdit.h"
#include "CallAddress.h"
#include "Calculator.h"
#include "PatchWindow.h"
#include "EditAsc.h"
#include "CP15Window.h"
#include "Nucleus.h"
#include "MemoryMap.h"
#include "Bookmarks.h"
#include "NewWatch.h"
#include "RAMAccess.h"
#include "FlashMap.h"
#include "AutoCGSN.h"
#include "Xrefs.h"
#include "Vkp.h"
#include "Terminal.h"
#include "Utility.h"
#include "About.h"
#include "comm.h"
#include "Wire.h"
#include "Globals.h"

#pragma hdrstop
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TMainForm *MainForm;

int HexSearch = 2;
bool inprocess = false;

// other variables

unsigned int TT1Lo[4096];
unsigned int *TT2Lo[4096];
unsigned int TT1Lc[4096];
unsigned int *TT2Lc[4096];

//---------------------------------------------------------------------------
__fastcall TMainForm::TMainForm(TComponent* Owner)
        : TForm(Owner)
{
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::bCompLoadClick(TObject *Sender)
{
        APC->binfile = compileAPC();
        if (! APC->binfile) return;
        if (APC->target != NONE) loadToAddress(APC->target);
}

//---------------------------------------------------------------------------

void startupInit() {

        randomize();
        calibrateDelayLoop();

        MainForm->Dump1->ColWidths[0] = 62;
        MainForm->Dump2->ColWidths[0] = 62;
        for (int i=1; i<=16; i++) {
                MainForm->Dump1->ColWidths[i] = 17;
                MainForm->Dump2->ColWidths[i] = 17;
        }
        MainForm->Dump1->ColWidths[8] = 22;
        MainForm->Dump2->ColWidths[8] = 22;
        MainForm->Dump1->ColWidths[16] = 22;
        MainForm->Dump2->ColWidths[16] = 22;
        for (int i=17; i<=32; i++) {
                MainForm->Dump1->ColWidths[i] = 7;
                MainForm->Dump2->ColWidths[i] = 7;
        }
        MainForm->Registers->ColWidths[2] = 20;
        MainForm->SideNames->Width = 0;

}

void rescanCommPorts() {

        char ctext[16], tbuf[16], tbuf2[256];
        int idx;

        strcpy(ctext, MainForm->cbComPort->Text.c_str());
        MainForm->cbComPort->Items->Clear();
        for (int i=0; i<256; i++) {
                sprintf(tbuf, "COM%i", i);
                if (QueryDosDevice(tbuf, tbuf2, 256) != 0) {
                        int idx = MainForm->cbComPort->Items->Add(tbuf);
                        if (stricmp(tbuf, ctext) == 0) MainForm->cbComPort->ItemIndex = idx;
                }
        }
}

void __fastcall TMainForm::FormCreate(TObject *Sender)
{
        char tbuf[16], *s;
        int i;

        apInit();
        startupInit();
        readConfig();
        applyConfig();

        RP[0].page = MainForm->Dump1;
        RP[0].Data = (unsigned char *)malloc(65536);
        RP[0].DataLines = 0;
        RP[0].a_pos = 0;
        RP[0].Sticker = MonSticker1;

        RP[1].page = MainForm->Dump2;
        RP[1].Data = (unsigned char *)malloc(65536);
        RP[1].DataLines = 0;
        RP[1].a_pos = 0;
        RP[1].Sticker = MonSticker2;

        CP.page = MainForm->vCPU;
        CP.Data = (unsigned char *)malloc(4096);
        CP.DataLines = 0;
        CP.a_pos = 0;
        CP.Thumb = 0;
        CP.InsWidth = 4;

        WP.count = 0;
        WP.items = NULL;
        WP.page = WatchesList;
        WP.Sticker = MonStickerW;
        WP.page->ColWidths[1] = 1500;
        WP.Visible = true;

        readSession();
        setSystemState(SS_DISCONNECTED);

        for (int i=0; i<9; i++) {
                if (AP[i].inuse) {
                        APC = &AP[i];
                        Pages->ActivePage = APC->sheet;
                        break;
                }
        }
        RPC = &RP[0];

        rescanCommPorts();
        sprintf(tbuf, "COM%i", CommPort);
        for (i=0; i<cbComPort->Items->Count; i++) {
                if (stricmp(cbComPort->Items->Strings[i].c_str(), tbuf) == 0) {
                        cbComPort->ItemIndex = i;
                }
        }
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::cbComPortChange(TObject *Sender)
{
        char *s = cbComPort->Text.c_str();
        if (strncmp(s, "COM", 3) != 0) {
                CommPort = 1;
        } else {
                CommPort = atoi(s+3);
        }
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::bDisconnectClick(TObject *Sender)
{
        lostConnection(0);
        setSystemState(SS_DISCONNECTED);
        disconnect();
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::bConnectClick(TObject *Sender)
{
        SimpleBoot = (ctrlState() != 0);
        if (connect(CommPort, Baudrate)) {
                setSystemState(SS_CONNECTING);
        }
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::New1Click(TObject *Sender)
{
        int i;

        for (i=0; i<9; i++) {
                if (! AP[i].inuse) break;
        }

        if (i == 9) {
                WarningMessage("Cannot open more than 9 files");
                return;
        }

        newSource(&AP[i]);
        APC = &AP[i];
        Pages->ActivePage = APC->sheet;

}

//---------------------------------------------------------------------------

void __fastcall TMainForm::AsmTextChange(TObject *Sender)
{
        ((TMemo *)Sender)->Tag = 1;
        if (APC) APC->binfile = NULL;
}

//---------------------------------------------------------------------------

void openSource(char *name) {

        char *sname;
        int i, j;

        sname = shortName(name);
        for (j=0; j<9; j++) {
                if (AP[j].inuse && stricmp(sname, AP[j].filename) == 0) {
                        APC = &AP[j];
                        MainForm->Pages->ActivePage = APC->sheet;
                        return;
                }
        }

        for (i=0; i<9; i++) {
                if (! AP[i].inuse) break;
        }
        if (i == 9) {
                WarningMessage("Cannot open more than 9 files");
                return;
        }

        if (readSource(name, &AP[i], 1)) {
                APC = &AP[i];
                MainForm->Pages->ActivePage = APC->sheet;
        }

}

void __fastcall TMainForm::Open1Click(TObject *Sender)
{
        char *name;
        AnsiString as;

        if(NewFileDlg->Execute()) {
                as = NewFileDlg->FileName.c_str();
                name = as.c_str();
                openSource(name);
        }
}

//---------------------------------------------------------------------------

void save() {
        writeSource(APC->filename, APC);
}

void saveAs() {

        char *name;
        AnsiString as;
        int l;

        l = strlen(APC->filename);
        if (l > 1 && tolower(APC->filename[l-1]) == 'c') {
                MainForm->SaveAsDlg->FilterIndex = 2;
                MainForm->SaveAsDlg->DefaultExt = "c";
        } else {
                MainForm->SaveAsDlg->FilterIndex = 1;
                MainForm->SaveAsDlg->DefaultExt = "asm";
        }

        if(MainForm->SaveAsDlg->Execute()) {

                as = MainForm->SaveAsDlg->FileName;
                name = as.c_str();

                if (writeSource(name, APC)) {
                        APC->binfile = NULL;
                        APC->sheet->Caption = makeCaption(name, APC->target);
                }

        }
        return;
}

void __fastcall TMainForm::Save1Click(TObject *Sender)
{
        if (strlen(APC->filename) == 0) {
                saveAs();
        } else {
                save();
        }
}

void __fastcall TMainForm::SaveAs1Click(TObject *Sender)
{
        saveAs();
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::Close1Click(TObject *Sender)
{
        if (! saveChanges(APC)) return;
        closePage(APC);
        for (int i=0; i<9; i++) {
                if (AP[i].inuse) {
                        APC = &AP[i];
                        Pages->ActivePage = APC->sheet;
                }
        }
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::AtCutClick(TObject *Sender)
{
        APC->page->CutToClipboard();
}

void __fastcall TMainForm::AtCopyClick(TObject *Sender)
{
        APC->page->CopyToClipboard();
}

void __fastcall TMainForm::AtPasteClick(TObject *Sender)
{
        APC->page->PasteFromClipboard();
}

//---------------------------------------------------------------------------

char *compile(char *filename, bool log) {

        static unsigned char zeros[256];

        FILE *F, *OF;
        int i, r;
        char *p;
        bool CSOURCE;
        char buf[256];
        unsigned char data[256];
        DWORD attr;
        unsigned int a, aaaa, tt, orsum;
        int ll;

        char currentdir[1024];
        char raw_path[1024];
        char raw_file[1024];

        char make_path[1024];
        char base_file[1024];
        char obj_file[1024];
        char elf_file[1024];
        char hex_file[1024];
        char bin_file[1024];
        char err_file[1024];
        char keil_path[1024];
        char ac_options[1024];
        char link_options[1024];

        save();
        MainForm->Mes->Clear();

        sprintf(keil_path, "%s", KeilArmPath);
        strcat(keil_path, "\\bin\\aa.exe");
        F = fopen(keil_path, "rb");
        if (F == NULL) {
                WarningMessage("Keil ARM tools were not found.\nPlease set path to Keil ARM directory");
                return NULL;
        }
        fclose(F);

        strcpy(raw_path, filename);
        p = strrchr(raw_path, '\\');
        if (p == NULL) {
                strcpy(raw_file, filename);
                strcpy(raw_path, BaseDir);
        } else {
                *p = '\0';
                strcpy(raw_file, p+1);
        }
        p = strrchr(raw_file, '.');
        if (p == NULL) {
                WarningMessage("Unknown source file extension");
                return NULL;
        }
        if (stricmp(p, ".asm") == 0) {
                CSOURCE = false;
        } else if (stricmp(p, ".c") == 0) {
                CSOURCE = true;
        } else {
                WarningMessage("Unknown source file extension");
                return NULL;
        }
        *p = '\0';

        sprintf(make_path, "%s\\%s", BaseDir, CSOURCE ? "cmake.bat" : "amake.bat");
        sprintf(base_file, "\"%s\"", raw_file);
        sprintf(obj_file, "%s.obj", raw_file);
        sprintf(elf_file, "%s.elf", raw_file);
        sprintf(hex_file, "%s.hex", raw_file);
        sprintf(bin_file, "%s.bin", raw_file);
        sprintf(err_file, "%s.err", raw_file);
        sprintf(keil_path, "\"%s\"", KeilArmPath);
        sprintf(ac_options, "\"%s\"", CSOURCE ? CompArgs : AsmArgs);
        sprintf(link_options, "\"%s\"", LinkArgs);

        disableFullScreen();

        GetCurrentDirectory(sizeof(currentdir), currentdir);
        SetCurrentDirectory(raw_path);
        attr = GetFileAttributes(bin_file);
        if (attr != 0xFFFFFFFF) {
                if (! DeleteFile(bin_file)) {
                        m_print("Cannot delete binary file (%s)", bin_file);
                }
        }
        r = spawnl(P_WAIT, make_path, make_path, base_file, keil_path, ac_options, link_options, NULL);
        if (log) {
                F = fopen(err_file, "r");
                if (F != NULL) {
                        while (fgets(buf, sizeof(buf), F) != NULL) {
                                m_print("%s", buf);
                        }
                }
                fclose(F);
                m_print(" ");
        }

        DeleteFile(raw_file);
        if (log) DeleteFile(err_file);
        DeleteFile(obj_file);
        DeleteFile(elf_file);
        if (CSOURCE) DeleteFile("_startup.obj");

        if (r != 0) {
                if (log) m_print("Assembler exited with error code %i", r);
                return NULL;
        } else {
                F = fopen(hex_file, "r");
                if (F == NULL) {
                        if (log) m_print("Cannot open %s", hex_file);
                        return NULL;
                }
                OF = fopen(bin_file, "wb");
                if (OF == NULL) {
                        if (log) m_print("Cannot open %s for writing", bin_file);
                        fclose(F);
                        return NULL;
                }

                a = 0;
                while (fgets(buf, sizeof(buf), F) != NULL) {
                        if (buf[0] != ':') continue;
                        ll = readhex(buf+1);
                        aaaa = (readhex(buf+3) << 8) | readhex(buf+5);
                        tt = readhex(buf+7);
                        if (tt == 0x01) break;
                        if (tt != 0x00) {
                                if (a != 0) {
                                        if (log) m_print("Error: non-linear output file");
                                        a=0; break;
                                }
                                continue;
                        }
                        orsum = 0;
                        for (i=0; i<ll; i++) { orsum |= (data[i] = readhex(buf+9+2*i)); }
                        if (aaaa == 0 && orsum == 0) continue;
                        if (a != 0 && aaaa != a) {
                                if (aaaa - a < 256) {
                                        fwrite(zeros, 1, aaaa-a, OF);
                                        a = aaaa;
                                } else {
                                        if (log) m_print("Error: non-linear output file");
                                        a=0; break;
                                }
                        }
                        fwrite(data, 1, ll, OF);
                        a = aaaa + ll;
                }
                fclose(F);
                fclose(OF);
                DeleteFile(hex_file);

                if (a == 0) {
                        if (log) m_print("No output file generated");
                        return NULL;
                }
                if (log) m_print("Assembled successfully");

                strcpy(bin_file, filename);
                p = strrchr(bin_file, '.');
                if (p == NULL) p = bin_file + strlen(bin_file);
                strcpy(p, ".bin");
                return strdup(bin_file);
        }
}

char *compileAPC() {

        char *p;

        if (MainForm->Pages->ActivePage == MainForm->SheetCPU) return NULL;

        p = APC->filename;

        if (p[0] == '\0') {
                saveAs();
                p = APC->filename;
                if (p[0] == '\0') return NULL;
        }
        return compile(p, true);

}

void execBinary(char *name, bool istrace) {

        FILE *F;
        unsigned char *buf;
        int i, l;
        char tbuf[128];
        unsigned int r[16], cpsr, ald, ld;
        int t;
        bool res;

        if (! is_online()) return;

        for (i=0; i<16; i++) r[i] = 0;
        cpsr = 0;

        F = fopen(name, "rb");
        if (F == NULL) {
                m_print("Cannot open binary file");
                return;
        }

        buf = (char *)malloc(16388);
        l = fread(buf, 1, 16384, F);
        fclose(F);

        if (l <= 0) {
                m_print("Binary file is empty");
                free(buf);
                return;
        }

        if (istrace) {
                if (! startTrace(buf, l, r, cpsr)) {
                        WarningMessage("Trace initialization error");
                }
                return;
        }

        if (! ExtraFeatures) {
                res = execute(buf, l, r, 8, cpsr, ExecTimeout);
        } else {
                res = execute2(buf, l, r, 8, cpsr, ExecTimeout);
        }
        if (res) {
                eventMonitor(true);
                if (SystemState == SS_READY) {
                        sprintf(tbuf, "Call: R0=%X", r[0]);
                        newEvent('>', tbuf);
                        setRegisters(r, cpsr, 13, NULL);
                }
        } else {
                newEvent('!', "Call failed");
        }

        free(buf);
        reload_dumps();

}

//---------------------------------------------------------------------------

void run(bool istrace) {

        char *asm_file, *bin_file;

        if (! is_online()) return;
        if (MainForm->Pages->ActivePage == MainForm->SheetCPU) {
                runFromPoint();
                return;
        }

        if (SystemState == SS_INTBRK) {
                WarningMessage("System is in internal breakpoint state");
                return;
        }

        if (SystemState == SS_EXTBRK) {
                programReset();
        }

        MainForm->Mes->Clear();

        if (! APC->binfile) {
                APC->binfile = compileAPC();
                if (! APC->binfile) return;
        }

        execBinary(APC->binfile, istrace);

}

//---------------------------------------------------------------------------

void loadToAddress(unsigned int a) {

        char *asm_file, *bin_file;
        FILE *F;
        unsigned char *buf;
        int l;

        if (! is_online()) return;

        if (! APC->binfile) {
                APC->binfile = compileAPC();
                if (! APC->binfile) return;
        }

        if (a == NONE) {
                if (! GotoForm->Invoke("Load to", NULL, &a)) return;
        }

        F = fopen(APC->binfile, "rb");
        if (F == NULL) {
                m_print("Cannot open binary file");
                return;
        }

        buf = (char *)malloc(16388);
        l = fread(buf, 1, 16384, F);
        fclose(F);

        if (l <= 0) {
                m_print("Binary file is empty");
                free(buf);
                return;
        }

        setMemory(a, buf, l, SM_STICKY);
        reload_dumps();
        free(buf);

}

//---------------------------------------------------------------------------

void __fastcall TMainForm::AtLoadtoClick(TObject *Sender)
{
        loadToAddress(NONE);
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::AtRunClick(TObject *Sender)
{
        run(false);
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::ConTrackerTimer(TObject *Sender)
{
        trackConnection();
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::bRunClick(TObject *Sender)
{
        run(false);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::bStepIntoClick(TObject *Sender)
{
        if (MainForm->Pages->ActivePage != SheetCPU) {
                run(true);
        } else {
                stepInto();
        }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::bStepOverClick(TObject *Sender)
{
        if (MainForm->Pages->ActivePage != SheetCPU) {
                run(true);
        } else {
                stepOver();
        }
}
//---------------------------------------------------------------------------

void update_coords() {

        AnsiString as;

        as.printf(" Line: %i Col: %i", APC->page->CaretPos.y+1, APC->page->CaretPos.x+1);
        MainForm->SBar->Panels->Items[1]->Text = as;
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::AsmText1MouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
        update_coords();
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::AsmText1KeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
      LineUpdateTimer->Enabled = true;

}

//---------------------------------------------------------------------------

void __fastcall TMainForm::LineUpdateTimerTimer(TObject *Sender)
{
       update_coords();
       LineUpdateTimer->Enabled = false;
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::PagesChange(TObject *Sender)
{
        for (int i=0; i<9; i++) {
                if (Pages->ActivePage == AP[i].sheet) {
                        APC = &AP[i];
                        break;
                }
        }
        update_coords();
        if (Pages->ActivePage == SheetCPU) {
                ClosePage1->Enabled = false;
                LogWindow->BringToFront();
                LogWindow->Tag = 1;
                cpu_reload();
                vCPU->SetFocus();
        } else {
                ClosePage1->Enabled = true;
                Mes->BringToFront();
                LogWindow->Tag = 0;
        }
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::Dump1Enter(TObject *Sender)
{
        RPC = &RP[0];
//        RP[1].SelStart = RP[1].SelEnd = RP[1].SelRow = NONE;
//        RP[1].page->Repaint();
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::Dump1TopLeftChanged(TObject *Sender)
{
        dump_reload(&RP[0], false);
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::Dump2TopLeftChanged(TObject *Sender)
{
        dump_reload(&RP[1], false);
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::Panel5Resize(TObject *Sender)
{
        dump_reload(&RP[0], false);
        dump_reload(&RP[1], false);
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::Splitter3Moved(TObject *Sender)
{
        dump_reload(&RP[0], false);
        dump_reload(&RP[1], false);
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::Dump1DrawCell(TObject *Sender, int ACol,
      int ARow, TRect &Rect, TGridDrawState State)
{
        drawRPcell(&RP[0], ACol, ARow, Rect, State);
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::Dump2DrawCell(TObject *Sender, int ACol,
      int ARow, TRect &Rect, TGridDrawState State)
{
        drawRPcell(&RP[1], ACol, ARow, Rect, State);
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::About1Click(TObject *Sender)
{
        AboutForm->ShowModal();
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::Dump2Enter(TObject *Sender)
{
        RPC = &RP[1];
//        RP[0].SelStart = RP[0].SelEnd = RP[0].SelRow = NONE;
//        RP[0].page->Repaint();

}

//---------------------------------------------------------------------------

void __fastcall TMainForm::FormCloseQuery(TObject *Sender, bool &CanClose)
{
        CanClose = closeSession();
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::FormClose(TObject *Sender, TCloseAction &Action)
{
        saveConfig();
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::Refresh1Click(TObject *Sender)
{
        reload_dumps();
}

//---------------------------------------------------------------------------

void cancelEditing() {

        if (! RPC) return;
        if (RPC->InEdit) {
                RPC->InEdit = false;
                RPC->page->Repaint();
        }

}

//---------------------------------------------------------------------------

void __fastcall TMainForm::Dump12KeyPress(TObject *Sender, char &Key)
{
        rpHandleKeypress(RPC, Key);
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::Goto1Click(TObject *Sender)
{
        rpGoto(RPC, -1);
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::Dump2Exit(TObject *Sender)
{
        cancelEditing();
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::Back1Click(TObject *Sender)
{
        unsigned int addr;

        if (RPC->InEdit) {
                cancelEditing();
        } else {
                if (RPC->a_pos > 0) {
                        addr = RPC->a_history[--(RPC->a_pos)];
                        rpSetAddress(RPC, addr, false);
                }
        }
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::Followaddress1Click(TObject *Sender)
{
        rpFollowAddress(RPC, RPC);
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::Followaddressinotherwin1Click(TObject *Sender)
{
        rpFollowAddress(RPC, RPC==&RP[0] ? &RP[1]:&RP[0]);
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::Dump12DblClick(TObject *Sender)
{
        rampage *rp;

        if (shiftState() != 0) {
                rp = (RPC==&RP[0]) ? &RP[1]:&RP[0];
        } else if (ctrlState() != 0) {
                rp = NULL;
        } else {
                rp = RPC;
        }

        if (RPC->page->Col == 0) {
                if (rp != RPC) rpSetAddress(rp, RPC->SelRow, true);
        } else {
                rpFollowAddress(RPC, rp);
        }

}

//---------------------------------------------------------------------------

void __fastcall TMainForm::HexASCII1Click(TObject *Sender)
{
        int col = RPC->page->Col;
        if (col < 17) col += 16; else col -= 16;
        RPC->page->Col = col;
}

//---------------------------------------------------------------------------

void saveMemoryDialog(int addr, int len)
{
        AnsiString filename;
        FILE *F;

        if (! is_online()) return;

        if (addr != ~0) {
                SaveMem->eAddress->Text = "0x" + IntToHex(addr, 8);
                SaveMem->eLength->Text = "0x" + IntToHex(len, 8);
        }

        if (SaveMem->ShowModal() == mrOk) {
                if (!TryStrToInt(SaveMem->eAddress->Text, addr) || !TryStrToInt(SaveMem->eLength->Text, len)) {
                        WarningMessage("Error in address or length fiels");
                        return;
                }

                F = fopen(SaveMem->eFilename->Text.c_str(), "wb");
                if (F == NULL) {
                        ErrorMessage("Cannot open file for writing");
                        return;
                }

                SaveProgress->initiate(F, addr, len);

        }
}

void __fastcall TMainForm::Savetofile1Click(TObject *Sender)
{
        unsigned int s, e, l;

        if (RPC->SelStart <= RPC->SelEnd) {
                s = RPC->SelStart;
                e = RPC->SelEnd;
        } else {
                s = RPC->SelEnd;
                e = RPC->SelStart;
        }

        if (s == NONE) s = e = RPC->SelRow;
        if (s == 0 || s == NONE) s = rpGetAddress(RPC);
        l = (s == e) ? 0x100 : e-s+1;
        saveMemoryDialog(s & ~0x03, (l+3) & ~0x03);
}

void __fastcall TMainForm::Savememorytofile1Click(TObject *Sender)
{
        saveMemoryDialog(~0, 0);
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::Dump12Click(TObject *Sender)
{
        cancelEditing();
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::bBootClick(TObject *Sender)
{
        char *asm_file, *bin_file;
        FILE *F;
        unsigned char buf[16384];
        int i, l, x;

        if (! is_connected()) return;
        if (MainForm->Pages->ActivePage == SheetCPU) return;
        MainForm->Mes->Clear();

        if (! APC->binfile) {
                APC->binfile = compileAPC();
                if (! APC->binfile) return;
        }

        F = fopen(APC->binfile, "rb");
        if (F == NULL) {
                m_print("Cannot open binary file");
                return;
        }

        l = fread(buf+3, 1, 16374, F);
        fclose(F);

        if (l <= 0) {
                m_print("Binary file is empty");
                return;
        }

        buf[0] = 0x30;
        buf[1] = l & 0xff;
        buf[2] = l >> 8;
        l += 3;

        x = 0;
        for (i=3; i<l; i++) {
                x ^= buf[i];
        }

        buf[l] = x;
        buf[l+1] = 0;
        l += 2;

        BootForm->Invoke(buf, l);

}

//---------------------------------------------------------------------------

void __fastcall TMainForm::SelectAll1Click(TObject *Sender)
{
        APC->page->SelectAll();
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::Find1Click(TObject *Sender)
{
        SearchForm->Open();
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::MesDblClick(TObject *Sender)
{
        char *p, *pp;
        TPoint cp;

        Mes->SelLength = 0;
        int y = Mes->CaretPos.y;
        AnsiString as = Mes->Lines->Strings[y];
        p = as.c_str();

        int n = strtol(p, &pp, 10);
        if (*pp == ' ' || *pp == '\t') {
                cp.x = 0;
                cp.y = n-1;
                APC->page->CaretPos = cp;
                APC->page->SelLength = APC->page->Lines->Strings[n-1].Length();
        }

}

//---------------------------------------------------------------------------

void __fastcall TMainForm::FindAgain1Click(TObject *Sender)
{
        SearchForm->NextSearch(NULL);
}

//---------------------------------------------------------------------------


void __fastcall TMainForm::bSearchTypeClick(TObject *Sender)
{
        static char lasthex[64], lasttext[64];
        AnsiString as;

        if (HexSearch == 2) {
                bSearchType->Caption = "ASCII:";
                HexSearch = 0;
                as = eHexSearch->Text;
                eHexSearch->MaxLength = 64;
                strcpy(lasthex, as.c_str());
                eHexSearch->Text = lasttext;
        } else if (HexSearch == 0) {
                bSearchType->Caption = "UCS2:";
                HexSearch = 1;
                eHexSearch->MaxLength = 32;
        } else {        // HexSearch == 1
                bSearchType->Caption = "Hex:";
                HexSearch = 2;
                as = eHexSearch->Text;
                strcpy(lasttext, as.c_str());
                eHexSearch->Text = lasthex;
        }
        eHexSearch->SelectAll();
}

//---------------------------------------------------------------------------

void MemorySearch(bool tabulate) {

        unsigned int res;
        unsigned int addr_s, addr_e;
        AnsiString as;
        char s[256];
        wchar_t ws[256];
        unsigned char what[256];
        unsigned char mask[256];
        int i, n;
        unsigned char c1, c2;
        memoryinfo mi;

        if (! is_online()) return;


        if (HexSearch == 2) {
                strcpy(s, MainForm->eHexSearch->Text.c_str());
                n = scan16m(s, what, mask, sizeof(what));
        } else if (HexSearch == 0) {
                strcpy(s, MainForm->eHexSearch->Text.c_str());
                n = strlen(s);
                memcpy(what, s, n);
                for (i=0; i<n; i++) {
                        if (what[i] == '?') {
                                what[i] = mask[i] = 0;
                        } else {
                                mask[i] = 0xff;
                        }
                }
        } else {        // HexSearch == 1
                n = MainForm->eHexSearch->Text.WideCharBufSize() - 1;
                MainForm->eHexSearch->Text.WideChar(ws, 32);
                for (i=0; i<n; i++) {
                        if (what[i] == '?') {
                                what[i*2] = what[i*2+1] = 0;
                                mask[i*2] = mask[i*2+1] = 0;
                        } else {
                                what[i*2] = ws[i] & 0xff;
                                what[i*2+1] = (ws[i] >> 8) & 0xff;
                                mask[i*2] = mask[i*2+1] = 0xff;
                        }
                }
                n *= 2;
        }
        if (n == 0 || n > 64) return;

        addr_s = rpGetAddress(RPC) + 1;
        getMemoryInfo(addr_s, &mi);
        if (mi.len == 0) return;

        if (mi.type == M_IO) {
                WarningMessage("Cannot search in I/O area");
                return;
        }

        SearchResForm->Dismiss();
        res = SearchProgress->Invoke(what, mask, n, mi.addr, mi.len, addr_s, tabulate);
        if (tabulate) {
                unsigned int *alist = SearchProgress->getResults();
                int acount = SearchProgress->getResultsCount();
                if (acount > 0) SearchResForm->Invoke(mi.addr, mi.len, what, mask, n, alist, acount, HexSearch);
        } else {
                if (res != NONE) {
                        rpSetAddress(RPC, res, true);
                        MainForm->eHexSearch->SetFocus();
                }
        }
        SearchProgress->freeResults();
}

void __fastcall TMainForm::bSearchClick(TObject *Sender)
{
        MemorySearch(false);
}

void __fastcall TMainForm::SearchAgain1Click(TObject *Sender)
{
        MemorySearch(false);
}

void __fastcall TMainForm::bSearchTabClick(TObject *Sender)
{
        MemorySearch(true);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::eHexSearchKeyPress(TObject *Sender, char &Key)
{
        char buf[256], buf2[256];
        int i, j, p1, p1e, p2, p2e;

        if (Key == 0x02) {
                Key = 0;
                Bookmark3Click(Sender);
        }
        if (Key == 0x0d || Key == 0x0a) {
                Key = 0;
                MemorySearch(ctrlState());
        }
        if (Key == 0x09) {
                Key = 0;
                MainForm->bSearchTypeClick(Sender);
        }

        strcpy(buf2, MainForm->eHexSearch->Text.c_str());
        p1 = eHexSearch->SelStart;
        p1e = p1 + eHexSearch->SelLength;

        if (HexSearch == 2) {

                int len = strlen(buf2);
                p2 = 0;
                for (i=0, j=0; i<len; i++) {
                        if (! (isxdigit(buf2[i]) || buf2[i] == '?')) continue;
                        if (Key > 'F') Key -= 0x20;
                        buf[j++] = buf2[i];
                        if (p1 > i) p2 = j;
                        if (p1e > i) p2e = j;
                }
                buf[j] = 0;
                len = j;

                if (isxdigit(Key) || Key == '?') {
                        if (Key > 'F') Key -= 0x20;
                        memmove(buf+p2+1, buf+p2, len-p2+1);
                        buf[p2++] = Key;
                }

                if (Key == 0x08) {
                        if (p1e != p1) {
                                memmove(buf+p2, buf+p2e, len-p2e+1); 
                        } else if (p2 > 0) {
                                memmove(buf+p2-1, buf+p2, len-p2+1);
                                p2--;
                        }
                }

                len = strlen(buf);
                for (i=0, j=0, p1=0; i<=len; i++) {
                        buf2[j++] = buf[i];
                        if ((j%3)==2) buf2[j++] = ' ';
                        if (p2 > i) p1 = j;
                }
                eHexSearch->Text = buf2;
                eHexSearch->SelStart = p1;
                Key = 0;

        }

}

//---------------------------------------------------------------------------

void __fastcall TMainForm::Search1Click(TObject *Sender)
{
        eHexSearch->SetFocus();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::RegistersDrawCell(TObject *Sender, int ACol,
      int ARow, TRect &rect, TGridDrawState state)
{
        drawRegCell(ACol, ARow, rect, state);
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::Newfile1Click(TObject *Sender)
{
        New1Click(Sender);
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::openFile1Click(TObject *Sender)
{
        Open1Click(Sender);

}

//---------------------------------------------------------------------------

void __fastcall TMainForm::ClosePage1Click(TObject *Sender)
{
        Close1Click(Sender);
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::Calladdress1Click(TObject *Sender)
{
        CallForm->Visible = true;
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::Patchmemory1Click(TObject *Sender)
{
        unsigned int addr;

        if (! is_online()) return;
        if (RPC->page->Col == 0 || RPC->page->Col > 16) return;

        addr = rpGetAddress(RPC);
        if (PatchForm->Invoke(addr) == mrOk) {
                setMemory(PatchForm->Address, PatchForm->Data, PatchForm->Length, SM_ISOLATED);
        }

        PatchesForm->reloadList();
        reload_dumps();
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::ApplyVKP1Click(TObject *Sender)
{
        AnsiString as;
        unsigned int a;
        int n, n1, n2;
        char buf[18000];
        unsigned char data1[4096], data2[4096], *p;
        unsigned char *currentData;
        unsigned int currentAddress, currentLength;
        int i, ln, line;
        bool checkdata;
        TStrings *strings;

        if (! is_online()) return;
        if (RPC->page->Col == 0 || RPC->page->Col > 16) return;

        line = -1;
        while (VkpForm->Invoke(line) == mrOk) {

                line = 0;
                checkdata = true;
                currentData = NULL;
                currentAddress = NONE;
                currentLength = 0;

                strings = VkpForm->VkpText->Lines;
                for (ln=0; ln<strings->Count; ln++) {
                        strcpy(buf+1, strings->Strings[ln].c_str());
                        line++;
                        p = buf+1; while (*p == ' ' || *p == '\t') p++;
                        if (*p == '#' || *p == ';' || *p == '\r' || *p == '\n' || *p == 0) continue;
                        n = scan16(p, data1, sizeof(data1), false);
                        if (isxdigit(*(p+n*2))) {
                                *(--p) = '0';
                                n = scan16(p, data1, sizeof(data1), false);
                        }
                        if (n > 8 || n < 2 || *(p+n*2) != ':') {
                                WarningMessage("Error in vkp file at line %i", line);
                                break;
                        }
                        for (a=0, i=0; i<n; i++) a = (a << 8) + data1[i];
                        if (a < FlashSize) a += FlashAddress;

                        p += n*2+1; while (*p == ' ' || *p == '\t') p++;
                        n1 = scan16(p, data1, sizeof(data1), false);
                        if (n1 == 0) {
                                WarningMessage("Error in vkp file at line %i", line);
                                break;
                        }
                        if (n1 == sizeof(data1)) {
                                WarningMessage("Too large data block at line %i", line);
                                break;
                        }

                        p += n1*2; while (*p == ' ' || *p == '\t') p++;
                        if (*p != '\r' && *p != '\n' && *p != ';') {
                                n2 = scan16(p, data1, sizeof(data1), false);
                                if (n2 == 0) {
                                        WarningMessage("Error in vkp file at line %i", line);
                                        break;
                                }
                                if (n2 != n1) {
                                        WarningMessage("Old and new data length differs at line %i", line);
                                        break;
                                }

                                // check old data
                                if (checkdata) {
                                        if (! readMemory(a, data2, n1)) return;
                                        if (memcmp(data1, data2, n1) != 0) {
                                                if (YesNoQuestion("Old data is not match (address 0x%08X). Continue?", a)) {
                                                        checkdata = false;
                                                } else {
                                                        line = -1;
                                                        break;
                                                }
                                        }
                                }
                        }

                        if (currentData != NULL && currentAddress+currentLength == a) {
                                currentData = (unsigned char *)realloc(currentData, currentLength+n1);
                                memcpy(currentData+currentLength, data1, n1);
                                currentLength += n1;
                        } else {
                                if (currentData != NULL) {
                                        setMemory(currentAddress, currentData, currentLength, SM_ISOLATED);
                                        free(currentData);
                                }
                                currentData = (unsigned char *)malloc(n1);
                                memcpy(currentData, data1, n1);
                                currentAddress = a;
                                currentLength = n1;
                        }
                }
                if (ln < strings->Count) continue;
                if (currentData != NULL) {
                        setMemory(currentAddress, currentData, currentLength, SM_ISOLATED);
                }
                break;

        }

        PatchesForm->reloadList();
        reload_dumps();
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::Opensession1Click(TObject *Sender)
{
        AnsiString as;
        char *s;

        if (OpenSessionDlg->Execute()) {
                as = OpenSessionDlg->FileName;
                s = as.c_str();
                if (strlen(s) > 4 && s[strlen(s)-4] == '.') s[strlen(s)-4] = '\0';

                closeSession();
                readSession(s);
        }
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::Savesession1Click(TObject *Sender)
{
        saveSession();
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::Patches1Click(TObject *Sender)
{
        PatchesForm->Invoke();
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::CPUwindow1Click(TObject *Sender)
{
        Pages->ActivePage = SheetCPU;
        vCPU->SetFocus();
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::Dump12SelectCell(TObject *Sender, int ACol,
      int ARow, bool &CanSelect)
{
        if (! RPC) return;
        cancelEditing();

        bool inhex = (ACol <= 16);
        MainForm->Goto1->Enabled = inhex;
        MainForm->Monitor1->Enabled = inhex;
        MainForm->References1->Enabled = inhex;
        MainForm->Patchmemory1->Enabled = inhex;
        MainForm->Goto1->ShortCut = inhex ? TextToShortCut("G") : 0;
        MainForm->Monitor1->ShortCut = inhex ? TextToShortCut("M") : 0;
        MainForm->References1->ShortCut = inhex ? TextToShortCut("X") : 0;
        MainForm->Patchmemory1->ShortCut = inhex ? TextToShortCut("P") : 0;

        if (ACol > 16) ACol -= 16;

        unsigned int a = RPC->BaseAddress + ARow * 0x10 + (ACol == 0 ? 0 : (ACol - 1));
        if (shiftState() == 0 && ! RPC->LMouseDown) {
                if (ACol == 0) {
                        RPC->SelStart = RPC->SelEnd = NONE;
                        RPC->SelRow = a;
                } else {
                        RPC->SelStart = RPC->SelEnd = a;
                }
        } else {
                if (RPC->SelStart == NONE) RPC->SelStart = RPC->SelRow;
                RPC->SelEnd = (ACol == 0 && RPC->SelStart < RPC->SelEnd) ? a-1 : a;
        }

        RPC->page->Repaint();
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::Dump12MouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
        int acol, arow;

        RPC->page->MouseToCell(X, Y, acol, arow);

        if (Button == mbLeft && ! Shift.Contains(ssShift)) {
                if (acol == 0) {
                        RPC->SelRow = rpGetAddress(RPC);
                        RPC->SelStart = RPC->SelEnd = NONE;
                } else {
                        RPC->SelStart = RPC->SelEnd = rpGetAddress(RPC);
                }
                RPC->LMouseDown = true;
                RPC->page->Repaint();
        }
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::Dump12MouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
        RPC->LMouseDown = false;
}

//---------------------------------------------------------------------------
void __fastcall TMainForm::CopyAddress1Click(TObject *Sender)
{
        char p[16];

        if (! is_online()) return;

        if (RPC->SelStart == NONE) {
                sprintf(p, "%08X", RPC->SelRow);
        } else {
                sprintf(p, "%08X", RPC->SelStart);
        }
        setClipboard(p);
}

//---------------------------------------------------------------------------
void __fastcall TMainForm::CopyWord1Click(TObject *Sender)
{
        unsigned int buf[1];
        char p[16];

        if (! is_online()) return;

        if (RPC->SelStart == NONE) {
                sprintf(p, "%08X", RPC->SelRow);
                setClipboard(p);
        } else {
                if (readMemory(RPC->SelStart & ~0x3, (unsigned char *)buf, 4)) {
                        sprintf(p, "%08X", *buf);
                        setClipboard(p);
                }
        }
}

//---------------------------------------------------------------------------
void __fastcall TMainForm::Copy1Click(TObject *Sender)
{
        copyRPCdata();
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::Paste1Click(TObject *Sender)
{
        pasteRPCdata();
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::MonitorTimerTimer(TObject *Sender)
{
        if (inprocess) return;
        if (! is_online()) return;

        if (RP[0].Monitor) dump_reload(&RP[0], true);
        if (RP[1].Monitor) dump_reload(&RP[1], true);
        if (WP.Monitor) watches_reload();
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::Monitor1Click(TObject *Sender)
{
        RPC->Monitor = ! RPC->Monitor;
        RPC->Sticker->Visible = RPC->Monitor;
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::DumpMenuPopup(TObject *Sender)
{
        Monitor1->Checked = RPC->Monitor;
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::Log1Click(TObject *Sender)
{
        LogWindow->BringToFront();
        LogWindow->Tag = 1;
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::Compileroutput1Click(TObject *Sender)
{
        Mes->BringToFront();
        LogWindow->Tag = 0;
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::Fill1Click(TObject *Sender)
{
        unsigned int s, e, n;
        unsigned char *buf;

        if (RPC->SelStart <= RPC->SelEnd) {
                s = RPC->SelStart;
                e = RPC->SelEnd;
        } else {
                s = RPC->SelEnd;
                e = RPC->SelStart;
        }

        if (s == NONE) return;

        n = (e - s) + 1;
        if (n > 65536) {
                WarningMessage("Block is too large");
                return;
        }

        buf = (unsigned char *)calloc(n+8, 1);
        setMemory(s, buf, n, SM_STICKY);
        free(buf);
        reload_dumps();

}

//---------------------------------------------------------------------------

void __fastcall TMainForm::Exit1Click(TObject *Sender)
{
        MainForm->Close();
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::CP15registers1Click(TObject *Sender)
{
        if (! is_online()) return;
        CP15Form->Invoke();
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::RAMAccessAnalyzer1Click(TObject *Sender)
{
        AccessAnalyzer->Invoke();
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::GenerateCGSNpatch1Click(TObject *Sender)
{
        AutoCGSNform->Invoke();
}

//---------------------------------------------------------------------------


void __fastcall TMainForm::vCPUDrawCell(TObject *Sender, int ACol,
      int ARow, TRect &Rect, TGridDrawState State)
{
        drawCPcell(ACol, ARow, Rect, State);
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::vCPUMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
        int acol, arow;

        CP.page->MouseToCell(X, Y, acol, arow);

        if (Button == mbLeft && ! Shift.Contains(ssShift)) {
                CP.SelStart = cpGetAddress();
                CP.SelEnd = CP.SelStart + CP.InsWidth-1;
                CP.LMouseDown = true;
                CP.page->Repaint();
        }

}
//---------------------------------------------------------------------------

void __fastcall TMainForm::vCPUMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
        CP.LMouseDown = false;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::vCPUSelectCell(TObject *Sender, int ACol,
      int ARow, bool &CanSelect)
{
        unsigned int a = CP.BaseAddress + ARow * (CP.Thumb ? 2 : 4);
        if (shiftState() == 0 && ! CP.LMouseDown) {
                CP.SelStart = CP.SelEnd = a;
                CP.SelEnd = CP.SelStart + CP.InsWidth-1;
        } else {
                CP.SelEnd = a + CP.InsWidth-1;
                if (CP.SelStart == NONE) CP.SelStart = a;
        }

        CP.page->Repaint();

}
//---------------------------------------------------------------------------

void __fastcall TMainForm::vCPUTopLeftChanged(TObject *Sender)
{
        cpu_reload();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::Refresh2Click(TObject *Sender)
{
        cpu_reload();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::Follow1Click(TObject *Sender)
{
        cpFollowAddress();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::vCPUDblClick(TObject *Sender)
{
        cpFollowAddress();
}

void __fastcall TMainForm::Goto2Click(TObject *Sender)
{
        cpGoto();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::Back2Click(TObject *Sender)
{
        unsigned int addr;

        if (CP.a_pos > 0) {
                addr = CP.a_history[--(CP.a_pos)];
                cpSetAddress(addr, false);
        }

}
//---------------------------------------------------------------------------

void __fastcall TMainForm::FormResize(TObject *Sender)
{
        SBar->Panels->Items[1]->Width = MainForm->Width - 235;
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::switchtodump11Click(TObject *Sender)
{
        Dump1->SetFocus();
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::switchtolog1Click(TObject *Sender)
{
        if (LogWindow->Tag) {
                LogWindow->SetFocus();
        } else {
                Mes->SetFocus();
        }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::switchtoeditor1Click(TObject *Sender)
{
        if (Pages->ActivePage != SheetCPU) {
                APC->page->SetFocus();
        } else {
                vCPU->SetFocus();
        }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::DumpAltDownClick(TObject *Sender)
{
        if (RPC == &RP[0]) {
                Dump2->SetFocus();
        } else {
                Registers->SetFocus();
        }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::altup1Click(TObject *Sender)
{
        Dump1->SetFocus();        
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::Undopatch1Click(TObject *Sender)
{
        patch *p;

        p = getPatchByAddress(rpGetAddress(RPC));
        if (p == NULL)
                { WarningMessage("No patch at selected address"); return; }

        undoPatch(p->address);
        PatchesForm->reloadList();
        reload_dumps();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::Loadfromfile1Click(TObject *Sender)
{
        unsigned char *buf;
        int l;
        AnsiString as;
        FILE *F;

        if (! is_online()) return;
        if (! OpenBinDlg->Execute()) return;

        as = OpenBinDlg->FileName;
        F = fopen(as.c_str(), "rb");
        if (F == NULL) {
                WarningMessage("Cannot open file");
                return;
        }

        buf = (char *)malloc(65536);
        l = fread(buf, 1, 65536, F);
        fclose(F);

        if (l <= 0) {
                m_print("Binary file is empty");
                free(buf);
                return;
        }

        setMemory(rpGetAddress(RPC), buf, l, SM_ISOLATED);
        free(buf);
        reload_dumps();

}
//---------------------------------------------------------------------------

void __fastcall TMainForm::SetLoadTarger1Click(TObject *Sender)
{
        unsigned int a;

        if (! GotoForm->Invoke("Target", NULL, &a)) return;
        APC->target = a;
        APC->sheet->Caption = makeCaption(APC->filename, a);

}
void __fastcall TMainForm::RemoveLoadTarget1Click(TObject *Sender)
{
        APC->target = NONE;
        APC->sheet->Caption = makeCaption(APC->filename, NONE);
}
//---------------------------------------------------------------------------
void renameDialog(unsigned int a) {

        char name[256], *p;
        int i;

        p = nameByAddress(a);
        if (p != NULL) strcpy(name, p); else name[0] = '\0';
        p = EditAscForm->Invoke("Name", name, false, 255);
        if (p != NULL) {
                if (*p != '\0') {
                        for (i=0; p[i] != '\0'; i++) { if (! isalnum(p[i])) p[i] = '_'; }
                        addName(p, a, true);
                } else {
                        removeName(a);
                }
                MainForm->vCPU->Repaint();
                updateSideNames();
        }
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::Name1Click(TObject *Sender)
{
        unsigned int a;

        a = cpGetAddress();
        renameDialog(a);
}

//---------------------------------------------------------------------------




void __fastcall TMainForm::LoadNames1Click(TObject *Sender)
{
        char buf[4096];
        AnsiString as;
        FILE *F;
        unsigned int a;
        char *p, *pp;

        if (! ImportIdcDlg->Execute()) return;

        as = ImportIdcDlg->FileName;
        F = fopen(as.c_str(), "r");

        if (F == NULL) {
                WarningMessage("Cannot open file");
                return;
        }

        while (fgets(buf, sizeof(buf), F) != NULL) {

                p = buf;
                while (*p == ' ' || *p == '\t') p++;
                if (strncmp(p, "MakeName", 8) != 0) continue;
                while (*p != '(' && *p != '\0') p++;
                if (*p == '\0') continue;
                p++;
                a = strtoul(p, &pp, 0);
                while (*p != '\"' && *p != '\0') p++;
                if (*p == '\0') continue;
                p++;
                pp = p;
                while (*pp != '\"' && *pp != '\0') pp++;
                if (*pp == '\0') continue;
                *pp = '\0';
                addNameFast(p, a, true);

        }

        fclose(F);

        reload_dumps();
        updateSideNames();

}
//---------------------------------------------------------------------------



void __fastcall TMainForm::ExportNames1Click(TObject *Sender)
{
        AnsiString as;
        FILE *F;
        aname **xx;
        int i, n;

        if (! ExportIdcDlg->Execute()) return;

        as = ExportIdcDlg->FileName;
        F = fopen(as.c_str(), "w");

        if (F == NULL) {
                WarningMessage("Cannot open file");
                return;
        }

        xx = getNameList(true);
        n = getNameCount(true);

        fprintf(F, "// Generated by ArmDebugger (c) Chaos\n");
        fprintf(F, "// Session: %s\n", SessionName);
        fprintf(F, "\n");
        fprintf(F, "static main(void) {\n");

        for (i=0; i<n; i++) {
                fprintf(F, "\tMakeName(0x%08X,\"%s\");\n", xx[i]->address, xx[i]->name);
        }

        fprintf(F, "}\n");
        fclose(F);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::Assemble12Click(TObject *Sender)
{
        char asm_file[1024], err_file[1024];
        char tbuf[1024];
        char *bin_file, *ctext, *p;
        unsigned char buf[16];
        aname **xx;
        int i, n, s;
        unsigned int a;
        AnsiString as;
        FILE *F, *BF, *EF;

        GetTempPath(sizeof(asm_file)-16, asm_file);
        strcpy(err_file, asm_file);
        strcat(asm_file, "a65temp.asm");
        strcat(err_file, "a65temp.err");

        F = fopen(asm_file, "w");
        if (F == NULL) {
                WarningMessage("Cannot open temporary file");
                return;
        }

        n = getNameCount(true);
        xx = getNameList(true);
        for (i=0; i<n; i++) {
                fprintf(F, "%s EQU 0x%X\n", xx[i]->name, xx[i]->address);
        }
        n = getNameCount(false);
        xx = getNameList(false);
        for (i=0; i<n; i++) {
                fprintf(F, "%s EQU 0x%X\n", xx[i]->name, xx[i]->address);
        }

        fflush(F);
        s = ftell(F);
        fclose(F);

        AssemblerForm->Status->Caption = "";
        ctext = NULL;

        while (AssemblerForm->Invoke(cpGetAddress(), ctext) == mrOk) {

                F = fopen(asm_file, "r+");
                fseek(F, s, SEEK_SET);
                a = cpGetAddress();
                fprintf(F, "AREA TEMPCODE, CODE, AT 0x%X\n", a);
                fprintf(F, CP.Thumb ? "CODE16\n" : "CODE32\n");
                as = AssemblerForm->EditLine->Text;
                fprintf(F, "\t%s\n", ctext = as.c_str());
                fprintf(F, "END\n");
                fprintf(F, "                                             ");
                fprintf(F, "                                             ");
                fclose(F);

                bin_file = compile(asm_file, false);
                if (bin_file != NULL) BF = fopen(bin_file, "rb");
                if(bin_file == NULL || BF == NULL) {
                        AssemblerForm->Status->Caption = "Error";
                        EF = fopen(err_file, "r");
                        if (EF != NULL) {
                                while (fgets(tbuf, sizeof(tbuf), EF) != NULL) {
                                        if (strncmp(tbuf, "***", 3) == 0) {
                                                p = strchr(tbuf, ':');
                                                if (p != NULL) {
                                                        AssemblerForm->Status->Caption = p+1;
                                                        break;
                                                }
                                        }
                                }
                                fclose(EF);
                        }
                } else {
                        n = fread(buf, 1, 16, BF);
                        fclose(BF);
                        if (n != CP.InsWidth && n != 4) {
                                AssemblerForm->Status->Caption = "Unsupported instruction";
                                continue;
                        }
                        AssemblerForm->Status->Caption = "";
                        setMemory(a, buf, n, SM_STICKY);
                        cpu_reload();
                        cpSetAddress(CP.Thumb ? (a+n)|1 : a+n, false);
                        ctext = NULL;
                }

        }
}
//---------------------------------------------------------------------------



void __fastcall TMainForm::SaveAsDlgTypeChange(TObject *Sender)
{
        SaveAsDlg->DefaultExt = (SaveAsDlg->FilterIndex == 1) ? "asm" : "c";
}
//---------------------------------------------------------------------------

void updateSideNames() {

        aname **xx;
        int i, n;

        n = getNameCount(true);
        xx = getNameList(true);

        MainForm->SideNames->Clear();
        MainForm->SideNames->Sorted = ! MainForm->SideNames->Tag;
        for (i=0; i<n; i++) {
                MainForm->SideNames->AddItem(xx[i]->name, NULL);
        }
}

void __fastcall TMainForm::SortbyAddress1Click(TObject *Sender)
{
        MainForm->SideNames->Tag = 1;
        updateSideNames();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::SortbyName1Click(TObject *Sender)
{
        MainForm->SideNames->Tag = 0;
        updateSideNames();
}
//---------------------------------------------------------------------------
void gotoCurrentName() {

        int n = MainForm->SideNames->ItemIndex;
        AnsiString as = MainForm->SideNames->Items->Strings[n];
        unsigned int a = addressByName(as.c_str());
        if (a != NONE) cpSetAddress(a, true);
}


void __fastcall TMainForm::SideNamesDblClick(TObject *Sender)
{
        gotoCurrentName();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::SideNamesKeyPress(TObject *Sender, char &Key)
{
        switch (Key) {
                case 0x0a:
                case 0x0d:
                        gotoCurrentName();
                        break;
                case 0x1b:
                        vCPU->SetFocus();
                        SideNames->Width = 0;
                        break;
        }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::switchtovCPU1Click(TObject *Sender)
{
        vCPU->SetFocus();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::switchtolog3Click(TObject *Sender)
{
        if (LogWindow->Tag) {
                LogWindow->SetFocus();
        } else {
                Mes->SetFocus();
        }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::switchto1Click(TObject *Sender)
{
        if (Pages->ActivePage != SheetCPU) {
                APC->page->SetFocus();
        } else {
                vCPU->SetFocus();
        }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::switchtosidenames1Click(TObject *Sender)
{
        if (SideNames->Width == 0) SideNames->Width = 75;
        SideNames->SetFocus();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::Names1Click(TObject *Sender)
{
        Pages->ActivePage = SheetCPU;
        if (Panel11->Width < 150) Panel11->Width = 150;
        SideNames->SetFocus();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ARMThumb1Click(TObject *Sender)
{
        unsigned int addr;
        memoryinfo mi;

        addr = cpGetAddress();
        getMemoryInfo(addr, &mi);
        CP.Freeze = true;
        if (CP.Thumb) {
                CP.Thumb = false;
                CP.InsWidth = 4;

        } else {
                CP.Thumb = true;
                CP.InsWidth = 2;
        }
        cpSetAddress(CP.Thumb ? addr+1 : addr, false);
        CP.Freeze = false;
        cpu_reload();

}
//---------------------------------------------------------------------------

void __fastcall TMainForm::Sessionoptions1Click(TObject *Sender)
{
        ConfigForm->Invoke();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::SpeedButton1Click(TObject *Sender)
{
        New1Click(Sender);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::SpeedButton2Click(TObject *Sender)
{
        Open1Click(Sender);
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::SpeedButton3Click(TObject *Sender)
{
        Save1Click(Sender);
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::Newsession1Click(TObject *Sender)
{
        newSession();        
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::Savesessionas1Click(TObject *Sender)
{
        saveSessionAs();
}
//---------------------------------------------------------------------------



void __fastcall TMainForm::AsmTextPopupPopup(TObject *Sender)
{
        if (APC->target == NONE) {
                SetLoadTarger1->Visible = true;
                RemoveLoadTarget1->Visible = false;
        } else {
                SetLoadTarger1->Visible = false;
                RemoveLoadTarget1->Visible = true;
        }
}
//---------------------------------------------------------------------------



void __fastcall TMainForm::Rename1Click(TObject *Sender)
{
        int nitems = MainForm->SideNames->Items->Count;
        if (nitems == 0) return;
        int n = MainForm->SideNames->ItemIndex;
        AnsiString as = MainForm->SideNames->Items->Strings[n];
        unsigned int a = addressByName(as.c_str());
        renameDialog(a);
        updateSideNames();
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::Undefine1Click(TObject *Sender)
{
        int nitems = MainForm->SideNames->Items->Count;
        if (nitems == 0) return;
        int n = MainForm->SideNames->ItemIndex;
        AnsiString as = MainForm->SideNames->Items->Strings[n];
        removeName(as.c_str());
        updateSideNames();
        nitems = MainForm->SideNames->Items->Count;
        if (n >= nitems) n = nitems-1; if (n < 0) n = 0;
        MainForm->SideNames->ItemIndex = n;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::Memorymap1Click(TObject *Sender)
{
        MMapView->Invoke();
}
//---------------------------------------------------------------------------




void __fastcall TMainForm::Calculator1Click(TObject *Sender)
{
        CalcForm->Invoke();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::References1Click(TObject *Sender)
{
        if (! is_online()) return;
        unsigned int a = rpGetAddress(RPC);
        XrefForm->Invoke(a);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::References2Click(TObject *Sender)
{
        if (! is_online()) return;
        unsigned int a = cpGetAddress();
        XrefForm->Invoke(a);
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::Crossreferences1Click(TObject *Sender)
{
        XrefForm->Visible = true;
        XrefForm->BringToFront();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::Bookmarks1Click(TObject *Sender)
{
        BkForm->Visible = true;
        BkForm->BringToFront();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::SpeedButton9Click(TObject *Sender)
{
        BkForm->Visible = true;
        BkForm->BringToFront();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::Bookmark1Click(TObject *Sender)
{
        unsigned int a = cpGetAddress();
        addSideEntry(ST_CODE, a, NULL, NULL, 0);
        BkForm->Visible = true;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::Bookmark2Click(TObject *Sender)
{
        unsigned int a = rpGetAddress(RPC);
        addSideEntry(ST_DATA, a, NULL, NULL, 0);
        BkForm->Visible = true;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::Cut1Click(TObject *Sender)
{
        ((TCustomEdit *)eHexSearch)->CutToClipboard();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::Copy3Click(TObject *Sender)
{
        ((TCustomEdit *)eHexSearch)->CopyToClipboard();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::Paste3Click(TObject *Sender)
{
       char key = 0;

        ((TCustomEdit *)eHexSearch)->PasteFromClipboard();
        eHexSearchKeyPress(Sender, key);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::Bookmark3Click(TObject *Sender)
{
        unsigned char what[256], mask[256], data[512];
        AnsiString as;
        char *s;
        int n;

        as = eHexSearch->Text;
        s = as.c_str();
        if (HexSearch == 2) {
                n = scan16m(s, what, mask, sizeof(what));
                if (n == 0 || n > 64) return;
                memcpy(data, what, n);
                memcpy(data+n, mask, n);
                addSideEntry(ST_HSEARCH, 0, NULL, data, n*2);
        } else {
                n = strlen(s);
                if (n == 0 || n > 64) return;
                addSideEntry(ST_TSEARCH, 0, s, NULL, 0);
        }
        BkForm->Visible = true;

}
//---------------------------------------------------------------------------


void __fastcall TMainForm::Clear1Click(TObject *Sender)
{
        clearAllEvents();
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::Deviceinformation1Click(TObject *Sender)
{
        InfoMessage(
                "BootROMaddress=%08X, BootROMsize=%08X\n"
                "FlashAddress=%08X, FlashSize=%08X FlashPageCount=%i\n"
                "IntRAM1address=%08X IntRAM1size=%08X\n"
                "IntRAM2address=%08X IntRAM2size=%08X\n"
                "IntRAM3address=%08X IntRAM3size=%08x\n"
                "ExtRAMaddress=%08X ExtRAMsize=%08X ExtRAMpageCount=%i\n"
                "IOaddress=%08X IOsize=%08X\n"
                "ShadowFlashAddress=%08X ShadowRAMaddress=%08X\n"
                "UseRAM=%08X\n"
                "PoolCount=%i PoolFree=%i\n"
                "HeaderAddr=%08X RelBitsAddr=%08X SnapDataAddr=%08X\n"
                "InternalCodeAddr=%08X TempExecAddr=%08X\n"
                "LoadableCodeAddr=%08X HolderAddr=%08X\n"
                "ZeroPage=%08X LogPage=%08X\n",

                BootROMaddress, BootROMsize,
                FlashAddress, FlashSize, FlashPageCount,
                IntRAM1address, IntRAM1size,
                IntRAM2address, IntRAM2size,
                IntRAM3address, IntRAM3size,
                ExtRAMaddress, ExtRAMsize, ExtRAMpageCount,
                IOaddress, IOsize,
                ShadowFlashAddress, ShadowRAMaddress,
                UseRAM,
                PoolCount, PoolFree,
                HeaderAddr, RelBitsAddr, SnapDataAddr,
                InternalCodeAddr, TempExecAddr,
                LoadableCodeAddr, HolderAddr,
                ZeroPage, LogPage
        );

}
//---------------------------------------------------------------------------


void __fastcall TMainForm::SetSnappoint1Click(TObject *Sender)
{
        if (! is_online()) return;
        WptForm->Invoke(cpGetAddress(), CP.Thumb, SF_SNAP);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::Breakpoints1Click(TObject *Sender)
{
        SnapsForm->Invoke();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::EventTimerTimer(TObject *Sender)
{
        eventMonitor(true);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::Breakpointonwrite1Click(TObject *Sender)
{
        unsigned int s, e, n;

        if (! is_online()) return;

        if (RPC->SelStart <= RPC->SelEnd) {
                s = RPC->SelStart;
                e = RPC->SelEnd;
        } else {
                s = RPC->SelEnd;
                e = RPC->SelStart;
        }

        if (s == NONE) return;

        s &= ~0x3;
        e = (e+4) & ~0x3;

        if ((s & ~0xfff) != ((e-1) & ~0xfff)) {
                WarningMessage("Selected area resides in more than one page");
                return;
        }

        setWatchpoint(s, e);
        reload_dumps();

}
//---------------------------------------------------------------------------

void __fastcall TMainForm::Removewatchpoint1Click(TObject *Sender)
{
        removeWatchpoint();
        reload_dumps();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::Newwatch1Click(TObject *Sender)
{
        NewWatchForm->Invoke(NULL);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ChangeWatch1Click(TObject *Sender)
{
        if (WP.count == 0) return;
        int n = WP.page->Row;
        NewWatchForm->Invoke(&(WP.items[n]));
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::Delete1Click(TObject *Sender)
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

void __fastcall TMainForm::MenuItem2Click(TObject *Sender)
{
        int arow = WP.page->Row;
        if (arow >= WP.count) return;
        watchitem *wi = &(WP.items[arow]);
        rpSetAddress(RPC, wi->Address, true);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MenuItem3Click(TObject *Sender)
{
        unsigned char buf[256];
        unsigned char xdata[16];
        unsigned char xb;
        unsigned short xs;
        unsigned int xi;

        if (! is_online()) return;
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

void __fastcall TMainForm::MenuItem5Click(TObject *Sender)
{
        watches_reload();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MenuItem6Click(TObject *Sender)
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

void __fastcall TMainForm::WatchesListDrawCell(TObject *Sender, int ACol,
      int ARow, TRect &Rect, TGridDrawState State)
{
        drawWPcell(&WP, ACol, ARow, Rect, State);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::switchtoregisters1Click(TObject *Sender)
{
        Registers->SetFocus();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::switchtolog4Click(TObject *Sender)
{
        if (LogWindow->Tag) {
                LogWindow->SetFocus();
        } else {
                Mes->SetFocus();
        }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::switchtodump1Click(TObject *Sender)
{
        Dump2->SetFocus();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::switchtowatches1Click(TObject *Sender)
{
        WatchesList->SetFocus();        
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::switchtoregisters2Click(TObject *Sender)
{
        Registers->SetFocus();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::switchtodump2Click(TObject *Sender)
{
        Dump2->SetFocus();        
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::switchtoasm1Click(TObject *Sender)
{
        if (Pages->ActivePage != SheetCPU) {
                APC->page->SetFocus();
        } else {
                vCPU->SetFocus();
        }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::switchtoregisters3Click(TObject *Sender)
{
        Registers->SetFocus();
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::Copy4Click(TObject *Sender)
{
        Mes->CopyToClipboard();        
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::Copy5Click(TObject *Sender)
{
        LogWindow->CopyToClipboard();        
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::FollowaddressinCPUwindow1Click(TObject *Sender)
{
        rpFollowAddress(RPC, NULL);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::Gotowatchpoint1Click(TObject *Sender)
{
        if (WatchpointAddr != 0) rpSetAddress(RPC, WatchpointAddr, true);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::Bookmark4Click(TObject *Sender)
{
        int nitems = MainForm->SideNames->Items->Count;
        if (nitems == 0) return;
        int n = MainForm->SideNames->ItemIndex;
        AnsiString as = MainForm->SideNames->Items->Strings[n];
        unsigned int a = addressByName(as.c_str());
        addSideEntry(ST_CODE, a, NULL, NULL, 0);
        BkForm->Visible = true;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::SetBreakpoint1Click(TObject *Sender)
{
        if (! is_online()) return;
        unsigned int a = cpGetAddress();
        snap *sn = snapAtAddress(a);
        if (sn == NULL) {
                WptForm->Invoke(cpGetAddress(), CP.Thumb, SF_BREAK);
        } else {
                deleteSnapAt(a);
                cpu_reload();
        }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::Fastsnappoint1Click(TObject *Sender)
{
        if (! is_online()) return;
        unsigned int a = cpGetAddress();
        snap *sn = snapAtAddress(a);
        if (sn == NULL) {
                WptForm->Invoke(cpGetAddress(), CP.Thumb, SF_FAST);
        } else {
                deleteSnapAt(a);
                cpu_reload();
        }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::OpeninDumpwindow1Click(TObject *Sender)
{
        char *p, *pp;

        LogWindow->SelLength = 0;
        int x = LogWindow->CaretPos.x;
        int y = LogWindow->CaretPos.y;
        AnsiString as = LogWindow->Lines->Strings[y];
        p = as.c_str();

        while (x > 0 && isxdigit(p[x-1])) x--;
        unsigned int a = strtoul(p+x, &pp, 16);
        if (pp == p+x) return;
        rpSetAddress(RPC, a, true);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::OpeninCPUwindow1Click(TObject *Sender)
{
        char *p, *pp;

        LogWindow->SelLength = 0;
        int x = LogWindow->CaretPos.x;
        int y = LogWindow->CaretPos.y;
        AnsiString as = LogWindow->Lines->Strings[y];
        p = as.c_str();

        while (x > 0 && isxdigit(p[x-1])) x--;
        unsigned int a = strtoul(p+x, &pp, 16);
        if (pp == p+x) return;
        cpSetAddress(a, true);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::LogWindowMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
        static DWORD ticks;

        int n = GetTickCount() - ticks;
        ticks = GetTickCount();

        if (n < 400) {
                if (Shift.Contains(ssCtrl)) {
                        OpeninCPUwindow1Click(Sender);
                } else {
                        OpeninDumpwindow1Click(Sender);
                }
        }
}
//---------------------------------------------------------------------------



void __fastcall TMainForm::bOffClick(TObject *Sender)
{
        if (! is_online()) return;
        phone_off();
        lostConnection(false);
        disconnect();
        setSystemState(SS_DISCONNECTED);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::AdrTimerTimer(TObject *Sender)
{
        int i, j, ok;
        unsigned char buf[32];
        unsigned char c;

        MainForm->AdrTimer->Enabled = false;

        for (i=0; i<CP.DataLines-1; i++) {

                if (CP.adrtgt[i] == 0) continue;
                if (CP.adrdata[i][0] != 0) continue;
                readMemory(CP.adrtgt[i], buf, 32);
                buf[31] = 0;

                if (CP.adrtgt[i] & 1 == 1 && buf[0] == 0xb5) {

                        sprintf(CP.adrdata[i], "; (thumb proc)");

                } else {

                        ok = 1;
                        for (j=0; j<31; j++) {
                                c = buf[j];
                                if (c == 0) break;
                                if (c < 32) ok = 0;
                                if (c > 127) ok = 0;
                        }
                        if (j < 2) ok = 0;

                        if (ok == 1) {
                                sprintf(CP.adrdata[i], "; \"%s%s\"", buf, (strlen(buf) == 31) ? "..." : "");
                                CP.adrdata[i][31] = 0;
                        } else {
                                CP.adrdata[i][0] = ' ';
                                CP.adrdata[i][1] = 0;
                        }
                }

                cHashAdd(CP.adrtgt[i], CP.adrdata[i]);
        }
        CP.page->Repaint();

}

//---------------------------------------------------------------------------
void __fastcall TMainForm::Enablepatch1Click(TObject *Sender)
{
        patch *p;

        p = getPatchByAddress(rpGetAddress(RPC));
        if (p == NULL)
                { WarningMessage("No patch at selected address"); return; }

        enablePatch(p);
        PatchesForm->reloadList();
        reload_dumps();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::DisablePatch1Click(TObject *Sender)
{
        patch *p;

        p = getPatchByAddress(rpGetAddress(RPC));
        if (p == NULL)
                { WarningMessage("No patch at selected address"); return; }

        disablePatch(p);
        PatchesForm->reloadList();
        reload_dumps();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::EnableAllPatches1Click(TObject *Sender)
{
        patch **pp;
        int i, n;

        n = getPatchCount();
        pp = getPatchList();
        for (i=0; i<n; i++) {
                enablePatch(pp[i]);
        }
        PatchesForm->reloadList();
        reload_dumps();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::DisableAllPatches1Click(TObject *Sender)
{
        patch **pp;
        int i, n;

        n = getPatchCount();
        pp = getPatchList();
        for (i=0; i<n; i++) {
                disablePatch(pp[i]);
        }
        PatchesForm->reloadList();
        reload_dumps();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::FlashMapViewer1Click(TObject *Sender)
{
        MapForm->Invoke();        
}
//---------------------------------------------------------------------------



void __fastcall TMainForm::NucleusRTOSresources1Click(TObject *Sender)
{
        NuForm->Invoke();
}
//---------------------------------------------------------------------------






void __fastcall TMainForm::ProgramReset1Click(TObject *Sender)
{
        programReset();
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::Copy2Click(TObject *Sender)
{
        copyAsmText();
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::CopyBinary1Click(TObject *Sender)
{
        copyAsmBinary();        
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::Paste2Click(TObject *Sender)
{
        pasteAsmBinary();        
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::RegistersDblClick(TObject *Sender)
{
        int num, row, col;
        unsigned int addr;

        col = Registers->Col;
        row = Registers->Row;
        num = regnum(col, row);
        if (num >= 0) {
                addr = getRegister(num);
        } else if (col == 1) {
                addr = getStackEntry(15-row);
        } else {
                addr = NONE;
        }

        if (shiftState() != 0) {
                if (addr != NONE) rpSetAddress(RPC, getRegister(num), true);
        } else if (ctrlState() != 0) {
                if (addr != NONE) cpSetAddress(getRegister(num), true);
        } else {
                RegistersEdit2Click(Sender);
        }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ViewItem1Click(TObject *Sender)
{
        if (Registers->Col <= 1) {
                int num = Registers->Col * 8 + Registers->Row;
                rpSetAddress(RPC, getRegister(num), true);
        }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::FollowinCPUWindow1Click(TObject *Sender)
{
        if (Registers->Col <= 1) {
                int num = Registers->Col * 8 + Registers->Row;
                cpSetAddress(getRegister(num), true);
        }
}
//---------------------------------------------------------------------------

void changeRegistersPopup() {

        if (MainForm->Registers->Col <= 1) {
                MainForm->RegistersEdit2->Caption = "Edit";
                MainForm->RegistersEdit2->Enabled = (SystemState == SS_INTBRK || SystemState == SS_EXTBRK);
                MainForm->ViewItem1->Enabled = true;
                MainForm->FollowinCPUWindow1->Enabled = true;
        } else {
                MainForm->RegistersEdit2->Caption = "Toggle";
                MainForm->RegistersEdit2->Enabled = (SystemState == SS_INTBRK || SystemState == SS_EXTBRK);
                MainForm->ViewItem1->Enabled = false;
                MainForm->FollowinCPUWindow1->Enabled = false;
        }
}

void __fastcall TMainForm::RegistersMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
        POINT pt;
        int row, col;

        Registers->MouseToCell(X, Y, col, row);
        if (col < 0 || row < 0 > col >= Registers->ColCount || row >= Registers->RowCount) return;
        Registers->Row = row;
        Registers->Col = col;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::RegistersSelectCell(TObject *Sender, int ACol,
      int ARow, bool &CanSelect)
{
        changeRegistersPopup();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::RegistersPopupPopup(TObject *Sender)
{
        changeRegistersPopup();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::RegistersEdit2Click(TObject *Sender)
{
        int row, col;

        if (SystemState != SS_INTBRK && SystemState != SS_EXTBRK) return;
        col = Registers->Col;
        row = Registers->Row;
        editRegister(col, row);
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::NewOriginHere1Click(TObject *Sender)
{
        unsigned int a;

        if (SystemState != SS_INTBRK && SystemState != SS_EXTBRK) return;
        a = cpGetAddress();
        updateRegister(15, CP.Thumb ? a|1 : a);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::GotoOrigin1Click(TObject *Sender)
{
        if (SystemState == SS_INTBRK || SystemState == SS_EXTBRK) {
                cpSetAddress(CurrentThumb ? CurrentAddr|1 : CurrentAddr, true);
        }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::Trace1Click(TObject *Sender)
{
        run(true);        
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::cbComPortDropDown(TObject *Sender)
{
        rescanCommPorts();
}
//---------------------------------------------------------------------------



void __fastcall TMainForm::FormKeyPress(TObject *Sender, char &Key)
{
        if (Key == 0x1b && shiftState() != 0) {
                if (SystemState == SS_DISCONNECTED) {
                        bConnectClick(Sender);
                } else {
                        bDisconnectClick(Sender);
                }
        }

}
//---------------------------------------------------------------------------

void __fastcall TMainForm::Panel6Resize(TObject *Sender)
{
        Registers->RowCount = (Registers->ClientHeight) / Registers->DefaultRowHeight;        
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::Terminal1Click(TObject *Sender)
{
        if (! is_online()) return;
        TerminalForm->Invoke();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::bVkpClick(TObject *Sender)
{
        ApplyVKP1Click(Sender);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::Memorizeaspatch1Click(TObject *Sender)
{
        unsigned int s, e, n;
        unsigned char *buf;

        if (! is_online()) return;

        if (RPC->SelStart <= RPC->SelEnd) {
                s = RPC->SelStart;
                e = RPC->SelEnd;
        } else {
                s = RPC->SelEnd;
                e = RPC->SelStart;
        }

        if (s == NONE) return;
        n = e-s+1;
        if (n > 65536) {
                WarningMessage("Block is too large");
                return;
        }

        buf = (char *)malloc(n);
        if (! readMemory(s, buf, n)) {
                WarningMessage("Error reading memory");
                return;
        }

        addPatch(s, buf, buf, n, SM_ISOLATED);
        free(buf);

        PatchesForm->reloadList();
}
//---------------------------------------------------------------------------

