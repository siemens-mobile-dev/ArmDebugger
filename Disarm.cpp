#include "Disarm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Debug.h"

#define C1 "\001"
#define C2 "\002"
#define C3 "\003"
#define C4 "\004"
#define C5 "\005"
#define C6 "\006"

static char *REGISTERS[] = {
        "R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7",
        "R8", "R9", "R10", "R11", "R12", "SP", "LR", "PC"
};

static char *CREGISTERS[] = {
        C2"R0"C1, C2"R1"C1, C2"R2"C1, C2"R3"C1, C2"R4"C1, C2"R5"C1, C2"R6"C1, C2"R7"C1,
        C2"R8"C1, C2"R9"C1, C2"R10"C1, C2"R11"C1, C2"R12"C1, C2"SP"C1, C2"LR"C1, C2"PC"C1
};

static char *CONDITIONS[] = {
        "EQ", "NE", "CS", "CC", "MI", "PL", "VS", "VC",
        "HI", "LS", "GE", "LT", "GT", "LE", "", ""
};

static char *DATAPROC[] = {
        "AND", "EOR", "SUB", "RSB", "ADD", "ADC", "SBC", "RSC",
        "TST", "TEQ", "CMP", "CMN", "ORR", "MOV", "BIC", "MVN"
};

static char *TDATAPROC[] = {
        "AND", "EOR", "LSL", "LSR", "ASR", "ADC", "SBC", "ROR",
        "TST", "NEG", "CMP", "CMN", "ORR", "MUL", "BIC", "MVN"
};

static char *TSPROC[] = {
        "ADD", "CMP", "MOV"
};

static char *LSROFF[] = {
        "STR", "STRH", "STRB", "LDRSB", "LDR", "LDRH", "LDRB", "LDRSH"
};

static char *ASCM[] = {
        "MOV", "CMP", "ADD", "SUB"
};

static char DATAOPNS[] = "3333333322223232";
static char SBITUSED[] = "1111111100001111";
static char USE_RD[]   = "0000000000000101";

static char *DSPAS[] = {
        "QADD", "QSUB", "QDADD", "QDSUB"
};

static char *SHIFTS[] = {
        "LSL", "LSR", "ASR", "ROR"
};

static char *MULTIPLES1[] = {
        "DA", "IA", "DB", "IB"
};

static char *MULTIPLES2[] = {
        "ED", "EA", "FD", "FA", "FA", "FD", "EA", "ED"
};


char *hex(unsigned int n) {

        static char buf[32];
        unsigned int x;

        if (n <= 9)
                sprintf(buf, "%i", n);
        else
                sprintf(buf, "0x%X", n);

        return buf;

}

unsigned int get_rimm(unsigned int n) {

        unsigned int imm, r;

        imm = n & 0xff;
        r = ((n >> 8) & 0xf) << 1;
        imm = (imm >> r) | (imm << (32-r));

        return imm;

}

char *shift_Rm(unsigned int n) {

        static char buf[32];
        int shift_amt, shift, Rm, Rs, b4;

        shift_amt = (n >> 7) & 0x1f;
        shift = (n >> 5) & 3;
        Rm = n & 0xf;
        Rs = (n >> 8) & 0xf;
        b4 = (n >> 4) & 1;

        if (b4 == 0) {
                if (shift_amt == 0 && shift == 0) {
                        strcpy(buf, CREGISTERS[Rm]);
                } else {
                        sprintf(buf, "%s,%s#%i", CREGISTERS[Rm], SHIFTS[shift], shift_amt);
                }
        } else {
                sprintf(buf, "%s,%s %s", CREGISTERS[Rm], SHIFTS[shift], CREGISTERS[Rs]);
        }

        return buf;

}

char *rlist(unsigned int reglist) {

        static char buf[64];
        int i, rstart, rend;

        rstart = -1;
        buf[0] = '\0';
        for (i = 0; i < 17; i++) {
                if (reglist & (1 << i)) {
                        if (rstart == -1) rstart = i;
                } else {
                        if (rstart != -1) {
                                rend = i-1;
                                if (buf[0] != '\0') strcat(buf, ",");
                                strcat(buf, CREGISTERS[rstart]);
                                if (rstart != rend) {
                                        strcat(buf, (rstart+1 == rend) ? "," : "-");
                                        strcat(buf, CREGISTERS[rend]);
                                }
                                rstart = -1;
                        }
                }
        }
        return buf;
}

char *localName(unsigned int addr) {

        char buf[64], *r;

        r = nameByAddress(addr);
        if (r == NULL) {
                sprintf(buf, "l_%08X", addr);
                addName(buf, addr, false);
                r = nameByAddress(addr);
        }
        return r;

}

void uncolorize(char *s) {

        char *p = s;
        char c;

        while (*s) {
                if ((c = *(s++)) & 0xf8) *(p++) = c;
        }
        *p = '\0';

}

void postprocess(instruction *si) {

        if (si->canonical[0] == '\0') strcpy(si->canonical, C6 "???");
        if (si->resolved[0] == '\0') strcpy(si->resolved, si->canonical);
        strcpy(si->colorized, si->resolved);
        uncolorize(si->canonical);
        uncolorize(si->resolved);

}

