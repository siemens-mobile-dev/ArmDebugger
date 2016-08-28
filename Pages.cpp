#include <stdio.h>
#include <vcl.h>
#include "DForm.h"
#include "Pages.h"
#include "comm.h"
#include "Wire.h"
#include "Debug.h"
#include "Disarm.h"
#include "Config.h"
#include "GotoWindow.h"
#include "Utility.h"
#include "Globals.h"

asmpage AP[9], *APC;
rampage RP[2], *RPC;
cpupage CP;
watchpage WP;

static unsigned int REG[16], REG_S[16];
static unsigned int CPSR, CPSR_S;
static int REG_COUNT = 0;
static unsigned int STACK[16];
static bool STACK_FILLED;
static int RRC=1;

void apInit() {

        AP[0].page = MainForm->AsmText1;
        AP[0].sheet = MainForm->Sheet1;
        AP[1].page = MainForm->AsmText2;
        AP[1].sheet = MainForm->Sheet2;
        AP[2].page = MainForm->AsmText3;
        AP[2].sheet = MainForm->Sheet3;
        AP[3].page = MainForm->AsmText4;
        AP[3].sheet = MainForm->Sheet4;
        AP[4].page = MainForm->AsmText5;
        AP[4].sheet = MainForm->Sheet5;
        AP[5].page = MainForm->AsmText6;
        AP[5].sheet = MainForm->Sheet6;
        AP[6].page = MainForm->AsmText7;
        AP[6].sheet = MainForm->Sheet7;
        AP[7].page = MainForm->AsmText8;
        AP[7].sheet = MainForm->Sheet8;
        AP[8].page = MainForm->AsmText9;
        AP[8].sheet = MainForm->Sheet9;

}

//---------------------------------------------------------------------------

// get offset of current address in Data array
unsigned int rpGetOffset(rampage *rp) {

        int x = rp->page->Col;
        if (x > 0 && x <= 16) x--; else if (x >= 17) x -= 17;
        return (rp->page->Row - rp->page->TopRow) * 0x10 + x;

}

//---------------------------------------------------------------------------

unsigned int rpGetAddress(rampage *rp) {

        int x = rp->page->Col;
        if (x > 0 && x <= 16) x--; else if (x >= 17) x -= 17;
        return rp->BaseAddress + rp->page->Row * 0x10 + x;

}

//---------------------------------------------------------------------------


void rpSetAddress(rampage *rp, unsigned int addr, bool save) {

        unsigned int top, saddr;
        TDrawGrid *page;
        memoryinfo mi;

        if (rp == NULL) {
                cpSetAddress(addr, save);
                return;
        }

        page = rp->page;
        getMemoryInfo(addr, &mi);
        if (mi.len == 0) return;

        if (save) {
                saddr = rpGetAddress(rp);
                if (rp->a_pos >= 254) {
                        memmove(&(rp->a_history[0]), &(rp->a_history[1]), 254 * sizeof(unsigned int));
                        rp->a_pos--;
                }
                rp->a_history[rp->a_pos++] = saddr;
        }

        top = addr & ~0x0f;
        rp->Freeze = true;
        rp->BaseAddress = mi.addr;
        rp->Size = mi.len;
        rp->Type = mi.type;
        rp->SelStart = rp->SelEnd = addr;
        page->RowCount = mi.len / 0x10;
        page->TopRow = (top - mi.addr) / 0x10;
        page->Row = page->TopRow;
        page->Col = (addr & 0x0f) + 1;
        rp->Freeze = false;
        dump_reload(rp, true);
        return;


}

//---------------------------------------------------------------------------

void rpFollowAddress(rampage *rps, rampage *rpd) {

        unsigned char *data;
        unsigned int addr, offset;

        if (! is_online()) return;
        if (rps->page->Col == 0 || rps->page->Col > 16) return;

        data = rps->Data;
        offset = rpGetOffset(rps) & ~0x03;
        addr = data[offset] + (data[offset+1] << 8) + (data[offset+2] << 16) +(data[offset+3] << 24);
        if (rpd != NULL) {
                rpSetAddress(rpd, addr, true);
        } else {
                cpSetAddress(addr, true);
        }

}

//---------------------------------------------------------------------------

void rpGoto(rampage *rp, int val) {

        char buf[2], *p;
        unsigned int addr;

        buf[0] = (val < 0) ? '\0' : digits[val];
        buf[1] = '\0';
        if (GotoForm->Invoke("Goto address", buf, &addr)) {
                rpSetAddress(rp, addr, true);
        }
}

//---------------------------------------------------------------------------

void dump_reload(rampage *rp, bool unconditional) {

        unsigned int addr, lines;
        unsigned int s_addr, s_lines;
        unsigned int read_addr, read_top, read_lines;
        unsigned int copy_from, copy_to, copy_lines;

        if (! is_online()) return;
        if (rp->Freeze) return;

        lines = rp->DataLines = rp->page->VisibleRowCount + 1;
        if (rp->page->TopRow + rp->DataLines > rp->page->RowCount)
                rp->DataLines--;
        addr = rp->BaseAddress + rp->page->TopRow * 0x10;
        s_addr = rp->SaveAddress;
        s_lines = rp->SaveLines;

        if (unconditional) {
                read_addr = addr;
                read_top = 0;
                read_lines = lines;
                copy_lines = 0;
        } else if (addr < s_addr) {
                read_addr = addr;
                read_top = 0;
                read_lines = (s_addr - addr) / 0x10;
                if (read_lines > lines) read_lines = lines;
                copy_from = 0;
                copy_to = read_lines;
                copy_lines = lines - read_lines;
        } else {
                read_addr = s_addr + s_lines * 0x10;
                if (read_addr < addr) read_addr = addr;
                read_top = (read_addr - addr) / 0x10;
                if (read_top > lines) read_top = lines;
                read_lines = lines - read_top;
                copy_from = (addr - s_addr) / 0x10;
                copy_to = 0;
                copy_lines = read_top;
        }

        if (copy_lines > 0) {
                memmove(rp->Data + copy_to * 0x10, rp->Data + copy_from * 0x10, copy_lines * 0x10);
        }

        if (read_lines > 0) {
                readMemory(read_addr, rp->Data + read_top * 0x10, read_lines * 0x10);
        }

        rp->SaveAddress = addr;
        rp->SaveLines = lines;
        rp->page->Repaint();

}

