#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <mem.h>
#include "parser.h"

// exclude LANGPACK, SWUPik
// thumb case


static unsigned char *fw;		// firmware data
static unsigned int *ifw;		// firmware data (arm, uint array)
static unsigned short *hfw;		// firmware data (thumb, ushort array)
static unsigned int blen, ilen, hlen;
static unsigned int base;

static char *blocks;
static char *fmap;              // functions bitmap (1=function start)
static char *fcode;             // code bitmap (1=code)
static char *fthumb;		// thumb bitmap (1=thumb)

static int verbose;

static char *msg = "";

int OUTSIDE(unsigned int a) {

	return (a < base || a >= (base+blen-1) || blocks[(a-base)/0x20000] == 0);

}

int FEXISTS(unsigned int addr) {

	unsigned int off;

	addr &= ~1;

	if (addr < base || addr >= (base+blen-1)) return 0;
	off = (addr - base) >> 1;
	return (fmap[off >> 3] & (1 << (off & 0x7))) == 0 ? 0 : 1;

}

int FADD(unsigned int addr, int thumb) {

	unsigned int off;

	addr &= ~1;
	if (OUTSIDE(addr) || FEXISTS(addr)) return 0;

	off = (addr - base) >> 1;
	fmap[off >> 3] |= (1 << (off & 0x7));
	if (thumb) fthumb[off >> 3] |= (1 << (off & 0x7));

	return 1;

}

void V(char *fmt, ...) {

        va_list ap;
        int n;

		if (! verbose) return;

        va_start(ap, fmt);
        vprintf(fmt, ap);
        va_end(ap);

}

ix_stack *SNEW(void) {

	ix_stack *s = (ix_stack *)malloc(sizeof(ix_stack));
	s->data = (unsigned int *)malloc(16*4);
	s->count = 0;
	s->size = 16;
	return s;

}

void SFREE(ix_stack *s) {

	free(s->data);
	free(s);

}

void SPUSH(ix_stack *s, unsigned int a) {

	if (s->count >= s->size - 1) {
		s->size *= 2;
		s->data = (unsigned int *)realloc(s->data, s->size*4);
	}

	s->data[s->count] = a;
	s->count++;

}

unsigned int SPOP(ix_stack *s) {

	if (s->count == 0) return NONE;
	s->count--;
	return s->data[s->count];

}

ix_handled *HNEW(void) {

	ix_handled *h = (ix_handled *)malloc(sizeof(ix_handled));
	h->data = (unsigned int *)malloc(16*4);
	h->count = 0;
	h->size = 16;
	return h;

}

void HFREE(ix_handled *h) {

	free(h->data);
	free(h);

}

void HADD(ix_handled *h, unsigned int a) {

	if (h->count >= h->size - 1) {
		h->size *= 2;
		h->data = (unsigned int *)realloc(h->data, h->size*4);
	}

	h->data[h->count] = a;
	h->count++;

}

int HEXISTS(ix_handled *h, unsigned int a) {

	int i;

	for (i=0; i<h->count; i++) {
		if (h->data[i] == a) return 1;
	}
	return 0;

}