#pragma warn -aus
instruction * dis_arm_v(unsigned int addr, unsigned int ins, bool target_data_given, unsigned int target_data) {

        static instruction si;

        unsigned int cond, cls, opcode, opcode2, misc, imm8, imm12, imm24, imm44, reglist;
        unsigned int rotate, Mask, CRn, CRd, CRm, cp_num, shift_amt, shift, dsp_op, target, size;
        unsigned int Rn, Rd, Rm, Rs, S, P, U, B, W, L, R, X, Y;
        unsigned int b4, b5, b7, b20, b21, b22, b23, b24, b27;
        int idx, i;
        char *a_cond, *a_S, *a_Rd, *a_Rm, *a_Rn, *a_Rs, *mn, c, *s, *tname;
        char buf[64];


        memset(&si, 0, sizeof(si));

        cond = (ins >> 28) & 0x0f;
        a_cond = CONDITIONS[cond];

        S = (ins >> 20) & 1;
        a_S = (S == 0) ? "" : "S";

        cls = (ins >> 25) & 0x07;
        opcode = (ins >> 21) & 0x0f;
        opcode2 = (ins >> 5) & 0x07;
        misc = (ins >> 4) & 0x0f;
        imm8 = ins & 0xff;
        imm12 = ins & 0xfff;
        imm24 = ins & 0xffffff;
        imm44 = ((ins & 0xf00) >> 4) | (ins & 0xf);
        reglist = ins & 0xffff;

        Rn = (ins >> 16) & 0x0f;
        Rd = (ins >> 12) & 0x0f;
        Rm = ins & 0x0f;
        Rs = (ins >> 8) & 0x0f;
        a_Rn = CREGISTERS[Rn];
        a_Rd = CREGISTERS[Rd];
        a_Rm = CREGISTERS[Rm];
        a_Rs = CREGISTERS[Rs];

        P = (ins >> 24) & 1;
        U = (ins >> 23) & 1;
        R = B = (ins >> 22) & 1;
        W = (ins >> 21) & 1;
        L = (ins >> 20) & 1;
        X = (ins >> 5) & 1;
        Y = (ins >> 6) & 1;

        rotate = Rs;
        Mask = Rn;
        CRn = Rn;
        CRd = Rd;
        CRm = Rm;
        cp_num = Rs;
        shift_amt = (ins >> 7) & 0x1f;
        shift = (ins >> 5) & 0x03;
        dsp_op = (ins >> 21) & 0x03;

        b4 = (ins >> 4) & 1;
        b5 = (ins >> 5) & 1;
        b7 = (ins >> 7) & 1;
        b20 = (ins >> 20) & 1;
        b21 = (ins >> 21) & 1;
        b22 = (ins >> 22) & 1;
        b23 = (ins >> 23) & 1;
        b24 = (ins >> 24) & 1;
        b27 = (ins >> 27) & 1;

        si.data = ins;
        si.length = 4;
        si.is_conditional = (cond != 0xe && cond != 0xf);

        if (cond == 0xf && b27 == 0) {

                // cache preload

        } else if (cls == 0 && b4 == 1 && b7 == 1) {

                // multiplies, extra load/store 5

                if (b24 == 0 && misc == 9 && b23 == 0 && b21 == 0 && Rd == 0) {

                        // mul
                        sprintf(si.canonical, "MUL%s%s\t%s, %s, %s",
                                a_cond,
                                a_S,
                                a_Rn,
                                a_Rm,
                                a_Rs
                        );

                } else if (b24 == 0 && misc == 9 && b23 == 0 && b21 == 1) {

                        // mla
                        sprintf(si.canonical, "MLA%s%s\t%s, %s, %s, %s",
                                a_cond,
                                a_S,
                                a_Rn,
                                a_Rm,
                                a_Rs,
                                a_Rd
                        );

                } else if (b24 == 0 && misc == 9 && b23 == 1) {

                        // [us]mull/mlal
                        sprintf(si.canonical, "%s%s%s%s\t%s, %s, %s, %s",
                                (b22 == 0) ? "U" : "S",
                                (b21 == 0) ? "MULL" : "MLAL",
                                a_cond,
                                a_S,
                                a_Rd,
                                a_Rn,
                                a_Rm,
                                a_Rs
                        );

                } else if (b24 == 1 && b23 == 0 && b21 == 0 && b20 == 0 && misc == 9 && Rs == 0) {

                        // swap
                        sprintf(si.canonical, "SWP%s%s\t%s, %s, [%s]",
                                a_cond,
                                (b22 == 0) ? "" : "B",
                                a_Rd,
                                a_Rm,
                                a_Rn
                        );

                        if (Rd == 15) {
                                si.is_branch = true;
                                if (cond == 0xe) si.is_endflow = true;
                        }

                } else if (misc == 0xb || misc == 0xd || misc == 0xf) {

                        // load/store halfword/byte/dword
                        if (misc == 0xb) c = 'H';
                        else if (b20 == 0) c = 'D';
                        else c = (b5 == 0) ? 'B' : 'H';

                        sprintf(buf, ",%s%s%s",
                                (b22 == 1) ? C3 "#" : "",
                                (U == 1) ? "" : "-",
                                (b22 == 1) ? hex(imm44) : CREGISTERS[Rm]
                        );
                        if (b22 == 1 && imm44 == 0) buf[0] = '\0';

                        mn = ((misc == 0xb && b20 == 0) || (misc == 0xf && b20 == 0)) ? "STR" : "LDR",
                        a_S = (b20 == 1 && (misc == 0xd || misc == 0xf)) ? "S" : "";

                        if (P == 1) {
                                // pre-indexed or offset ldr[b]/str[b]
                                sprintf(si.canonical, "%s%s%s%c\t%s, [%s%s]%s",
                                        mn,
                                        a_cond,
                                        a_S,
                                        c,
                                        a_Rd,
                                        a_Rn,
                                        buf,
                                        (W == 1) ? "!" : ""
                                );
                        } else {
                                // post-indexed ldr[b][t]/str[b][t]
                                sprintf(si.canonical, "%s%s%s%c\t%s, [%s]%s",
                                        mn,
                                        a_cond,
                                        a_S,
                                        c,
                                        a_Rd,
                                        a_Rn,
                                        buf
                                );
                        }

                        // ldr(h/b) =0x...

                }

        } else if (cls == 0 && (opcode & 0xc) == 0x8 && S == 0) {

                // miscellaneous instructions 2,4

                if (misc == 0 && b21 == 0 && Rn == 0xf && Rs == 0 && Rm == 0) {

                        // mrs M1

                        sprintf(si.canonical, "MRS%s\t%s, %sPSR",
                                a_cond,
                                a_Rd,
                                (b22 == 0) ? C5 "C" : C5 "S"
                        );

                } else if (misc == 0 && b21 == 1 && Rd == 0xf && Rs == 0) {

                        // msr M2

                        sprintf(si.canonical, "MSR%s\t%sPSR_%s%s%s%s%s, %s",
                                a_cond,
                                (b22 == 0) ? C5 "C" : C5 "S",
                                ((ins >> 16) & 1) ? "c" : "",
                                ((ins >> 17) & 1) ? "x" : "",
                                ((ins >> 18) & 1) ? "s" : "",
                                ((ins >> 19) & 1) ? "f" : "",
                                C1,
                                a_Rm
                        );

                } else if ((misc == 1 || misc == 3) && b22 == 0 && b21 == 1 && Rn == 0xf && Rd == 0xf && Rs == 0xf) {

                        // bx, blx reg M3,M5

                        sprintf(si.canonical, "%s%s\t%s",
                                (misc == 1) ? "BX" : "BLX",
                                a_cond,
                                a_Rm
                        );
                        si.is_branch = true;
                        if (cond == 0xe && misc == 1) si.is_endflow = true;
                        si.is_exchange = true;

                } else if (misc == 1 && b22 == 1 && b21 == 1 && Rn == 0xf && Rs == 0xf) {

                        // clz M4

                        sprintf(si.canonical, "CLZ%s\t%s, %s",
                                a_cond,
                                a_Rd,
                                a_Rm
                        );

                } else if (misc == 5 && Rs == 0) {

                        // enhanced DSP add/substracts M6

                        sprintf(si.canonical, "%s%s\t%s, %s, %s",
                                DSPAS[dsp_op],
                                a_cond,
                                a_Rd,
                                a_Rm,
                                a_Rs
                        );

                } else if (misc == 7 && b22 == 0 && b21 == 1) {

                        // bkpt M7

                        sprintf(si.canonical, "BKPT\t"C5"%s",
                                hex(((ins & 0x000fff00) >> 4) | (ins & 0xf))
                        );

                } else if (misc >= 8) {

                        // enhanced DSP multiplies M8

                        if (dsp_op == 0) {
                                sprintf (si.canonical, "SMLA%c%c%s\t%s, %s, %s, %s",
                                        (X == 0) ? 'B' : 'T',
                                        (Y == 0) ? 'B' : 'T',
                                        a_cond,
                                        a_Rn,
                                        a_Rm,
                                        a_Rs,
                                        a_Rd
                                );
                        } else if (dsp_op == 1) {
                                sprintf (si.canonical, "%s%c%s\t%s, %s, %s%s%s",
                                        (X == 0) ? "SMLAW" : "SMULW",
                                        (Y == 0) ? 'B' : 'T',
                                        a_cond,
                                        a_Rn,
                                        a_Rm,
                                        a_Rs,
                                        (X == 0) ? ", " : "",
                                        (X == 0) ? a_Rd : ""
                                );
                        } else if (dsp_op == 2) {
                                sprintf (si.canonical, "SMLAL%c%c%s\t%s, %s, %s, %s",
                                        (X == 0) ? 'B' : 'T',
                                        (Y == 0) ? 'B' : 'T',
                                        a_cond,
                                        a_Rd,
                                        a_Rn,
                                        a_Rm,
                                        a_Rs
                                );
                        } else {
                                sprintf (si.canonical, "SMUL%c%c%s\t%s, %s, %s",
                                        (X == 0) ? 'B' : 'T',
                                        (Y == 0) ? 'B' : 'T',
                                        a_cond,
                                        a_Rn,
                                        a_Rm,
                                        a_Rs
                                );
                        }
                }

        } else if ((cls == 0 || cls == 1) && ! ((opcode & 0xc) == 0x8 && S == 0)) {

                // data processing instructions 1,3,6

                if (SBITUSED[opcode] == '0') a_S = "";

                if (DATAOPNS[opcode] == '3') {          // 3-operand
                        sprintf(si.canonical, "%s%s%s\t%s, %s, %s%s",
                                DATAPROC[opcode],
                                a_cond,
                                a_S,
                                a_Rd,
                                a_Rn,
                                (cls == 0) ? "" : C3 "#",
                                (cls == 0) ? shift_Rm(ins) : hex(get_rimm(ins))
                        );
                } else {                                // 2-operand
                        sprintf(si.canonical, "%s%s%s\t%s, %s%s",
                                DATAPROC[opcode],
                                a_cond,
                                a_S,
                                USE_RD[opcode] == '1' ? a_Rd : a_Rn,
                                (cls == 0) ? "" : C3 "#",
                                (cls == 0) ? shift_Rm(ins) : hex(get_rimm(ins))
                        );
                }

                if (Rd == 15) {
                        si.is_branch = true;
                        if (cond == 0xe) si.is_endflow = true;
                }

                if (opcode == 13 && Rd == 15 && Rm == 14 && b4 == 0 && shift_amt == 0 && S == 0) {
                        sprintf(si.resolved, "RET%s", a_cond);
                }

                if ((opcode == 2 || opcode == 4) && cls == 1 && Rn == 15) {

                        // adr Rd, #...

                        si.is_adr = true;
                        si.has_target = true;
                        if (opcode == 2) {
                                si.target_address = addr - get_rimm(ins) + 8;
                        } else {
                                si.target_address = addr + get_rimm(ins) + 8;
                        }
                        si.target_size = 4;
                        sprintf(si.resolved, "ADR%s%s\t%s, %s%s",
                                a_cond,
                                a_S,
                                a_Rd,
                                C4,
                                hex(si.target_address)
                        );
                }

        } else if (cls == 1 && (opcode == 9 || opcode == 0xb) && S == 0 && Rd == 0xf) {

                // move immediate to status register 8

                sprintf(si.canonical, "MSR%s\t%sPSR_%s%s%s%s"C1", "C3"#%s",
                        a_cond,
                        (R == 0) ? C5 "C" : C5 "S",
                        ((ins >> 16) & 1) ? "c" : "",
                        ((ins >> 17) & 1) ? "x" : "",
                        ((ins >> 18) & 1) ? "s" : "",
                        ((ins >> 19) & 1) ? "f" : "",
                        hex(get_rimm(ins))
                );

        } else if (cls == 2 || (cls == 3 && b4 == 0)) {

                // load/store immediate/register offset 9,10

                sprintf(buf, ",%s%s%s" C1,
                        (cls == 2) ? C3 "#" : "",
                        (U == 1) ? "" : "-",
                        (cls == 2) ? hex(imm12) : shift_Rm(ins)
                );
                if (cls == 2 && imm12 == 0) buf[0] = '\0';

                if (P == 1) {
                        // pre-indexed or offset ldr[b]/str[b]
                        sprintf(si.canonical, "%s%s%s\t%s, [%s%s]%s",
                                (L == 0) ? "STR" : "LDR",
                                a_cond,
                                (B == 0) ? "" : "B",
                                a_Rd,
                                a_Rn,
                                buf,
                                (W == 1) ? "!" : ""
                        );
                } else {
                        // post-indexed ldr[b][t]/str[b][t]
                        sprintf(si.canonical, "%s%s%s%s\t%s, [%s]%s",
                                (L == 0) ? "STR" : "LDR",
                                a_cond,
                                (B == 0) ? "" : "B",
                                (W == 0) ? "" : "T",
                                a_Rd,
                                a_Rn,
                                buf
                        );
                }

                if (Rd == 15) {
                        si.is_branch = true;
                        if (cond == 0xe) si.is_endflow = true;
                }

                if (cls == 2 && W == 0 && P == 1 && L == 1 && Rn == 0xf) {

                        // ldr Rn, =...

                        si.has_target = si.needs_target_data = true;
                        si.target_address = (U == 1) ? addr + imm12 + 8 : addr - imm12 + 8;
                        si.target_size = (B == 0) ? 4 : 1;
                        si.is_ldrvalue = true;
                        if (target_data_given) {
                                sprintf(si.resolved, "LDR%s%s\t%s, %s=%s",
                                        a_cond,
                                        (B == 0) ? "" : "B",
                                        a_Rd,
                                        C3,
                                        hex((B == 0) ? target_data : target_data & 0xff)
                                );
                        }

                }

        } else if (cls == 4 && cond != 0xf) {

                // load/store multiple 13

                s = rlist(reglist);
                idx = (L << 2) | (P << 1) | U;
                sprintf(si.canonical, "%s%s%s\t%s%s, {%s}%s",
                        (L == 0) ? "STM" : "LDM",
                        a_cond,
                        (Rn == 13) ? MULTIPLES2[idx] : MULTIPLES1[idx & 3],
                        a_Rn,
                        (W == 0) ? "" : "!",
                        s,
                        (b22 == 0) ? "" : "^"
                );

                if (L == 1 && (reglist & 0x8000) != 0) {
                        si.is_branch = true;
                        if (cond == 0xe) si.is_endflow = true;
                }

        } else if (cls == 5) {

                // b, bl, bx ,blx 15,16

                if (imm24 & 0x800000) imm24 |= 0xff000000;
                target = addr + 8 + (imm24 << 2);
                size = 4;
                if (cond == 0xf) {
                        target += (b24 << 1);
                        size = 2;
                }

                tname = localName(target);

                if (cond != 0xf) {
                        sprintf(si.canonical, "%s%s\t"C4"%s",
                                (b24 == 0) ? "B" : "BL",
                                a_cond,
                                tname == NULL ? hex(target) : tname
                        );
                } else {
                        sprintf(si.canonical, "BLX\t"C4"%s",
                                tname == NULL ? hex(target) : tname
                        );
                }

                si.has_target = true;
                si.target_address = target;
                si.target_size = size;
                si.is_branch = true;
                if (cond == 0xe && b24 == 0) si.is_endflow = true;

        } else if (cls == 6) {

                // cp load/store 17

        } else if (cls == 7 && b24 == 0) {

                // cdp/mcr/mrc 18,19

                sprintf(si.canonical, "%s%s\t"C5"p%i"C1", "C5"%i"C1", "C2"%s%i"C1", "C5"c%i"C1", "C5"c%i"C1", "C5"%i",
                        (b4 == 0) ? "CDP" : ((b20 == 0) ? "MCR" : "MRC"),
                        cond==0xf ? "2" : a_cond,
                        cp_num,
                        (b4 == 0) ? (ins >> 20) & 0xf : (ins >> 21) & 0x7,
                        (b4 == 0) ? "c" : "R",
                        Rd,
                        CRn,
                        CRm,
                        opcode2
                );


        } else if (cls == 7 && b24 == 1 && cond != 0xf) {

                // swi 20

                sprintf(si.canonical, "SWI%s\t"C3"%s",
                        a_cond,
                        hex(imm24)
                );

        } else {

                // undefined instruction

        }

        postprocess(&si);
        return &si;

}