//---------------------------------------------------------------------------

void reload_dumps() {

        dump_reload(&RP[0], true);
        dump_reload(&RP[1], true);
        cpu_reload();
        watches_reload();

}

//---------------------------------------------------------------------------

void drawRPcell(rampage *rpc, int acol, int arow, TRect &rect, TGridDrawState state) {

        TCanvas *can;
        unsigned int addr, offset, offset_4;
        int toprow;
        unsigned char *data;
        unsigned char c;
        TRect rect2, *wrect;
        bool sel, focus;
        char buf[64];


        data = rpc->Data;
        can = rpc->page->Canvas;
        toprow = rpc->page->TopRow;
        can->Brush->Color = (TColor)0xffffff;
        can->Font = rpc->page->Font;
        can->Font->Color = (TColor)0;

        if (acol == 0) {
              addr = rpc->BaseAddress + arow * 0x10;
              sprintf(buf, "%08X", addr);
              can->FillRect(rect);
              can->TextOut(rect.Left+1, rect.Top, buf);
              if (state.Contains(gdFocused)) {
                can->Brush->Color = RECT_COLOR;
                can->FrameRect(rect);
              }
              return;
        }

        if ( ! is_online() || ! is_connected() || (arow - toprow >= rpc->DataLines) ) {
                can->FillRect(rect);
                return;
        }

        focus = state.Contains(gdFocused);

        if (acol <= 16) {

                rect2.left = rect.left;
                rect2.top = rect.top;
                rect2.right = rect.left + rpc->page->ColWidths[1];
                rect2.bottom = rect.bottom;

                addr = rpc->BaseAddress + arow * 0x10 + (acol - 1);
                if (isChanged(addr)) can->Font->Color = (TColor)0x0000f8;
                if (rpc->SelStart <= rpc->SelEnd) {
                        sel = (addr >= rpc->SelStart && addr <= rpc->SelEnd);
                        wrect = (addr == rpc->SelEnd) ? &rect2 : &rect;
                } else {
                        sel = (addr >= rpc->SelEnd && addr <= rpc->SelStart);
                        wrect = (addr == rpc->SelStart) ? &rect2 : &rect;
                }

                if (rpc->InEdit && sel) {
                        can->FillRect(*wrect);
                        buf[0] = digits[rpc->EditValue];
                        buf[1] = '\0';
                        can->TextOut(rect.left+1, rect.top, buf);
                        return;
                }

                offset = (arow - toprow)* 0x10 + (acol - 1);
                offset_4 = offset | 0x03;
                c = data[offset_4];

                if ((c | 0x01) == 0xa1) {  // flash
                        can->Brush->Color = (TColor)0xfff0d8;
                } else if (c == 0xa8) {    // RAM
                        can->Brush->Color = (TColor)0xd0ffff;
                } else if((c == 0x00) && (data[offset_4-1] == 0x08)) {
                        can->Brush->Color = (TColor)0xd0ffff;
                } else {
                        can->Brush->Color = (TColor)0xffffff;
                }
                if (addr >= WatchpointAddr && addr < WatchpointAddr+WatchpointLen) {
                        can->Brush->Color = (TColor)0x0000a0;
                        can->Font->Color = (TColor)0xffffff;
                }
                if (sel || focus) can->Brush->Color = SEL_COLOR;

                can->FillRect(*wrect);

                c = data[offset];
                buf[0] = digits[c >> 4];
                buf[1] = digits[c & 0x0f];
                buf[2] = '\0';
                can->TextOut(rect.left+1, rect.top, buf);

                if (focus) {
                        can->Brush->Color = RECT_COLOR;
                        can->FrameRect(rect2);
                }

        } else {

                can->FillRect(rect);
                offset = (arow - toprow)* 0x10 + (acol - 17);
                c = data[offset];
                if (c < ' ' || c > 0x7f) {
                        can->Font->Color = (TColor)0xa0a0a0;
                        c = '.';
                }
                buf[0] = c;
                buf[1] = '\0';
                can->TextOut(rect.left+1, rect.top, buf);

                if (focus) {
                        can->Brush->Color = RECT_COLOR;
                        can->FrameRect(rect);
                }
        }

}

//---------------------------------------------------------------------------

