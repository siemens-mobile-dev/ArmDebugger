//---------------------------------------------------------------------------

#include <vcl.h>
#include <stdio.h>
#include "BootWindow.h"
#include "DForm.h"
#include "Config.h"
#include "comm.h"
#include "Utility.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TBootForm *BootForm;

char *BootCmd[5];
dataline *lines = NULL;
int linescount = 0;
int totalBytes = 0;

static TEdit *BootEdit[5];
static TButton *ExecCmd[5];
static unsigned char *Buffer;
static int Length;
static int Bytes;
static bool CancelQuery = false;

//---------------------------------------------------------------------------
__fastcall TBootForm::TBootForm(TComponent* Owner)
        : TForm(Owner)
{
        BootEdit[0] = BootCmd0;
        BootEdit[1] = BootCmd1;
        BootEdit[2] = BootCmd2;
        BootEdit[3] = BootCmd3;
        BootEdit[4] = BootCmd4;
        ExecCmd[0] = ExecCmd0;
        ExecCmd[1] = ExecCmd1;
        ExecCmd[2] = ExecCmd2;
        ExecCmd[3] = ExecCmd3;
        ExecCmd[4] = ExecCmd4;
}
//---------------------------------------------------------------------------
void clearData() {

        if (lines) free(lines);
        lines = NULL;
        linescount = 0;
        BootForm->Answer->RowCount = 1;
        BootForm->Answer->Repaint();

}

void appendData(char dir, unsigned char *buf, int len) {

        dataline *cline;
        int n;
        unsigned char *p;

        p = buf;
        if (linescount > 0) {
                cline = &lines[linescount-1];
                if (cline->direction == dir && cline->count < 16) {
                        n = 16 - cline->count;
                        if (n > len) n = len;
                        memcpy(&cline->data[cline->count], p, n);
                        cline->count += n;
                        p += n;
                        len -= n;
                }
        }

        while (len > 0) {
                linescount++;
                lines = (dataline *)realloc(lines, linescount * sizeof(dataline));
                cline = &lines[linescount-1];
                cline->direction = dir;
                n = (len > 16) ? 16 : len;
                memcpy(cline->data, p, n);
                cline->count = n;
                p += n;
                len -= n;
        }

        BootForm->Answer->RowCount = linescount;
        BootForm->Answer->Row = linescount - 1;
        BootForm->Answer->Repaint();

}

void writeDataToFile(FILE *F) {

        dataline *cline;
        int i, j;
        unsigned char c;

        for (i=0; i<linescount; i++) {
                cline = &lines[i];
                fprintf(F, "%s ", (cline->direction == '>') ? "->" : "<-");
                for (j=0; j<16; j++) {
                        if (cline->count > j) {
                                fprintf(F, "%02X ", cline->data[j]);
                        } else {
                                fprintf(F, "   ");
                        }
                        if (j == 7 || j == 15) fprintf(F, " ");
                }
                for (j=0; j<16; j++) {
                        c = cline->data[j];
                        if (cline->count > j) {
                                fprintf(F, "%c", (c >= 32 && c <= 127) ? c : '.');
                        }
                }
                fprintf(F, "\n");
        }

}

void reset() {

        BootForm->bSave->Enabled = false;
        totalBytes = 0;
        clearData();
        BootForm->Timer2->Enabled = false;
        BootForm->Timer1->Enabled = true;
        for (int i=0; i<5; i++) ExecCmd[i]->Enabled = false;
        BootForm->StatusBar1->SimpleText = "Scanning...";
        setSpeed(115200);

}

void realClose() {

        AnsiString as;
        int i;

        for (i=0; i<5; i++) {
                as = BootEdit[i]->Text;
                if (BootCmd[i]) free(BootCmd[i]);
                BootCmd[i] = strdup(as.c_str());
        }

        BootForm->Timer1->Enabled = false;
        BootForm->Timer2->Enabled = false;
        MainForm->Enabled = true;
        inprocess = false;
        BootForm->Hide();

}


void sendCmd(char *cmd) {

        unsigned char data[16384], *p;
        char buf[4], c;
        int n;

        p = data;
        while (*cmd) {
                c = *cmd;
                if (c == ' ' || c == '\t') { cmd++; continue; }
                if (c == '\"') {
                        cmd++;
                        while (*cmd && *cmd != '\"') {
                                c = *cmd;
                                if (c == '\\' && *(cmd+1)) {
                                        c = *(++cmd);
                                        switch (c) {
                                                case 'r':
                                                        *(p++) = '\r';
                                                        break;
                                                case 'n':
                                                        *(p++) = '\n';
                                                        break;
                                                case 't':
                                                        *(p++) = '\t';
                                                        break;
                                                default:
                                                        *(p++) = c;
                                                        break;
                                        }
                                } else {
                                        *(p++) = c;
                                }
                                cmd++;
                        }
                        if (*cmd == '\"') cmd++;
                } else if (isxdigit(c) && isxdigit(*(cmd+1))) {
                        buf[0] = c;
                        buf[1] = *(cmd+1);
                        buf[2] = 0;
                        *(p++) = (unsigned char)strtoul(buf, NULL, 16);
                        cmd += 2;
                } else {
                         WarningMessage("Error parsing command");
                         return;
                }
        }

        n = p - data;
        writecom(data, n);
        appendData('>', data, n);

}