int instype_a(unsigned int addr, unsigned int *target) {

		unsigned int ins, cond, unc, cls, opcode, Rn, Rd, Rm, S, L, U;
		unsigned int b4, b7, b15, b24, op1, op2, a;

		*target = NONE;

		if (addr < base || addr >= (base+blen-1)) return INS_UNDEFINED;
		if ((addr & 3) != 0) return INS_UNDEFINED;

		ins = ifw[(addr-base)/4];

		cond = (ins >> 28) & 0x0f;
		unc = (cond >= 0x0e);
		cls = (ins >> 25) & 0x07;
		opcode = (ins >> 21) & 0x0f;
		Rn = (ins >> 16) & 0x0f;
		Rd = (ins >> 12) & 0x0f;
		Rm = ins & 0x0f;
		S = L = (ins >> 20) & 1;
		U = (ins >> 23) & 1;
		b4 = (ins >> 4) & 1;
		b7 = (ins >> 7) & 1;
		b15 = (ins >> 15) & 1;
		b24 = (ins >> 24) & 1;

		// addxx pc, pc, Rn,LSL#2

		if ((ins & 0x0ffffff0) == 0x008ff100) return INS_CASE;

		// bx.. lr

		if ((ins & 0x0fffffff) == 0x012fff1e) return unc ? INS_RET: INS_CRET;

		// mov.. pc, lr

		if ((ins & 0x0fffffff) == 0x01a0f00e) return unc ? INS_RET : INS_CRET;

		// mov.. pc, rn

		if ((ins & 0x0ffffff0) == 0x01a0f000) return unc ? INS_B_R : INS_CB_R;

		// add/sub.. pc, pc, N
		// (can be calculated)

		if ((ins & 0x0f3ff000) == 0x020ff000) return unc ? INS_B_R : INS_CB_R;

		// misc instructions

		if (cls == 0 && S == 0 && (opcode & 0xc) == 8 && (b4 == 0 || (b4 == 1 && b7 == 0))) {

			op1 = (ins >> 21) & 0x3;
			op2 = (ins >> 4) & 0xf;

			if (op1 == 1 && op2 == 1) {

				// bx rn

				if (Rm == 0x0e) {
					return unc ? INS_RET : INS_CRET;
				} else {
					return unc ? INS_B_R : INS_CB_R;
				}

			} else if (op1 == 1 && op2 == 3) {

				return unc ? INS_BL_R : INS_CBL_R;

			}

			return INS_LINEAR;

		}

		// multiplies, extra load/stores

		if (cls == 0 && b4 == 1 && b7 == 1) {

			return INS_LINEAR;

		}

		// data processing

		if (cls == 0 && (b4 == 0 || (b4 == 1 && b7 == 0))) {

			if (Rd == 15) return unc ? INS_B_R : INS_CB_R;

			return INS_LINEAR;

		}

		// undefined instruction

		if ((ins & 0x0fb00000) == 0x03000000) return INS_UNDEFINED;

		// move immediate to status register

		if (cls == 1 && (opcode & 0xd) == 0x9 && S == 0 && Rd == 15) return INS_LINEAR;

		// data processing immediate

		if (cls == 1) {

			if (Rd == 15) return unc ? INS_B_R : INS_LINEAR;

			return INS_LINEAR;

		}

		// undefined instruction

		if (cls == 3 && b4 == 1) return INS_UNDEFINED;

		// undefined instruction

		if ((ins & 0xf8000000) == 0xf0000000) return INS_UNDEFINED;

		// undefined instruction

		if (cls == 4 && cond == 0xf) return INS_UNDEFINED;

		// ldr.. pc,[sp],#N

		if ((ins & 0x0ffff000) == 0x049df000) return unc ? INS_RET : INS_CRET;

		// ldr.. pc, [pc,#N]

		if ((ins & 0x0f7ff000) == 0x051ff000) {

			a = addr + 8 + ((U == 0) ? 0-(ins & 0xfff) : (ins & 0xfff));
			if ((a & 3) != 0) return INS_UNDEFINED;
			*target = ifw[(a-base)/4];
			return unc ? INS_B_A : INS_CB_A;

		}

		// ldr/str.. rn, [...]

		if (cls == 2 || cls == 3) {

			if (Rd == 15) return unc ? INS_B_R : INS_CB_R;
			if (Rn == 13) return INS_LINEAR;
			return (L == 0) ? INS_STR : INS_LDR;

		}

		// ldm.... rn[!], {...,pc}

		if (cls == 4 && L == 1 && b15 == 1) {

			return unc ? INS_RET : INS_CRET;

		}

		// ldm..fd/stm..fd

		if (cls == 4) {

			if (Rn == 13) {
				return INS_STACK;
			} else {
				return (L == 1) ? INS_LDR : INS_STR;
			}

		}

		// blx

		if (cls == 5 && cond == 0xf) {

			a = ((ins & 0x00ffffff) << 2) | (((ins >> 24) << 1) & 0x2);
			if ((a & 0x02000000) != 0) a |= 0xfc000000;
			*target = (addr + 8 + a) | 1;
			return INS_BL_A;

		}

		// b/bl

		if (cls == 5) {

			a = ((ins & 0x00ffffff) << 2);
			if ((a & 0x02000000) != 0) a |= 0xfc000000;
			*target = addr + 8 + a;
			if (b24 == 0) {
				return unc ? INS_B_A : INS_CB_A;
			} else {
				return unc ? INS_BL_A : INS_CBL_A;
			}

		}

		// undefined instruction

		if ((ins & 0xff000000) == 0xff000000) return INS_UNDEFINED;

		// cdp/mrc/mcr/swi/...

		return INS_LINEAR;

}