void rpHandleKeypress(rampage *rpc, char &Key) {

        unsigned char buf[16];
        unsigned int addr, offset;
        TDrawGrid *page = rpc->page;
        unsigned char key = Key;

        // ascii editing
        if (page->Col > 16 && key >= 0x20 && key <= 0x7f) {
                addr = rpGetAddress(rpc);
                offset = rpGetOffset(rpc);
                buf[0] = Key;
                setMemory(addr, buf, 1, SM_STICKY);
                readMemory(addr, buf, 1);
                rpc->Data[offset] = buf[0];
                if (page->Col < 32) {
                        page->Col++;
                } else {
                        page->Col = 17;
                        page->Row++;
                }
                return;
        }

        if (key >= 'a' && key <= 'f') key -= 0x20;
        if ((key >= '0' && key <= '9') || (key >= 'A' && key <= 'F')) {
                key -= 0x30;
                if (key > 9) key -= 7;

                // address changing
                if (page->Col == 0) {
                        rpGoto(rpc, key);
                        return;
                }

                // editing available only in connected state
                if (! is_online()) return;

                // hex editing
                if (! rpc->InEdit) {
                        rpc->InEdit = true;
                        rpc->EditValue = key;
                        page->Repaint();
                } else {
                        rpc->InEdit = false;

                        addr = rpGetAddress(rpc);
                        offset = rpGetOffset(rpc);
                        buf[0] = (rpc->EditValue << 4) | key;
                        setMemory(addr, buf, 1, SM_STICKY);
                        readMemory(addr, buf, 1);
                        rpc->Data[offset] = buf[0];
                        if (page->Col < 16) {
                                page->Col++;
                        } else {
                                page->Col = 1;
                                page->Row++;
                        }
                }
        }
}

void cpFollowAddress() {

        unsigned int a, n, offset;
        unsigned int state[18];
        instruction *ins;

        if (! is_online()) return;

        a = cpGetAddress();
        offset = cpGetOffset();
        n = *((unsigned int *)(CP.Data+offset));
        ins = CP.Thumb ? disthumb(a, n, state) : disarm(a, n, state);
        if (ins->is_ldrvalue && ins->has_target) {
                if (inSpace(ins->target_address)) {
                        readMemory(ins->target_address, (unsigned char *)&a, 4);
                        if (inSpace(a)) {
                                ins->is_branch ? cpSetAddress(a, true) : rpSetAddress(RPC, a, true);
                        }
                }
        } else if (ins->is_adr) {
                rpSetAddress(RPC, ins->target_address, true);
        } else if (ins->is_branch && ins->has_target) {
                a = ins->target_address;
                if (ins->target_size == 2) a++;
                cpSetAddress(a, true);
        }

}

void watches_reload() {

        unsigned int idx[64], addrs[64], data[64], buf[1];
        unsigned int addr;
        watchitem *wi;
        int i, j, n;

        if (! WP.Visible) return;
        if (WP.count == 0) return;

        if (! is_online()) {
                for (i=0; i<WP.count; i++) {
                        WP.items[i].Valid = false;
                }
                WP.page->Repaint();
                return;
        }

        for (i=j=0; i<WP.count; i++) {

                wi = &WP.items[i];
                wi->handled = false;
                wi->Valid = false;
                wi->s_realAddress = wi->realAddress;
                memcpy(wi->s_data, wi->data, wi->Length);

                if (j >= 64) continue;
                if (wi->Ref != 0) continue;
                if (wi->Length > 4) continue;

                idx[j] = i;
                addrs[j] = wi->Address;
                j++;

        }

        if (j > 0) {
                if (queryRAM(addrs, data, j)) {
                        for (i=0; i<j; i++) {
                                n = idx[i];
                                WP.items[n].handled = true;
                                WP.items[n].Valid = true;
                                WP.items[n].realAddress = addrs[i];
                                memcpy(WP.items[n].data, &data[i], 4);
                        }
                }
        }


        for (i=0; i<WP.count; i++) {

                wi = &WP.items[i];
                if (wi->handled) continue;
                wi->Valid = false;
                addr = wi->Address;
                if (wi->Ref >= 1) {
                        if (! readMemory(addr, (unsigned char *)buf, 4)) continue;
                        addr = buf[0] + wi->Offset[0];
                }
                if (wi->Ref >= 2) {
                        if (! inSpace(addr)) continue;
                        if (! readMemory(addr, (unsigned char *)buf, 4)) continue;
                        addr = buf[0] + wi->Offset[1];
                }
                if (wi->Ref >= 3) {
                        if (! inSpace(addr)) continue;
                        if (! readMemory(addr, (unsigned char *)buf, 4)) continue;
                        addr = buf[0] + wi->Offset[2];
                }
                if (wi->Ref >= 4) {
                        if (! inSpace(addr)) continue;
                        if (! readMemory(addr, (unsigned char *)buf, 4)) continue;
                        addr = buf[0] + wi->Offset[3];
                }

                if (! inSpace(addr)) continue;
                wi->realAddress = addr;
                if (! readMemory(addr, wi->data, wi->Length)) continue;
                wi->Valid = true;

        }

        for (i=0; i<WP.count; i++) {
                wi = &WP.items[i];
                if (! wi->Valid) continue;
                if (wi->first) {
                        wi->s_realAddress = wi->realAddress;
                        memcpy(wi->s_data, wi->data, wi->Length);
                        wi->first = false;
                }
        }

        WP.page->Repaint();

}