#pragma warn -aus
instruction * dis_thumb_v(unsigned int addr, unsigned int ins, bool target_data_given, unsigned int target_data) {

        static instruction si;
        unsigned int ins2, cls3, cls4, cls5, cls6, opcode1, opcode2, cond, reglist, n;
        unsigned int R02, R35, R68, R810;
        char *a_R02, *a_R35, *a_R68, *a_R810, *a_cond, *s, *tname;
        unsigned int b0, b7, b8, b9, b10, b11, b12;

        memset(&si, 0, sizeof(si));

        ins2 = (ins >> 16) & 0xffff;
        ins &= 0xffff;

        R02 = ins & 0x07;
        R35 = (ins >> 3) & 0x07;
        R68 = (ins >> 6) & 0x07;
        R810 = (ins >> 8) & 0x07;
        a_R02 = CREGISTERS[R02];
        a_R35 = CREGISTERS[R35];
        a_R68 = CREGISTERS[R68];
        a_R810 = CREGISTERS[R810];

        cls3 = (ins >> 13) & 0x7;
        cls4 = (ins >> 12) & 0xf;
        cls5 = (ins >> 11) & 0x1f;
        cls6 = (ins >> 10) & 0x3f;

        opcode1 = (ins >> 11) & 0x3;
        opcode2 = (ins >> 8) & 0x3;
        cond = (ins >> 8) & 0xf;
        a_cond = CONDITIONS[cond];

        reglist = ins & 0xff;

        b0 = ins & 1;
        b7 = (ins >> 7) & 1;
        b8 = (ins >> 8) & 1;
        b9 = (ins >> 9) & 1;
        b10 = (ins >> 10) & 1;
        b11 = (ins >> 11) & 1;
        b12 = (ins >> 12) & 1;

        if (cls3 == 0 && opcode1 != 3) {

                // shift by immediate 1

                n = (ins >> 6) & 0x1f;
                sprintf(si.canonical, "%s\t%s, %s, "C3"#%s",
                        SHIFTS[opcode1],
                        a_R02,
                        a_R35,
                        hex(n == 0 ? 32 : n)
                );

        } else if (cls3 == 0 && opcode1 == 3) {

                // add/sub immediate/register 2,3

                n = (ins >> 6) & 0x07;
                if (b10 == 1 && n == 0) {
                        sprintf(si.canonical, "MOV\t%s, %s",
                                a_R02,
                                a_R35
                        );
                } else {
                        sprintf(si.canonical, "%s\t%s, %s, %s%s",
                                (b9 == 0) ? "ADD" : "SUB",
                                a_R02,
                                a_R35,
                                (b10 == 0) ? "" : C3 "#",
                                (b10 == 0) ? a_R68 : hex(n)
                        );
                }

        } else if (cls3 == 1) {

                // add/sub/cmp/mov immediate 4

                sprintf(si.canonical, "%s\t%s, "C3"#%s",
                        ASCM[opcode1],
                        a_R810,
                        hex(ins & 0xff)
                );

        } else if (cls6 == 0x10) {

                // data-processing reg 5

                sprintf(si.canonical, "%s\t%s, %s",
                        TDATAPROC[(ins >> 6) & 0x0f],
                        a_R02,
                        a_R35
                );

        } else if (cls6 == 0x11 && opcode2 != 0x3) {

                // special data processing 6

                R02 |= ((ins >> 4) & 0x8);
                R35 |= ((ins >> 3) & 0x8);
                sprintf(si.canonical, "%s\t%s, %s",
                        TSPROC[opcode2],
                        CREGISTERS[R02],
                        CREGISTERS[R35]
                );

                if (R02 == 15 && opcode2 != 0x1) {
                        si.is_branch = si.is_endflow = true;
                }

        } else if (cls6 == 0x11 && opcode2 == 0x3 && R02 == 0) {

                // bx/blx reg 7

                sprintf(si.canonical, "%s\t%s",
                        (b7 == 0) ? "BX" : "BLX",
                        CREGISTERS[((ins >> 3) & 0x8) | R35]
                );
                si.is_branch = si.is_endflow = true;

        } else if (cls5 == 0x9) {

                // load from literal pool 8

                sprintf(si.canonical, "LDR\t%s, [PC, "C3"#%s"C1"]",
                        a_R810,
                        hex((ins & 0xff) * 4)
                );

                si.has_target = si.needs_target_data = true;
                si.target_address = (addr & ~0x3) + 4 + (ins & 0xff) * 4;
                si.target_size = 4;
                si.is_ldrvalue = true;
                if (target_data_given) {
                        sprintf(si.resolved, "LDR\t%s, "C3"=%s",
                                a_R810,
                                hex(target_data)
                        );
                }

        } else if (cls4 == 0x5) {

                // load/store reg offset 9

                sprintf(si.canonical, "%s\t%s, [%s, %s]",
                        LSROFF[(ins >> 9) & 0x7],
                        a_R02,
                        a_R35,
                        a_R68
                );

        } else if (cls3 == 0x3) {

                // load/store w/b immediate offset 10

                n = (ins >> 6) & 0x1f;
                sprintf(si.canonical, "%s%s\t%s, [%s%s%s"C1"]",
                        (b11 == 0) ? "STR" : "LDR",
                        (b12 == 0) ? "" : "B",
                        a_R02,
                        a_R35,
                        (n == 0) ? "" : ", "C3"#",
                        (n == 0) ? "" : hex(n * ((b12 == 0) ? 4 : 1))
                );


        } else if (cls4 == 0x8) {

                // load/store h immediate offset 11

                n = (ins >> 6) & 0x1f;
                sprintf(si.canonical, "%s\t%s, [%s%s%s"C1"]",
                        (b11 == 0) ? "STRH" : "LDRH",
                        a_R02,
                        a_R35,
                        (n == 0) ? "" : ", "C3"#",
                        (n == 0) ? "" : hex(n * 2)
                );

        } else if (cls4 == 0x9) {

                // load/store to/from stack 12

                n = ins & 0xff;
                sprintf(si.canonical, "%s\t%s, [SP%s%s"C1"]",
                        (b11 == 0) ? "STR" : "LDR",
                        a_R810,
                        (n == 0) ? "" : ", "C3"#",
                        (n == 0) ? "" : hex(n * 4)
                );

        } else if (cls4 == 0xa) {

                // add to sp/pc 13

                n = (ins & 0xff) * 4;
                sprintf(si.canonical, "ADD\t%s, %s, "C3"#%s",
                        a_R810,
                        (b11 == 0) ? CREGISTERS[15] : CREGISTERS[13],
                        hex(n)
                );

                if (b11 == 0) {
                        si.is_adr = true;
                        si.target_address = ((addr + 4) & ~0x3) + n;
                        si.target_size = 4;
                        sprintf(si.canonical, "ADR\t%s, "C4"%s",
                                a_R810,
                                hex(si.target_address)
                        );
                }

        } else if (cls4 == 0xb) {

                // misc instructions 14

                if (((ins >> 8) & 0xf) == 0) {

                        // adjust SP M1

                        sprintf(si.canonical, "%s\t%s, "C3"#%s",
                                (b7 == 0) ? "ADD" : "SUB",
                                CREGISTERS[13],
                                hex((ins & 0x7f) * 4)
                        );

                } else if (b9 == 0 && b10 == 1) {

                        // push/pop register list M2

                        s = rlist(reglist);
                        sprintf(si.canonical, "%s\t{%s%s%s}",
                                (b11 == 0) ? "PUSH" : "POP",
                                s,
                                (b8 == 1) ? ((s == 0) ? "" : ",") : "",
                                (b8 == 1) ? ((b11 == 0) ? CREGISTERS[14] : CREGISTERS[15]) : ""
                        );
                        if (b11 == 1 && b8 == 1) {
                                si.is_branch = si.is_endflow = true;
                        }

                } else if (((ins >> 8) & 0xf) == 0x0e) {

                        // breakpoint M3

                        sprintf(si.canonical, "BKPT\t"C5"%s",
                                hex(ins & 0xff)
                        );

                }

        } else if (cls4 == 0xc) {

                // load/store multiple 15
                s = rlist(reglist);
                sprintf(si.canonical, "%s\t%s!, {%s}",
                        (b11 == 0) ? "STMIA" : "LDMIA",
                        a_R810,
                        s
                );

        } else if (cls4 == 0xd && cond != 0xe && cond != 0xf) {

                // conditional branch 16

                n = ins & 0xff;
                if (n & 0x80) n |= 0xffffff00;
                n = addr + 4 + (n << 1);

                tname = localName(n);
                sprintf(si.canonical, "B%s\t"C4"%s",
                        a_cond,
                        (tname == NULL) ? hex(n) : tname
                );
                si.is_branch = true;
                si.is_conditional = true;
                si.has_target = true;
                si.target_address = n;
                si.target_size = 2;

        } else if (cls4 == 0xd && cond == 0xf) {

                // swi 18

                sprintf(si.canonical, "SWI\t"C3"%s",
                        hex(ins & 0xff)
                );

        } else if (cls5 == 0x1c) {

                // unconditional branch 1-word 19

                n = ins & 0x7ff;
                if (n & 0x400) n |= 0xfffff800;
                n = addr + 4 + (n << 1);

                tname = localName(n);
                sprintf(si.canonical, "B\t"C4"%s",
                        (tname == NULL) ? hex(n) : tname
                );
                si.is_branch = si.is_endflow = true;
                si.has_target = true;
                si.target_address = n;
                si.target_size = 2;

        } else if (cls5 == 0x1e) {

                // bl/blx prefix 22

                cls5 = (ins2 >> 11) & 0x1f;
                if ((cls5 == 0x1d && (ins2 & 1) == 0) || cls5 == 0x1f) {
                        n = ins & 0x7ff;
                        if (n & 0x400) n |= 0xfffff800;
                        n = (n << 12) + addr + 4 + ((ins2 & 0x7ff) << 1);
                        if (cls5 == 0x1d) n &= ~0x3;

                        tname = localName(n);
                        sprintf(si.canonical, "%s\t"C4"%s",
                                (cls5 == 0x1d) ? "BLX" : "BL",
                                (tname == NULL) ? hex(n) : tname
                        );
                }
                si.is_branch = true;
                si.is_exchange = (cls5 == 0x1d);
                si.has_target = true;
                si.target_address = n;
                si.target_size = (cls5 == 0x1d) ? 4 : 2;

        } else if ((cls5 == 0x1d && b0 == 0) || cls5 == 0x1f) {

                // bl/blx suffix 20,23

                strcpy(si.canonical, " ");

        } else {

                // undefined instruction
                strcpy(si.canonical, C6 "???");

        }

        postprocess(&si);
        return &si;

}
#pragma warn +aus