int instype_t(unsigned int addr, unsigned int *target) {

		unsigned short ins, preins, preins2, cls3, cls4, cls5, cls6, Rm, HRm, Rd, HRd;
		unsigned short cond, sdpc, b0, b7, b8, b9, b10, b11, opc;
		unsigned int a;

		*target = NONE;

		if (addr < base || addr >= (base+blen-1)) return INS_UNDEFINED;
		addr &= ~1;

		ins = hfw[(addr-base)/2];

		cls3 = (ins >> 13) & 0x7;
		cls4 = (ins >> 12) & 0xf;
		cls5 = (ins >> 11) & 0x1f;
		cls6 = (ins >> 10) & 0x3f;
		sdpc = (ins >> 8) & 3;
		Rm = (ins >> 3) & 7;
		HRm = (ins >> 3) & 0xf;
		Rd = ins & 7;
		HRd = ((ins >> 4) & 0x8) | Rd;
		b0 = ins & 1;
		b7 = (ins >> 7) & 1;
		b8 = (ins >> 8) & 1;
		b9 = (ins >> 9) & 1;
		b10 = (ins >> 10) & 1;
		b11 = (ins >> 11) & 1;

		// shift, add/substract/compare/move

		if (cls3 == 0) return INS_LINEAR;

		if (cls3 == 1) {

			// ! set value
			return INS_LINEAR;

		}

		// data-processing register

		if (cls6 == 0x10) return INS_LINEAR;

		// special data processing

		if (cls6 == 0x11 && sdpc != 3) {

			if (HRd == 15 && (sdpc == 0 || sdpc == 2)) {
/*
                                if (sdpc == 0 && HRm <= 7) {
                       			preins = hfw[(addr-base)/2 - 1];
                       			preins2 = hfw[(addr-base)/2 - 2];
                                        if (preins == (0x0040 | (HRm << 3) | HRm) &&
                                            (preins2 & 0xf800) == 0x5800
                                        ) {
                                                return INS_CASE;
                                        }
                                }
 */
				return INS_B_R;
			} else {
				return INS_LINEAR;
			}

		}

		// branch/link/exchange Rm

		if (cls6 == 0x11 && sdpc == 3) {

			if (HRm == 14) return INS_RET;
			return (b7 == 0) ? INS_B_R : INS_BL_R;

		}

		// load from literal pool

		if (cls5 == 0x9) {

			// ! set target (value)
			return INS_LINEAR;

		}

		// load/store register offset

		if (cls4 == 0x5) {

			opc = (ins >> 9) & 0x7;
			return (opc <= 2) ? INS_STR : INS_LDR;

		}

		// load/store word/byte immediate offset

		if (cls3 == 0x3 || cls4 == 0x8) {

			// ! set value

			return (b11 == 0) ? INS_STR : INS_LDR;

		}

		// load/store to/from stack

		if (cls4 == 0x9) {

			return INS_LINEAR;

		}

		// add to SP or PC

		if (cls4 == 0xa) {

			if (b11 == 0) { // pc
				// ! adr: set value
				return INS_LINEAR;
			} else {
				return INS_LINEAR;
			}

		}

		// misc

		if (cls4 == 0x0b) {

			if (b11 == 0 && b10 == 1 && b9 == 0) {
				// push
				return INS_STACK;
			} else if (b11 == 1 && b10 == 1 && b9 == 0) {
				// pop
				return (b8 == 1) ? INS_RET : INS_STACK;
			} else {
				return INS_LINEAR;
			}
         
		}

		// load/store multiple

		if (cls4 == 0x0c) {

			return (b11 == 1) ? INS_LDR : INS_STR;

		}

		// conditional branch / swi

		if (cls4 == 0x0d) {

			cond = (ins >> 8) & 0xf;
			if (cond == 0xe) return INS_UNDEFINED;
			if (cond == 0xf) return INS_LINEAR;

			a = (unsigned int)(ins & 0xff) << 1;
			if ((a & 0x100) != 0) a |= 0xffffff00;
			*target = (addr + 4 + a) | 1;
			return INS_CB_A;

		}

		// unconditional branch

		if (cls5 == 0x1c) {

			a = (unsigned int)(ins & 0x7ff) << 1;
			if ((a & 0x800) != 0) a |= 0xfffff000;
			*target = (addr + 4 + a) | 1;
			return INS_B_A;

		}

		// bl/blx prefix

		if (cls5 == 0x1e) {

			return INS_LINEAR;

		}

		if (cls5 == 0x1d && b0 == 1) return INS_UNDEFINED;

		// blx suffix

		if (cls5 == 0x1d) {

			preins = hfw[(addr-base)/2 - 1];
			if ((preins & 0xf800) != 0xf000) return INS_UNDEFINED;
			a = (unsigned int)(preins & 0x7ff) << 12;
			if ((a & 0x400000) != 0) a |= 0xff800000;
			a |= ((unsigned int)(ins & 0x7ff) << 1);
			*target = (addr + 2 + a) & ~3;
			return INS_BL_A;

		}

		// bl suffix

		if (cls5 == 0x1f) {

			preins = hfw[(addr-base)/2 - 1];
			if ((preins & 0xf800) != 0xf000) return INS_UNDEFINED;
			a = (unsigned int)(preins & 0x7ff) << 12;
			if ((a & 0x400000) != 0) a |= 0xff800000;
			a |= ((unsigned int)(ins & 0x7ff) << 1);
			*target = (addr + 2 + a) | 1;
			return INS_BL_A;

		}

		// something other...

		return INS_UNDEFINED;

}