void drawWPcell(watchpage *wpc, int acol, int arow, TRect &rect, TGridDrawState state) {

        TCanvas *can;
        watchitem *wi;
        char buf[2048], tbuf[16], *p;
        unsigned char *bdata;
        unsigned short *hdata;
        unsigned int *idata;
        int i, n, l;
        unsigned short *hp;

        can = wpc->page->Canvas;
        can->Brush->Color = (TColor)0xffffff;
        if (state.Contains(gdFocused)) can->Brush->Color = SEL_COLOR;
        can->Font = DebuggerFont;
        can->Font->Color = (TColor)0;
        can->Font->Style = TFontStyles();

        can->FillRect(rect);
        if (arow >= wpc->count) return;

        wi = &(wpc->items[arow]);
        if (acol == 0) {
                sprintf(buf, "%08X", wi->Address);
                if (wi->Ref >= 1) strcat(buf, "*");
                if (wi->Ref >= 2) strcat(buf, "*");
                if (wi->Ref >= 3) strcat(buf, "*");
                if (wi->Ref >= 4) strcat(buf, "*");
                can->TextRect(rect, rect.Left+2, rect.Top-1, buf);
        } else {
                if (! wi->Valid) {
                        can->Font->Color = (TColor)0xd0d080;
                        can->TextRect(rect, rect.Left+2, rect.Top-1, "<Inaccessible>");
                        return;
                }
                if (wi->realAddress == 0) {
                        can->TextRect(rect, rect.Left+2, rect.Top-1, "NULL");
                        return;
                }
                if (wi->realAddress != wi->s_realAddress) can->Font->Style = TFontStyles() << fsBold;
                if (wi->Type == 3) {    // string
                        if (strncmp(wi->s_data, wi->data, wi->Length) != 0) can->Font->Style = TFontStyles() << fsBold;
                } else if (wi->Type == 4) {     // unicode
                        if (ustrncmp(wi->s_data, wi->data, wi->Length/2) != 0) can->Font->Style = TFontStyles() << fsBold;
                } else {
                        if (memcmp(wi->s_data, wi->data, wi->Length) != 0) can->Font->Style = TFontStyles() << fsBold;
                }

                n = *((int *)(wi->data));
                bdata = wi->data;
                hdata = (unsigned short *)(wi->data);
                idata = (unsigned int *)(wi->data);
                l = wi->Length;
                p = buf;
                switch (wi->Type) {
                        case WTYPE_BYTE:
                                if (l == 1) {
                                        sprintf(buf, "%02X (%i)", n & 0xff, n & 0xff);
                                } else {
                                        for (i=0; i<l; i++) {
                                                sprintf(p, "%s%02X", i==0?"":", ", bdata[i]);
                                                p += strlen(p);
                                        }
                                }
                                break;
                        case WTYPE_HALFWORD:
                                if (l == 2) {
                                        sprintf(buf, "%04X (%i)", n & 0xffff, n & 0xffff);
                                } else {
                                        for (i=0; i<l/2; i++) {
                                                sprintf(p, "%s%04X ", i==0?"":", ", hdata[i]);
                                                p += strlen(p);
                                        }
                                }
                                break;
                        case WTYPE_WORD:
                                if (l == 4) {
                                        sprintf(buf, "%08X (%i)", n, n);
                                } else {
                                        for (i=0; i<l/4; i++) {
                                                sprintf(p, "%s%08X ", i==0?"":", ", idata[i]);
                                                p += strlen(p);
                                        }
                                }
                                break;
                        case WTYPE_STRING:
                                buf[0] = '\"';
                                for (i=0; i<64; i++) {
                                        if (wi->data[i] == '\0') break;
                                        buf[i+1] = (wi->data[i] < 32) ? '?' : wi->data[i];
                                }
                                buf[i+1] = '\"';
                                buf[i+2] = 0;
                                break;
                        case WTYPE_UNICODE:
                                buf[0] = '\"';
                                hp = (unsigned short *)(wi->data);
                                for (i=0; i<64; i++) {
                                        if (hp[i] == 0) break;
                                        if (hp[i] >= 32 && hp[i] < 256) {
                                                buf[i+1] = (unsigned char)hp[i];
                                        } else {
                                                buf[i+1] = '&';
                                        }
                                }
                                buf[i+1] = '\"';
                                buf[i+2] = 0;
                                break;
                }
                can->TextRect(rect, rect.Left+2, rect.Top-1, buf);
        }

}


void cHashAdd(unsigned int addr, char *text) {

        int i;

        for (i=0; i<64; i++) {
                if (CP.chash_a[i] == addr) return;
        }

        CP.chash_a[CP.chash_pos] = addr;
        strcpy(CP.chash_t[CP.chash_pos], text);
        CP.chash_pos++;
        if (CP.chash_pos >= 64) CP.chash_pos = 0;

}

void cHashGet(unsigned int addr, char *text) {

        int i;

        for (i=0; i<64; i++) {
                if (CP.chash_a[i] == addr) {
                        strcpy(text, CP.chash_t[i]);
                        break;
                }
        }

}

unsigned int cpGetAddress() {

        int y = CP.page->Row;
        return CP.BaseAddress + y * CP.InsWidth;

}