void TBootForm::Invoke(unsigned char *buf, int len) {

        int i, w;

        Buffer = buf;
        Length = len;
        CancelQuery = false;

        w = 19;
        Answer->ColWidths[0] = w;
        for (i=1; i<8; i++) Answer->ColWidths[i] = w;
        Answer->ColWidths[8] = w + w/2;
        for (i=9; i<16; i++) Answer->ColWidths[i] = w;
        Answer->ColWidths[16] = w + w/2;
        for (i=17; i<=32; i++) Answer->ColWidths[i] = 8;

        for (i=0; i<5; i++) BootEdit[i]->Text = BootCmd[i];

        reset();

        MainForm->Enabled = false;
        inprocess = true;
        Show();
}

void __fastcall TBootForm::bCloseClick(TObject *Sender)
{
        CancelQuery = true;
}

void __fastcall TBootForm::bResetClick(TObject *Sender)
{
        reset();
}
//---------------------------------------------------------------------------
void __fastcall TBootForm::FormCloseQuery(TObject *Sender, bool &CanClose)
{
        CancelQuery = true;
        CanClose = false;
}
//---------------------------------------------------------------------------
void __fastcall TBootForm::Timer1Timer(TObject *Sender)
{
        int s;
        unsigned char b[256];

        if (CancelQuery) {
                realClose();
                return;
        }

        setTimeout(100);

        writecom("AT", 2);
        if (readcom(b, 1) <= 0)
                return;

        if (b[0] != 0xb0 && b[0] != 0xc0) {
                readcom(b, 256);
                return;
        }

        StatusBar1->SimpleText = "Sending loader...";
        StatusBar1->Repaint();

        setTimeout(2000);
        writecom(Buffer, Length);

        if (readcom(b, 1) <= 0 || (b[0] != 0xb1 && b[0] != 0xc1)) {
                WarningMessage("No answer from bootstrap loader");
                return;
        }

        StatusBar1->SimpleText = "Reading...";
        StatusBar1->Repaint();

        for (int i=0; i<5; i++) ExecCmd[i]->Enabled = true;
        Timer1->Enabled = false;
        Timer2->Enabled = true;
}

//---------------------------------------------------------------------------
void __fastcall TBootForm::Timer2Timer(TObject *Sender)
{
        unsigned char b[16];
        int i, n, s, nr;

        if (CancelQuery) {
                realClose();
                return;
        }

        setTimeout(1);

        n = readcom(b, 16);
        if (n <= 0) return;

        appendData('<', b, n);

        totalBytes += n;
        sprintf(b, "Reading...   %i bytes", totalBytes);
        StatusBar1->SimpleText = (char *)b;
        bSave->Enabled = true;
}
//---------------------------------------------------------------------------
void __fastcall TBootForm::bSaveClick(TObject *Sender)
{
        FILE *F;
        char *name;

        if (BootSave->Execute()) {
                name = BootSave->FileName.c_str();
                F = fopen(name, "wb");
                if (F == NULL) {
                        MessageBox(NULL, "Cannot open file", "Error", MB_OK|MB_ICONERROR);
                        return;
                }

                writeDataToFile(F);
                fclose(F);
        }
}
//---------------------------------------------------------------------------

void __fastcall TBootForm::AnswerDrawCell(TObject *Sender, int ACol,
      int ARow, TRect &Rect, TGridDrawState State)
{
        TCanvas *can;
        char buf[16];
        dataline *cline;
        unsigned char c;

        can = Answer->Canvas;
        can->Brush->Color = (TColor)0xffffff;
        can->FillRect(Rect);

        if (linescount == 0) return;

        cline = &lines[ARow];
        can->Font = Answer->Font;
        can->Font->Color = (cline->direction == '>') ? (TColor)0x0000ff : (TColor)0xff0000;


        if (ACol == 0) {
                buf[0] = cline->direction;
                buf[1] = 0;
        } else if (ACol >= 1 && ACol <= 16) {
                if (cline->count < ACol) return;
                c = cline->data[ACol-1];
                sprintf(buf, "%02X", c);
        } else {
                if (cline->count < ACol-16) return;
                c = cline->data[ACol-17];
                buf[0] = (c >= 32 && c <= 127) ? c : '.';
                buf[1] = 0;
        }
        can->TextOut(Rect.Left+1, Rect.Top, buf);

}
//---------------------------------------------------------------------------

void __fastcall TBootForm::ExecCmd0Click(TObject *Sender)
{
        int n = ((TComponent *)Sender)->Tag;
        sendCmd(BootEdit[n]->Text.c_str());
}
//---------------------------------------------------------------------------
void __fastcall TBootForm::BootCmd0KeyPress(TObject *Sender, char &Key)
{
        int n = ((TComponent *)Sender)->Tag;
        if (Key == 0x0d) {
                sendCmd(BootEdit[n]->Text.c_str());
                Key = 0;
        }
}
//---------------------------------------------------------------------------