int rollout(unsigned int addr, int reliable) {


	static unsigned int from = 0;

	static int level = 0;
	int i;
        unsigned int j;

	ix_stack *S;
	ix_handled *H;
	unsigned int a, aa, tgt;
	unsigned int ins, ins1, ins2;
	unsigned int arm, thumb, inswidth, r;
	unsigned char c;
	int (*instype_f)(unsigned int, unsigned int *);

	if ((addr & 1) == 0) {
		inswidth = 4;
		arm = 1;
		thumb = 0;
		instype_f = instype_a;
	} else {
		inswidth = 2;
		arm = 0;
		thumb = 1;
		instype_f = instype_t;
	}
	addr &= ~1;

	level++;

	if (reliable) FADD(addr, thumb);

	a = addr;
	r = R_INPROCESS;
	S = SNEW();			// stack
	H = HNEW();			// handled addresses

	while (r == R_INPROCESS) {

		//fprintf(stderr, "    %08X\n", a);

		// check for non-code conditions

		msg = "Non-code area";

		if (arm) {

			ins1 = ifw[(a-base)/4];
			ins2 = ifw[(a-base)/4+1];

                        // zero area
			if (ins1 == 0 && ins2 == 0) { r = R_ERR; break; }

                        // text string
			for (j=a-base; j<a-base+8; j++) {
				c = fw[j];
				if (c < 0x20 || c > 0x7f) break;
			}
			if (j == a-base+8) { r = R_ERR; break; }

		}

                if (reliable) {
                        j = (a - base) >> 1;
                        fcode[j >> 3] |= (1 << (j & 0x7));
                        if (thumb) fthumb[j >> 3] |= (1 << (j & 0x7));
                }

		switch ((*instype_f)(a, &tgt)) {

			case INS_LINEAR:		// linear-flow or unknown-dest-flow+continue
			case INS_LDR:
			case INS_STR:
			case INS_CRET:
			case INS_CB_R:
			case INS_STACK:
			case INS_BL_R:
			case INS_CBL_R:

				a += inswidth;
				break;

			case INS_B_R:			// branch to undefined location (register)
									// b/bx rn; add pc,...; ldr pc,...

				a =  SPOP(S);
				if (a == NONE) r = R_OK;
				HADD(H,a);
				break;

			case INS_B_A:			// unconditional branch
									// b/bx N; add pc,N; ldr pc,[pc,N]

				aa = tgt;
				if (OUTSIDE(aa) || HEXISTS(H, aa) || FEXISTS(aa)) {
					a = SPOP(S);
					if (a == NONE) r = R_OK;
					HADD(H, a);
				} else {
					if ((aa & 1) != thumb) {
						r = R_ERR;
						msg = "direct a/t branch";
					} else {
						HADD(H,aa);
						a = aa;
					}
				}
				break;

			case INS_CB_A:			// conditional branch
							// bne/bxne N; addne pc,N; ldrne pc,[pc,N]

				aa = tgt;
				if ((aa & 1) != thumb) {
					r = R_ERR;
					msg = "direct a/t branch";
				} else {
					if (! (HEXISTS(H,aa) || FEXISTS(aa) || OUTSIDE(aa))) SPUSH(S,aa);
					a += inswidth;
				}
				break;

			case INS_BL_A:			// branch with link
			case INS_CBL_A:			// bl/blx N; blne/blxne N

				if (reliable) {
					from = a;
					aa = tgt;
					if (! (OUTSIDE(aa) || FEXISTS(aa))) {
						rollout(aa, 1);
					}
				}
				a += inswidth;
				break;

			case INS_CASE:			// case instruction
							// addxx pc,pc,Rn,LSL#2

				if (arm) {

					while (1) {
						a += 4;
						ins = ifw[(a-base)/4];
						if ((ins & 0xff000000) != 0xea000000) break;
						instype_a(a, &tgt);
						aa = tgt;
						if (! (HEXISTS(H,aa) || FEXISTS(aa) || OUTSIDE(aa))) SPUSH(S,aa);
					}
					a = SPOP(S);
					if (a == NONE) r = R_OK;
					HADD(H,a);

				} else {

				}
				break;

			case INS_RET:			// return instruction
									// bx lr; mov pc,lr; ldmfd sp!,{...,pc}; ldr pc,[sp],#4

				r = R_OK;
				a = SPOP(S);
				if (a != NONE) {
					r = R_INPROCESS;
					HADD(H,a);
				}
				break;

			case INS_UNDEFINED:

				r = R_ERR;
				msg = "undefined instruction";
				break;
	
		}

	}

	SFREE(S);
	HFREE(H);

	if (reliable && r != R_OK) {
		for (i=0; i<level; i++) V(" ");
		V("- %08X... %s (%08X)\n", addr, msg, a);
	}

	level--;

	return r;

}