void cpu_reload() {

        unsigned int addr, data;
        unsigned int s_addr;
        unsigned int read_addr, read_top;
        unsigned int copy_from, copy_to;
        int lines, s_lines, read_lines, copy_lines;
        instruction *ins;
        unsigned int r[18];
        unsigned int q_addr[256], q_data[256], q_shift[256], q_pos[256];
        int i, n, xadr, cpos;

        if (! is_online()) return;
        if (CP.Freeze) return;
        if (MainForm->Pages->ActivePage != MainForm->SheetCPU) return;

        lines = CP.DataLines = CP.page->VisibleRowCount + 1;
        if (CP.page->TopRow + CP.DataLines > CP.page->RowCount)
                CP.DataLines--;
        addr = CP.BaseAddress + CP.page->TopRow * CP.InsWidth;

        readMemory(addr, CP.Data, lines * CP.InsWidth);

        n = 0;
        xadr = 0;
        for (i=0; i<lines-1; i++) {
                data = *((unsigned int *)(CP.Data + i*CP.InsWidth));
                if (CP.Thumb) {
                        ins = disthumb(addr + i*2, data, r);
                } else {
                        ins = disarm(addr + i*4, data, r);
                }

                CP.adrtgt[i] = 0;
                CP.adrdata[i][0] = 0;

                if (ins->is_adr && ins->target_address >= FlashAddress && ins->target_address <= FlashAddress+FlashSize-1) {
                        CP.adrtgt[i] = ins->target_address;
                        xadr = 1;
                }
                if (ins->needs_target_data && inSpace(ins->target_address)) {
                        q_addr[n] = ins->target_address & ~0x03;
                        q_shift[n] = (ins->target_address & 0x03) * 8;
                        q_pos[n] = i;
                        n++;
                } else {
                        CP.target[i] = NONE;
                }
        }
        if (n > 0) {
                queryRAM(q_addr, q_data, n);
                for (i=0; i<n; i++) {
                        cpos = q_pos[i];
                        CP.target[cpos] = q_data[i] >> q_shift[i];
                        if (q_data[i] >= FlashAddress && q_data[i] <= FlashAddress+FlashSize-1) {
                                CP.adrtgt[cpos] = q_data[i];
                                xadr = 1;
                        }
                }
        }

        for (int i=0; i<lines-1; i++) {
                cHashGet(CP.adrtgt[i], CP.adrdata[i]);
        }

        MainForm->AdrTimer->Enabled = false;
        if (xadr != 0) {
                MainForm->AdrTimer->Interval = 275;
                MainForm->AdrTimer->Enabled = true;
        }

        CP.page->Repaint();
}


void drawCPcell(int acol, int arow, TRect &rect, TGridDrawState state) {

        static int SCOLORS[7] = { 0, 0x800000, 0x008000, 0x0028a8, 0xff0000, 0xff00ff, 0x0000ff };

        int W1 = 15;
        int W2 = DCharWidth*14;
        int W3 = DCharWidth*17;

        TCanvas *can;
        TColor asmcolor, adrcolor, bgcolor, inscolor;
        bool onecolor;
        unsigned int addr, offset, offset_4, n, v, pos;
        unsigned char *data;
        unsigned char c;
        int toprow;
        bool sel;
        char buf[256], *p, *pp;
        instruction *ins;
        unsigned int r[18];
        int i, l, al, issnap;
        snap **sn;

        data = CP.Data;
        can = CP.page->Canvas;
        toprow = CP.page->TopRow;
        bgcolor = (TColor)0xffffff;
        can->Font = DebuggerFont;

        addr = CP.BaseAddress + arow * CP.InsWidth;

        adrcolor = (TColor)0;
        onecolor = isChanged4(addr);
        asmcolor = (TColor)(onecolor ? 0x0000f8 : SCOLORS[1]);
        inscolor = (TColor)0xb8b8b8;

        if (CP.SelStart <= CP.SelEnd) {
                sel = (addr >= CP.SelStart && addr <= CP.SelEnd);
        } else {
                sel = (addr >= CP.SelEnd && addr <= CP.SelStart);
        }
        if (sel) bgcolor = SEL_COLOR;

        addr = CP.BaseAddress + arow * CP.InsWidth;
        offset = (arow - toprow) * CP.InsWidth;
        n = *((unsigned int *)(data+offset));

        issnap = 0;
        if ((CP.Thumb && ((n & 0xfff0) == 0xbe00) || ((n >> 16) & 0xfff0) == 0xbe00) || (!CP.Thumb && (n & 0xfffffff0) == 0xe1200070)) {
                sn = getSnapList();
                for (i=1; i<16; i++) {
                        if (sn[i] != NULL && sn[i]->addr == addr) {
                                n = sn[i]->instruction;
                                issnap = sn[i]->isbreak ? 2 : 1;
                                break;
                        }
                }
        }

        if ((SystemState == SS_INTBRK || SystemState == SS_EXTBRK) && addr == CurrentAddr) {
                adrcolor = asmcolor = inscolor = clWhite;
                onecolor = true;
                bgcolor = (issnap == 2) ? (TColor)0x0000c0 : (TColor)0xc00000;
        }

        can->Brush->Color = bgcolor;
        can->FillRect(rect);

        // address / name

                p = nameByAddress(addr);
                if (p == NULL) {
                        can->Font->Color = adrcolor;
                        sprintf(buf, " %08X:", addr);
                } else {
                        can->Font->Color = SCOLORS[4];
                        strcpy(buf, p);
                        strcat(buf, ":");
                        can->Font->Style = TFontStyles() << fsBold;
                }
                al = can->TextWidth(buf);
                can->TextOut(rect.Left+1+W1, rect.Top, buf);
                can->Font->Style = TFontStyles();

        if (! is_online() || ! is_connected() || arow - toprow >= CP.DataLines) {
                return;
        }

        // breakpoint / current position

        if ((SystemState == SS_INTBRK || SystemState == SS_EXTBRK) && addr == CurrentAddr) {
                MainForm->Icons->Draw(can, rect.Left+1, rect.Top, 0, true);
        } else {
                if (issnap == 1) {
                        MainForm->Icons->Draw(can, rect.Left+1, rect.Top, 2, true);
                } else if (issnap == 2) {
                        MainForm->Icons->Draw(can, rect.Left+1, rect.Top, 1, true);
                }
        }

        // assembler mnemonic

        can->Font->Color = asmcolor;
        ins = CP.Thumb ? disthumb(addr, n, r) : disarm(addr, n, r);
        i = arow - toprow;
        if (ins->needs_target_data && inSpace(ins->target_address) && CP.target[i] != NONE) {
                ins = CP.Thumb ? disthumb2(addr, n, r, CP.target[i]) : disarm2(addr, n, r, CP.target[i]);
        }
        for (p=ins->colorized; *p!=0 && *p!='\t'; p++) ;
        l = p-ins->colorized;
        strncpy(buf, ins->colorized, l);
        buf[l] = buf[l+1] = '\0';
        if ((buf[0] & 0xf8) == 0) {
                if (! onecolor) can->Font->Color = SCOLORS[buf[0]];
                buf[0] = ' ';
        }
        can->TextOut(rect.Left+1+W1+W2+W3, rect.Top, buf);
        if (*p == '\t') p++;
        pos = rect.Left+1+W1+W2+W3+60;
        while ((c = *p) != 0) {
                if ((c & 0xf8) == 0) {
                        if ((*(p+1) & 0xf8) != 0 && ! onecolor) can->Font->Color = SCOLORS[c];
                        p++;
                        continue;
                }
                for (pp=p+1; (*pp & 0xf8) != 0; pp++) ;
                memcpy(buf, p, pp-p);
                buf[pp-p] = '\0';
                can->TextOut(pos, rect.Top, buf);
                pos += can->TextWidth(buf);
                p = pp;
        }
        if (CP.adrtgt[i] != 0 && CP.adrdata[i][0] != 0) {
                can->Font->Color = (TColor)0xa0a0a0;
                can->TextOut(rect.Left+1+W1+W2+W3+175, rect.Top, CP.adrdata[i]);
        }

        // instruction

        can->Font->Color = inscolor;
        int v1 = n & 0xff;
        int v2 = (n >> 8) & 0xff;
        int v3 = (n >> 16) & 0xff;
        int v4 = (n >> 24) & 0xff;
        if (CP.Thumb) {
                sprintf(buf, "%02X %02X", v1, v2);
        } else {
                sprintf(buf, "%02X %02X %02X %02X", v1, v2, v3, v4);
        }
        if (al < W2-2) can->TextOut(rect.Left+1+W1+W2, rect.Top, buf);

        // line

        if (ins->is_endflow) {
                can->Pen->Color = (TColor)0xf0b090;
                can->Pen->Style = psDot;
                can->Pen->Width = 1;
                can->MoveTo(rect.Left, rect.Bottom-1);
                can->LineTo(rect.Right, rect.Bottom-1);
        }

        // rect

        if (state.Contains(gdFocused)) {
                rect.left--;
                rect.right++;
                can->Brush->Color = RECT_COLOR;
                can->FrameRect(rect);
        }

}

