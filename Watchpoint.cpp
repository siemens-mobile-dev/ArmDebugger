//---------------------------------------------------------------------------

#include <vcl.h>
#include <stdio.h>
#pragma hdrstop

#include "Debug.h"
#include "Pages.h"
#include "Disarm.h"
#include "Utility.h"
#include "Snappoints.h"
#include "Watchpoint.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TWptForm *WptForm;
//---------------------------------------------------------------------------
__fastcall TWptForm::TWptForm(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------

int insType(char *s) {

        int i;

        char *t1[] = {
                "ADC", "ADD", "AND", "BIC", "EOR", "MUL", "ORR", "RSB",
                "RSC", "SBC", "SUB", "ASR", "LSL", "LSR", "ROR", NULL
        };
        char *t2[] = {
                "CLZ", "LDR", "MLA", "MOV", "MRC", "MRS", "MVN", "SWP",
                "NEG", NULL
        };
        char *t3[] = {
                "SMLAL", "SMULL", "UMLAL", "UMULL", NULL
        };

        for (i=0; t1[i] != NULL; i++)
                if (strncmp(s, t1[i], strlen(t1[i])) == 0) return 1;
        for (i=0; t2[i] != NULL; i++)
                if (strncmp(s, t2[i], strlen(t2[i])) == 0) return 2;
        for (i=0; t3[i] != NULL; i++)
                if (strncmp(s, t3[i], strlen(t3[i])) == 0) return 3;
        return 0;
}

void genText(unsigned int addr, unsigned int data, bool is_thumb, char *buf) {

        instruction *ins;
        unsigned int r[18];
        char *s, *s2, *p;
        int rnums[16];
        int nregs, nops, i, is, j, n;
        bool f;

        ins = is_thumb ?  disthumb(addr, data, r) : disarm(addr, data, r);

        nregs = 0;
        s = ins->resolved;
        while (*s) {
                n = -1;
                if (isalnum(*(s-1))) { s++; continue; }
                f = (*(s-1) == '-');
                if (*s == 'R') {
                        n = strtoul(s+1, &p, 10);
                        if (p == s+1 || n > 15) n = -1;
                        s = p;
                } else if (*s == 'L' && *(s+1) == 'R' && ! isalnum(*(s+2))) {
                        n = 14;
                        s += 2;
                } else {
                        s++;
                }

                if (n != -1) {
                        if (f && nregs > 0) {
                                for (i=rnums[nregs-1]; i<n; i++) {
                                        rnums[nregs++] = i;
                                }
                        }
                        rnums[nregs++] = n;
                }
        }

        sprintf(buf, "%%A: <%s> ", ins->resolved);

        s = ins->resolved;
        nops = 1;
        while (*s) {
                if (*s == ',') {
                        nops++;
                } else if (*s == '[') {
                        while (*s && *s != ']') s++;
                        s--;
                }
                s++;
        }

        s = ins->resolved;
        if (strncmp(s, "BL", 2) == 0) {

                while (*s && *s != '\t') s++;
                if (*s) s++;
                s2 = s;
                while (*s2 && *s2 != ' ') s2++;
                *s2 = 0;
                sprintf(buf, "%%A: %s(%%R0,%%R1,%%R2,%%R3)\n", s);

        } else if (strncmp(s, "MRS", 3) == 0) {

                sprintf(buf+strlen(buf), "Flags=%%C");

        } else {
                is = 0;
                switch (insType(s)) {
                        case 1:
                                if (nops > 2) is++;
                                break;
                        case 2:
                                is++;
                                break;
                        case 3:
                                is+=2;
                                break;
                }
                for (i=is; i<nregs; i++) {
                        for (j=is; j<i; j++) {
                                if (rnums[j] == rnums[i]) break;
                        }
                        if (j == i) sprintf(buf+strlen(buf), "R%i=%%R%i ", rnums[i], rnums[i]);
                }
        }

        if ((data & 0xffffc000) == 0xe92d4000 && ! is_thumb) {
                sprintf(buf, "%%A(%%R0,%%R1,%%R2,%%R3) [LR=%%R14]");
        } else if ((data & 0xff00) == 0xb500 && is_thumb) {
                sprintf(buf, "%%A(%%R0,%%R1,%%R2,%%R3) [LR=%%R14]");
        }

        s = buf;
        while (*s) {
                if (*s == '\t') *s = ' ';
                s++;
        }
}

bool TWptForm::Invoke(unsigned int addr, bool is_thumb, int type) {

        char buf[4096], buf2[64], *p, *pp;
        unsigned int ins;
        snap **sn;
        bool update = false;
        int i, num;
        int mr;

        /*
        if (addr < FlashAddress || addr > FlashAddress+FlashSize-1) {
                return WarningMessage("Cannot set snappoint in non-flash area");
        }
        */

        sn = getSnapList();
        for (i=1; i<16; i++) {
                if (sn[i] != NULL && sn[i]->addr == addr) {
                        update = true;
                        num = i;
                        break;
                }
        }

        if (update) {
                ins = sn[num]->instruction;
        } else {
                if (freeSnap() == -1) return WarningMessage("Cannot set more than 15 snappoints");
                if (! readMemory(addr, (unsigned char *)&ins, 4)) {
                        return WarningMessage("Cannot read instruction");
                }
        }

        if (! canSetSnap(ins, is_thumb)) return WarningMessage("Cannot set snappoint on this instruction");

        sprintf(buf, "%s at %08X (%s)",
                (type == SF_BREAK) ? "Breakpoint" : "Snappoint",
                addr,
                is_thumb ? "Thumb" : "ARM");
        Caption = buf;
        ActiveControl = RtEdit;

        if (update) {
                strcpy(buf, sn[num]->text);
        } else {
                if (type == SF_BREAK) {
                        sprintf(buf , "~");
                } else if (type == SF_FAST) {
                        sprintf(buf , "%%A: CPSR=%%C\n%%RR");
                } else {
                        genText(addr, ins, is_thumb, buf);
                }
        }
        RtEdit->Lines->SetText(buf);

        if (update) {
                eCond->Text = sn[num]->condition;
        } else {
                eCond->Text = "";
        }

        while (1) {
                mr = (type == SF_SNAP) ? ShowModal() : mrOk;
                if (mr == mrCancel) return false;
                if (mr == mrNo) {
                        if (update) deleteSnap(num);
                        break;
                }
                if (update) deleteSnap(num);
                strcpy(buf, RtEdit->Lines->GetText());
                p = buf+strlen(buf)-1;
                while (p > buf && (*p == '\r' || *p == '\n')) *(p--) = 0;
                strcpy(buf2, eCond->Text.c_str());
                if (addSnap(addr, is_thumb, buf, buf2, SP_NEW)) break;
        }

        SnapsForm->updateSnaps();
        reload_dumps();
        return true;

}

void __fastcall TWptForm::RtEditKeyPress(TObject *Sender, char &Key)
{
        if (Key == 0x1b) { Key = 0; ModalResult = mrCancel; }
        if (Key == 0x0a && GetKeyState(VK_CONTROL) != 0) { Key = 0; ModalResult = mrOk;}
}
//---------------------------------------------------------------------------