int insclass_arm(unsigned int ins) {

        unsigned int cond, cls, opcode, misc, S, Rn, Rd, Rm, Rs;
        unsigned int b4, b7, b20, b21, b22, b23, b24;

        cond = (ins >> 28) & 0x0f;
        opcode = (ins >> 21) & 0x0f;
        S = (ins >> 20) & 1;
        cls = (ins >> 25) & 0x07;
        misc = (ins >> 4) & 0x0f;

        Rn = (ins >> 16) & 0x0f;
        Rd = (ins >> 12) & 0x0f;
        Rm = ins & 0x0f;
        Rs = (ins >> 8) & 0x0f;

        b4 = (ins >> 4) & 1;
        b7 = (ins >> 7) & 1;
        b20 = (ins >> 20) & 1;
        b21 = (ins >> 21) & 1;
        b22 = (ins >> 22) & 1;
        b23 = (ins >> 23) & 1;
        b24 = (ins >> 24) & 1;

        if (cond == 0xf && (cls == 2 || cls == 3)) {
                return ICA_CPLD;
        } else if (cls == 0 && b4 == 1 && b7 == 1) {
                if (misc == 9 && b24 == 0 && b23 == 0 && b22 == 0) {
                        return ICA_MUL;
                } else if (misc == 9 && b24 == 0 && b23 == 1) {
                        return ICA_MULL;
                } else if (b24 == 1 && b23 == 0 && b21 == 0 && b20 == 0 && misc == 9 && Rs == 0) {
                        return ICA_SWAP;
                } else if (misc == 0xb && b22 == 0 && Rs == 0) {
                        return ICA_LSHALFR;
                } else if (misc == 0xb && b22 == 1) {
                        return ICA_LSHALFI;
                } else if ((misc == 0xd || misc == 0xf) && b22 == 0 && b20 == 0 && Rs == 0) {
                        return ICA_LSDWORDR;
                } else if ((misc == 0xd || misc == 0xf) && b22 == 0 && b20 == 1 & Rs == 0) {
                        return ICA_LSSHBR;
                } else if ((misc == 0xd || misc == 0xf) && b22 == 1 && b20 == 0) {
                        return ICA_LSDWORDI;
                } else if ((misc == 0xd || misc == 0xf) && b22 == 1 && b20 == 1) {
                        return ICA_LSSHBI;
                } else {
                        return ICA_UNDEFINED;
                }
        } else if (cls == 0 && (opcode & 0xc) == 0x8 && S == 0) {
                if (misc == 0 && b21 == 0 && Rn == 0xf && Rs == 0 && Rm == 0) {
                        return ICA_MRSR;
                } else if (misc == 0 && b21 == 1 && Rd == 0xf && Rs == 0) {
                        return ICA_MSRR;
                } else if (misc == 1 && b22 == 0 && b21 == 1 && Rn == 0xf && Rd == 0xf && Rs == 0xf) {
                        return ICA_BXR;
                } else if (misc == 3 && b22 == 0 && b21 == 1 && Rn == 0xf && Rd == 0xf && Rs == 0xf) {
                        return ICA_BLXR;
                } else if (misc == 1 && b22 == 1 && b21 == 1 && Rn == 0xf && Rs == 0xf) {
                        return ICA_CLZ;
                } else if (misc == 5 && Rs == 0) {
                        return ICA_DSPAS;
                } else if (misc == 7 && b22 == 0 && b21 == 1) {
                        return ICA_BKPT;
                } else if (misc >= 8) {
                        return ICA_DSPM;
                } else {
                        return ICA_UNDEFINED;
                }
        } else if (cls == 0 && b4 == 0 && ! ((opcode & 0xc) == 0x8 && S == 0)) {
                return ICA_DPROCIS;
        } else if (cls == 0 && b4 == 1 && ! ((opcode & 0xc) == 0x8 && S == 0)) {
                return ICA_DPROCRS;
        } else if (cls == 1 && ! ((opcode & 0xc) == 0x8 && S == 0)) {
                return ICA_DPROCI;
        } else if (cls == 1 && (opcode == 9 || opcode == 0xb) && S == 0 && Rd == 0xf) {
                return ICA_MSRI;
        } else if (cls == 2) {
                return ICA_LSI;
        } else if (cls == 3 && b4 == 0) {
                return ICA_LSR;
        } else if (cls == 4 && cond != 0xf) {
                return ICA_LSM;
        } else if (cls == 5 && cond != 0xf) {
                return ICA_BBL;
        } else if (cls == 5 && cond == 0xf) {
                return ICA_BLX;
        } else if (cls == 6) {
                return ICA_CPLS;
        } else if (cls == 7 && b24 == 0 && b4 == 0) {
                return ICA_CDP;
        } else if (cls == 7 && b24 == 0 && b4 == 1) {
                return ICA_CRT;
        } else if (cls == 7 && b24 == 1 && cond != 0xf) {
                return ICA_SWI;
        } else {
                return ICA_UNDEFINED;
        }
}