void cpSetAddress(unsigned int addr, bool save) {

        unsigned int top, bottom, saddr;
        TDrawGrid *page = CP.page;
        memoryinfo mi, cmi;
        bool setthumb;

        getMemoryInfo(addr, &mi);
        if (mi.len == 0) return;

        saddr = cpGetAddress();
        if (save && (CP.a_pos == 0 || ((saddr ^ CP.a_history[CP.a_pos-1]) & ~3) != 0)) {
                if (CP.a_pos >= 254) {
                        memmove(&(CP.a_history[0]), &(CP.a_history[1]), 254 * sizeof(unsigned int));
                        CP.a_pos--;
                }
                CP.a_history[CP.a_pos++] = CP.Thumb ? saddr+1 : saddr;
        }

        setthumb = ((addr & 1) != 0);
        addr &= ~(setthumb ? 1 : 3);

        getMemoryInfo(saddr, &cmi);
        top = cmi.addr + page->TopRow * CP.InsWidth;
        bottom = top + page->VisibleRowCount * CP.InsWidth;
        if (addr >= top && addr < bottom-5 && setthumb == CP.Thumb) {
                page->Row = (addr - mi.addr) / CP.InsWidth;
                MainForm->vCPU->Repaint();
        } else {
                CP.Freeze = true;
                CP.Thumb = setthumb;
                CP.InsWidth = setthumb ? 2 : 4;
                top = addr - 4 * CP.InsWidth;
                if (top < mi.addr) top = mi.addr;
                CP.BaseAddress = mi.addr;
                CP.Size = mi.len;
                CP.Type = mi.type;
                page->RowCount = mi.len / CP.InsWidth;
                page->TopRow = (top - mi.addr) / CP.InsWidth;
                page->Row = (addr - mi.addr) / CP.InsWidth;
                CP.Freeze = false;
                cpu_reload();
        }
        MainForm->Pages->ActivePage = MainForm->SheetCPU;
        return;


}

unsigned int cpGetOffset() {

        return (CP.page->Row - CP.page->TopRow) * CP.InsWidth;

}

void cpGoto() {

        unsigned int addr;

        if (GotoForm->Invoke("Goto address", NULL, &addr)) {
                cpSetAddress(CP.Thumb ? addr|1 : addr, true);
        }
}

int regnum(int col, int row) {

        int r;

        if (col == 0 && row <= 15) return row;
        if (col == 1) {
                r = RRC + row;
                if (r <= 15) return r;
        }
        return -1;

}

int stackpos(int col, int row) {

        int r;

        if (regnum(col, row) != -1) return -1;
        if (col == 0) return -1;
        if (col == 1) {
                r = RRC - row - 1;
                if (r >= 0 && r <= 15) return r;
        }
        return -1;

}