bool analyze(unsigned char *buf, unsigned int len, unsigned int flashbase, flashinfo *fli) {

	static unsigned int F = 0xf0000000;
	static unsigned int E = 0xe0000000;

	unsigned int i, j, a, l, ins1, ins2, ins3, thumb;
	ix_function tf;
	char *p;

	ix_function *flist = NULL;
	int fsize;
	int fcount;

        base = flashbase;
	fw = buf;
	ifw = (unsigned int *)fw;
	hfw = (unsigned short *)fw;
	blen = len;
	hlen = len/2;
	ilen = len/4;

	blocks = (unsigned char *)malloc(ilen / 0x8000);
	fmap = (unsigned char *)malloc(len / 16);
	fcode = (unsigned char *)malloc(len / 16);
	fthumb = (unsigned char *)malloc(len / 16);
	memset(fmap, 0, len / 16);
	memset(fcode, 0, len / 16);
	memset(fthumb, 0, len / 16);

	for (i=0; i<ilen; i+=0x8000) {

		blocks[i/0x8000] =
				((ifw[i] == 0x00534646) ||
				 (ifw[i] == 0x5f534646) ||
				 (ifw[i] == 0x55464545) ||
				 (ifw[i] == 0x494c4545)
				) ? 0 : 1;

	}

	V("Searching for ARM procedures...\n");

	rollout(base, 1);

	for (i=0x8000; i<ilen; i++) {

		a = base+i*4;
		if (OUTSIDE(a)) continue;

		ins1 = ifw[i];
		ins2 = ifw[i+1];
		

		// STMFD SP!, ... or
		// STR LR, [SP,#-4]

		if ( (ins1 & 0xffffc000) == 0xe92d4000 ||
			 (ins1 & 0xffffffff) == 0xe52de004
		) {
			if (rollout(a, 0) == R_OK) {
				rollout(a, 1);
			}
		}
		else if ( (ins1 & 0xffff0fff) == 0xe59f0000 &&
				  (ins2 & 0xfffffff0) == 0xe12fff10
		) {
			rollout(a, 1);
			a = ifw[i+2];
			if (! OUTSIDE(a)) rollout(a, 1);
		}

	}

	V("Searching for ARM/Thumb links...\n");


	for (i=0; i<ilen; i++) {

		a = ifw[i];
		if (OUTSIDE(a)) continue;
		if (FEXISTS(a)) continue;

		if ((a & 3) == 0) {

			//arm
			ins1 = ifw[(a-base)/4];
			ins2 = ifw[(a-base)/4+1];
			ins3 = ifw[(a-base)/4+2];
			if ((ins1 & 0xffffc000) == 0xe92d4000 && rollout(a, 0) == R_OK) {
				rollout(a, 1);
			} else if (        (ins1 & F) == E
					&& (ins2 & F) == E
					&& ((ins3 & F) == E || (ins3 & 0x0f000000) == 0x0a000000)
					&& rollout(a, 0) == R_OK) {
				rollout(a, 1);
			} else if ((ins1 & 0xfffcf000) == 0xe3500000 && rollout(a, 0) == R_OK) {
				rollout(a, 1);
			} else if (ins1 == 0xe12fff1e) {
				rollout(a, 1);
			} else if ((ins1 & 0xffff0000) == 0xe3a00000 && ins2 == 0xe12fff1e) {
				rollout(a, 1);
			} else if ((ins1 & 0xff7ff000) == 0xe51ff000) {
				rollout(a, 1);
			} else {
				// just for tests
				l = 0;
				j = a-base;
				while ((fw[j] >= 0x20 && fw[j] <= 0x7f) || fw[j] == 0x0d || fw[j] == 0x0a) { l++; j++; }
				if (l > 2) continue;
				if ((ins1 & F) != E) continue;
				//if (rollout(a, 0) == R_OK) printf(": %08X\n", a);
			}

		} else if ((a & 1) == 1) {

			//thumb
			a &= ~1;
			ins1 = hfw[(a-base)/2];
			if ((ins1 & 0xfe00) == 0xb400 && rollout(a|1, 0) == R_OK) {
				rollout(a|1, 1);
			}

		}

	}

	// make array of functions
	// rollout: calc boundaries, references

	fsize = fcount = 0;

	for (i=0; i<(len>>1); i++) {

		if ((fmap[i >> 3] & (1 << (i & 0x7))) != 0) {

			a = base + (i << 1);
			thumb = ((fthumb[i >> 3] & (1 << (i & 0x7))) == 0) ? 0 : 1;

			if (fsize <= fcount) {
				fsize = fcount + 256;
				flist = (ix_function *)realloc(flist, fsize * sizeof(ix_function));
			}

			flist[fcount].addr = a;
			flist[fcount].thumb = thumb;
			fcount++;

		}

	}

        free(blocks);

	fli->ixf = flist;
        fli->count = fcount;
        fli->fmap = fmap;
        fli->fcode = fcode;
        fli->fthumb = fthumb;

	return true;

}



