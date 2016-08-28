#ifndef PARSER_H
#define PARSER_H

#define NONE 0

#define INS_LINEAR		0		// ordinary instruction
#define INS_B_R			1		// unconditional branch to unknown location
#define INS_CB_R		2		// conditional branch to unknown location
#define INS_B_A			3		// unconditional branch to address
#define INS_CB_A		4		// conditional branch to address
#define INS_BL_R		5		// branch with link to unknown location
#define INS_CBL_R		6		// cond. branch with link to unknown location
#define INS_BL_A		7		// branch with link (maybe change arm/thumb)
#define INS_CBL_A		8		// conditional branch with link
#define INS_RET			9		// return instruction
#define INS_CRET		10		// conditional return instruction
#define INS_STACK		11		// stack load/store instruction
#define INS_CASE		12		// case instruction
#define INS_LDR			13		// case instruction
#define INS_STR			14		// case instruction
#define INS_UNDEFINED	255		// undefined instruction

#define SL_FLAT 0
#define SL_LR 1
#define SL_PUSH 2

#define R_INPROCESS 0
#define R_OK 1
#define R_MAYBE 2
#define R_ERR 3


typedef struct ix_stack_s {

	int size;
	int count;
	unsigned int *data;

} ix_stack;

typedef struct ix_handled_s {

	int size;
	int count;
	unsigned int *data;

} ix_handled;

typedef struct ix_function_s {

	unsigned int addr;
	int thumb;
	unsigned int low, high;

        unsigned int signature_cksum;
        unsigned int fuzzy_cksum;

        int const_count;
        unsigned int *constants;

        int strings_count;
        char **strings;




} ix_function;

typedef struct flashinfo_s {

        ix_function *ixf;
        int count;
        unsigned char *fmap;
        unsigned char *fcode;
        unsigned char *fthumb;

} flashinfo;

bool analyze(unsigned char *buf, unsigned int len, unsigned int flashbase, flashinfo *fli);

#endif