void setRegisters(unsigned int r[16], unsigned int cpsr, int count, unsigned int stack[16]) {

        int i;

        for (i=0; i<16; i++) {
                REG_S[i] = REG[i];
                REG[i] = r[i];
        }
        CPSR_S = CPSR;
        CPSR = cpsr;
        REG_COUNT = count;
        if (stack != NULL) {
                for (i=0; i<16; i++) {
                        STACK[i] = stack[i];
                        STACK_FILLED = true;
                }
        } else {
                STACK_FILLED = false;
        }
        MainForm->Registers->Repaint();
}

unsigned int getRegister(int num) {

        return (num < REG_COUNT) ? REG[num] : NONE;

}

unsigned int getStackEntry(int num) {

        return STACK[num];

}

void editRegister(int col, int row) {

        unsigned int v;
        char tbuf[16], tbuf2[16];
        int num, pos;

        num = regnum(col, row);
        if (num >= 0) {
                // register
                sprintf(tbuf, "%08X", REG[num]);
                if (! GotoForm->Invoke(regnames[num], tbuf, &v)) return;
                updateRegister(num, v);
        } else if (col == 1 && row <= 7) {
                // stack
                pos = 7 - row;
                sprintf(tbuf, "%08X", STACK[pos]);
                sprintf(tbuf2, "[SP+%02X]", pos);
                if (! GotoForm->Invoke(tbuf2, tbuf, &v)) return;
                if (! setMemory(REG[13]+pos*4, (unsigned char *)&v, 4, SM_INTERNAL)) return;
                STACK[pos] = v;
                MainForm->Registers->Repaint();
                reload_dumps();
        } else if (col == 2 && row <= 3) {
                // cpsr
                v = CPSR ^ (0x80000000 >> row);
                updateCPSR(v);
        }
}

void drawRegCell(int acol, int arow, TRect &rect, TGridDrawState state) {

        char buf[16];
        TCanvas *can;
        TPoint pt;
        int rn, fv, fvs, pos;
        TRect rect2;

        RRC = MainForm->Registers->VisibleRowCount;

        can = MainForm->Registers->Canvas;

        can->Brush->Color = state.Contains(gdFocused) ? (TColor)0xe8e8e8 : (TColor)0xffffff;
        can->FillRect(rect);


        if (acol < 2) {
                pt.x = rect.Right-1;
                pt.y = rect.Top;
                can->Pen->Color = (TColor)0x808080;
                can->Pen->Style = psSolid;
                can->PenPos = pt;
                can->LineTo(pt.x, rect.Bottom);
        }
        rect.Right -= 1;
        rect2 = rect;

        can->Font = DebuggerFont;
        can->Font->Color = (TColor)0;
        rn = regnum(acol, arow);
        if (rn >= 0) {
                can->Font->Style = (REG[rn] == REG_S[rn] || REG_S[rn] == REG_EMPTY) ? TFontStyles() : TFontStyles() << fsBold;
                can->Font->Color = (TColor)0x60a060;
                can->TextRect(rect, rect.Left+2, rect.Top-1, regnames[rn]);
                if (rn < REG_COUNT) {
                        sprintf(buf, "%08X", REG[rn]);
                        can->Font->Color = (TColor)0;
                        rect2.Left += DCharWidth*4;
                        can->TextRect(rect2, rect2.Left, rect2.Top-1, buf);
                }
        } else if (acol == 1) {
                pos = stackpos(acol, arow);
                if (pos != -1) {
                        can->Brush->Color = state.Contains(gdFocused) ? (TColor)0xe8e8e8 : (TColor)0xffffe0;
                        can->FillRect(rect);
                        can->Font->Style = TFontStyles();
                        can->Font->Color = (TColor)0x60a060;
                        sprintf(buf, "+%02X", pos*4);
                        can->TextRect(rect, rect.Left+2, rect.Top-1, buf);
                        if (STACK_FILLED) {
                                sprintf(buf, "%08X", STACK[pos]);
                                can->Font->Style = TFontStyles();
                                can->Font->Color = (TColor)0;
                                rect2.Left += DCharWidth*4;
                                can->TextRect(rect2, rect2.Left, rect2.Top-1, buf);
                        }
                }
        } else if (acol == 2 && arow <= 3) {
                fv = (CPSR >> (31 - arow)) & 1;
                fvs = (CPSR_S >> (31 - arow)) & 1;
                can->Font->Style = (fv == fvs || fvs == REG_EMPTY) ? TFontStyles() : TFontStyles() << fsBold;
                can->Font->Color = (TColor)0x60a060;
                buf[0] = "NZCVQ"[arow];
                buf[1] = '\0';
                can->TextRect(rect, rect.Left+2, rect.Top-1, buf);
                if (REG_COUNT != 0) {
                        buf[0] = fv ? '1' : '0';
                        buf[1] = '\0';
                        can->Font->Color = (TColor)0;
                        rect2.Left += DCharWidth*2;
                        can->TextRect(rect2, rect2.Left, rect2.Top-1, buf);
                }
        } else {
                return;
        }

        if (state.Contains(gdFocused)) {
                can->Brush->Color = RECT_COLOR;
                can->FrameRect(rect);
        }

/*
        if (acol == 1 && arow == 7) {
                pt.x = rect.Left;
                pt.y = rect.Bottom-1;
                can->Pen->Color = 0x808080;
                can->Pen->Style = psSolid;
                can->PenPos = pt;
                can->LineTo(rect.Right, pt.y);
                rect.Right -= 1;
        }
*/
        return;
}

