#include <vcl.h>
#include <stdio.h>
#include "DForm.h"
#include "Config.h"
#include "EventLog.h"

static int EvtCount = 0;

void newEvent(char marker, char *text) {

        TRichEdit *W = MainForm->LogWindow;
        int color;
//        char c2[4] = { 0x0a, 0x20, 0x20, 0 };
        char c2[16];
        char *buf = strdup(text);
        char *p = buf;
        char *ep = buf + strlen(buf);
        bool firstline = true;
        char *pp;

        W->Lines->BeginUpdate();

        while (p < ep) {
                pp = strchr(p, '\n');
                if (pp == NULL) pp = p+strlen(p);
                *pp = 0;
                if (*(pp-1) == '\r') *(pp-1) = 0;

                W->SelStart = 9999999;
                c2[1] = firstline ? marker : ' ';
                if (firstline) {
                        if (marker == '*') {
                                EvtCount++;
                                sprintf(c2, "\n%c %i ", marker, EvtCount);
                        } else {
                                sprintf(c2, "\n%c ", marker);
                        }
                        switch (marker) {
                                case '>':
                                        color = 0x00c000;
                                        break;
                                case '*':
                                        color = 0xff0000;
                                        break;
                                case 'x':
                                        color = 0x0000ff;
                                        break;
                                case '+':
                                case '-':
                                        color = 0xff00ff;
                                        break;
                                case '!':
                                        color = 0x37aeff;
                                        break;
                                default:
                                        color = 0;
                                        break;
                        }
                } else {
                        strcpy(c2, "\n  ");
                }
                W->SelAttributes->Style = TFontStyles() << fsBold;
                W->SelAttributes->Color = (TColor) color;
                W->SetSelTextBuf(c2);
                W->SelAttributes->Style = TFontStyles();
                W->SelAttributes->Color = (TColor) 0;
                W->SelStart = W->SelStart + W->SelLength;
                W->SetSelTextBuf(p);
                W->SelStart = W->SelStart + W->SelLength;
                if (Logfd != NULL) {
                        fputs(c2, Logfd);
                        fputs(p, Logfd);
                }
                firstline = false;
                p = pp+1;
        }

        W->BringToFront();
        W->SelStart = (W->Text.Length());
        SendMessage(W->Handle, 0xB7, 0, 0);
        W->Lines->EndUpdate();
        free(buf);

}

void repaintEventLog() {

        MainForm->LogWindow->Repaint();

}

void clearAllEvents() {

        MainForm->LogWindow->Clear();

}


