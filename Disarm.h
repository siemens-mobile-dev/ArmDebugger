#ifndef DisarmH
#define DisarmH

#define ICA_UNDEFINED 0
#define ICA_CPLD 1
#define ICA_MUL 2
#define ICA_MULL 3
#define ICA_SWAP 4
#define ICA_LSHALFR 5
#define ICA_LSHALFI 6
#define ICA_LSDWORDR 7
#define ICA_LSSHBR 8
#define ICA_LSDWORDI 9
#define ICA_LSSHBI 10
#define ICA_MRSR 11
#define ICA_MSRR 12
#define ICA_BXR 13
#define ICA_BLXR 14
#define ICA_CLZ 15
#define ICA_DSPAS 16
#define ICA_BKPT 17
#define ICA_DSPM 18
#define ICA_DPROCIS 19
#define ICA_DPROCRS 20
#define ICA_DPROCI 21
#define ICA_MSRI 22
#define ICA_LSI 23
#define ICA_LSR 24
#define ICA_LSM 25
#define ICA_BBL 26
#define ICA_BLX 27
#define ICA_CPLS 28
#define ICA_CDP 29
#define ICA_CRT 30
#define ICA_SWI 31

#define ICT_SHI 50
#define ICT_ASR 51
#define ICT_ASI 52
#define ICT_ASCMI 53
#define ICT_DPR 54
#define ICT_SDP 55
#define ICT_BXR 56
#define ICT_LLP 57
#define ICT_LSR 58
#define ICT_LSWBI 59
#define ICT_LSHI 60
#define ICT_LSST 61
#define ICT_ASPPC 62
#define ICT_AJSP 63
#define ICT_PP 64
#define ICT_BKPT 65
#define ICT_LSM 66
#define ICT_CBR 67
#define ICT_SWI 68
#define ICT_UBR 69
#define ICT_BLXS 70
#define ICT_BLBLXP 71
#define ICT_BLS 72
#define ICT_UNDEFINED 73

typedef struct _instruction {

        unsigned int data;
        int length;

        char canonical[80];
        char resolved[80];
        char colorized[80];

        bool is_conditional;
        bool is_adr;
        bool is_ldrvalue;
        bool is_branch;
        bool is_exchange;
        bool is_endflow;
        bool is_coprocessor;

        bool has_target;
        bool needs_target_data;
        unsigned int target_address;
        int target_size;

        unsigned int reg_used;
        unsigned int reg_changed;

} instruction;

instruction * disarm(unsigned int addr, unsigned int ins, unsigned int state[18]);
instruction * disarm2(unsigned int addr, unsigned int ins, unsigned int state[18], unsigned int target);
instruction * disthumb(unsigned int addr, unsigned int ins, unsigned int state[18]);
instruction * disthumb2(unsigned int addr, unsigned int ins, unsigned int state[18], unsigned int target);
int insclass_arm(unsigned int ins);
int insclass_thumb(unsigned int ins);

#endif