void copyData(unsigned int xStart, unsigned int xEnd) {

        unsigned int s, e, n;
        int len;
        unsigned char *buf;
        char *p, *pp;

        if (xStart <= xEnd) {
                s = xStart;
                e = xEnd;
        } else {
                s = xEnd;
                e = xStart;
        }
        len = (e - s) + 1;

        if (len > 65536) {
                WarningMessage("Block is too large");
                return;
        }

        n = len * 3 + (len / 16) * 2 + 16;
        p = pp = (char *)malloc(n);

        buf = (unsigned char *)malloc(len);
        if (readMemory(s, buf, len)) {
                for (int i=0; i<len; i++) {
                        *(pp++) = digits[buf[i] >> 4];
                        *(pp++) = digits[buf[i] & 0xf];
                        if ((i & 0x0f) == 0x0f || i+1 == len) {
                                *(pp++) = '\r';
                                *(pp++) = '\n';
                        } else {
                                *(pp++) = ' ';
                        }
                }
                *(pp++) = '\0';
                setClipboard(p);
        }
        free(buf);
        free(p);
}

void pasteData(unsigned int addr) {

        HGLOBAL H;
        unsigned int n;
        unsigned char *buf, *d;
        unsigned char buf1[4];
        unsigned char c1, c2;
        char *p;

        if (addr == NONE) return;

        if (! OpenClipboard(NULL)) return;

        H = GetClipboardData(CF_TEXT);
        if (H == NULL) {
                CloseClipboard();
                return;
        }

        p = (char *)GlobalLock(H);
        if (p == NULL) {
                CloseClipboard();
                return;
        }

        buf = (unsigned char *)malloc(65536);
        d = buf;
        n = 0;

        while (*p) {
                if (isxdigit(*p) && isxdigit(*(p+1))) {
                        c1 = *(p++); if (c1 >= 0x60) c1 -= 0x20; c1 -= 0x30; if (c1 > 9) c1 -= 7;
                        c2 = *(p++); if (c2 >= 0x60) c2 -= 0x20; c2 -= 0x30; if (c2 > 9) c2 -= 7;
                        *(d++) = (c1 << 4) | c2;
                        if (++n == 65536) break;
                } else {
                        p++;
                }
        }

        GlobalUnlock(H);
        CloseClipboard();

        if (n >= 65536) {
                WarningMessage("Block is too large");
                free(buf);
                return;
        }

        setMemory(addr, buf, n, SM_STICKY);
        free(buf);

}

void copyRPCdata() {

        char tbuf[16];

        if (! is_online()) return;
        if (RPC->SelStart == NONE) {
                sprintf(tbuf, "%08X", RPC->SelRow);
                setClipboard(tbuf);
        } else {
                copyData(RPC->SelStart, RPC->SelEnd);
        }

}

void pasteRPCdata() {

        if (! is_online()) return;
        pasteData(RPC->SelStart);
        reload_dumps();

}

void copyAsmText() {

        unsigned int s, e, addr, data, r[16], atgt, tgt;
        int len, pos, size, width, n;
        unsigned char *buf;
        char *p, *name;
        char tbuf[256];
        instruction *ins;

        if (! is_online()) return;
        if (CP.SelStart == NONE) return;

        if (CP.SelStart <= CP.SelEnd) {
                s = CP.SelStart;
                e = CP.SelEnd;
        } else {
                s = CP.SelEnd;
                e = CP.SelStart;
        }
        len = (e - s) + 1;

        if (len > 65536) {
                WarningMessage("Block is too large");
                return;
        }

        size = 16384;
        p = (char *)malloc(size);
        pos = 0;

        buf = (unsigned char *)malloc(len);
        if (readMemory(s, buf, len)) {
                width = CP.InsWidth;
                for (int i=0; i<len; i+=width) {
                        addr = s+i;
                        data = *((unsigned int *)(buf+i));
                        ins = CP.Thumb ? disthumb(addr, data, r) : disarm(addr, data, r);
                        if (ins->needs_target_data && inSpace(ins->target_address)) {
                                atgt = ins->target_address;
                                if (atgt >= s && atgt <= e-4) {
                                        tgt = *((unsigned int *)(buf+(atgt-s)));
                                        ins = CP.Thumb ? disthumb2(addr, data, r, tgt) : disarm2(addr, data, r, tgt);
                                } else {
                                        if (readMemory(atgt, (unsigned char *)&tgt, 4)) {
                                                ins = CP.Thumb ? disthumb2(addr, data, r, tgt) : disarm2(addr, data, r, tgt);
                                        }
                                }
                        }
                        name = nameByAddress(addr);
                        if (name == NULL) {
                                sprintf(tbuf, "%08X", addr);
                        } else {
                                sprintf(tbuf, "%s:", name);
                        }

                        if (pos+82 > size) {
                                size = size + size/2;
                                p = (char *)realloc(p, size);
                        }
                        if (! CP.Thumb) {
                                n = sprintf(p+pos, "%-31s %02X %02X %02X %02X  %s\r\n",
                                        tbuf,
                                        data & 0xff,
                                        (data >> 8) & 0xff,
                                        (data >> 16) & 0xff,
                                        (data >> 24) & 0xff,
                                        ins->resolved
                                );
                        } else {
                                n = sprintf(p+pos, "%-31s %02X %02X            %s\r\n",
                                        tbuf,
                                        data & 0xff,
                                        (data >> 8) & 0xff,
                                        ins->resolved
                                );
                        }
                        pos += n;
                }
        }

        setClipboard(p);
        free(buf);
        free(p);

}

void copyAsmBinary() {

        if (! is_online()) return;
        if (CP.SelStart == NONE) return;
        copyData(CP.SelStart, CP.SelEnd);

}

void pasteAsmBinary() {

        if (! is_online()) return;
        if (CP.SelStart == NONE) return;
        pasteData(CP.SelStart);
        reload_dumps();

}