int insclass_thumb(unsigned int ins) {

        unsigned int cls3, cls4, cls5, cls6, opc8, opc11, cond, b9, b10;

        cls3 = (ins >> 13) & 0x7;
        cls4 = (ins >> 12) & 0xf;
        cls5 = (ins >> 11) & 0x1f;
        cls6 = (ins >> 10) & 0x3f;
        opc11 = (ins >> 11) & 0x3;
        opc8 = (ins >> 8) & 0x3;
        cond = (ins >> 8) && 0xf;
        b9 = (ins >> 9) & 1;
        b10 = (ins >> 10) & 1;

        if (cls3 == 0 && opc11 != 3) {
                return ICT_SHI;
        } else if (cls3 == 0 && opc11 == 3 && b10 == 0) {
                return ICT_ASR;
        } else if (cls3 == 0 && opc11 == 3 && b10 == 1) {
                return ICT_ASI;
        } else if (cls3 == 1) {
                return ICT_ASCMI;
        } else if (cls6 == 0x10) {
                return ICT_DPR;
        } else if (cls6 == 0x11 && opc8 != 3) {
                return ICT_SDP;
        } else if (cls6 == 0x11 && opc8 == 3) {
                return ICT_BXR;
        } else if (cls5 == 0x9) {
                return ICT_LLP;
        } else if (cls4 == 0x5) {
                return ICT_LSR;
        } else if (cls3 == 0x3) {
                return ICT_LSWBI;
        } else if (cls4 == 0x8) {
                return ICT_LSHI;
        } else if (cls4 == 0x9) {
                return ICT_LSST;
        } else if (cls4 == 0xa) {
                return ICT_ASPPC;
        } else if (cls4 == 0xb) {
                if (b10 == 0 && b9 == 0) {
                        return ICT_AJSP;
                } else if (b10 == 1 && b9 == 0) {
                        return ICT_PP;
                } else if (b10 == 1 && b9 == 1) {
                        return ICT_BKPT;
                }
        } else if (cls4 == 0xc) {
                return ICT_LSM;
        } else if (cls4 == 0xd && cond != 0xe && cond != 0xf) {
                return ICT_CBR;
        } else if (cls4 == 0xd && cond == 0xf) {
                return ICT_SWI;
        } else if (cls5 == 0x1c) {
                return ICT_UBR;
        } else if (cls5 == 0x1d) {
                return ICT_BLXS;
        } else if (cls5 == 0x1e) {
                return ICT_BLBLXP;
        } else if (cls5 == 0x1f) {
                return ICT_BLS;
        } else {
                return ICT_UNDEFINED;
        }
}

instruction * disarm(unsigned int addr, unsigned int ins, unsigned int state[18]) {
        return dis_arm_v(addr, ins, false, 0);
}

instruction * disarm2(unsigned int addr, unsigned int ins, unsigned int state[18], unsigned int target) {
        return dis_arm_v(addr, ins, true, target);
}

instruction * disthumb(unsigned int addr, unsigned int ins, unsigned int state[18]) {
        return dis_thumb_v(addr, ins, false, 0);
}

instruction * disthumb2(unsigned int addr, unsigned int ins, unsigned int state[18], unsigned int target) {
        return dis_thumb_v(addr, ins, true, target);
}

