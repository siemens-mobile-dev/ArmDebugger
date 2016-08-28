#ifndef PagesH
#define PagesH

#include "DForm.h"

typedef struct asmpage_t {

        TTabSheet *sheet;
        TRichEdit *page;
        char *filename;
        char *binfile;
        unsigned int target;
        bool inuse;

} asmpage;

typedef struct rampage_t {

        TDrawGrid *page;
        TStaticText *Sticker;

        unsigned int BaseAddress;
        unsigned int Size;
        unsigned int Type;
        unsigned int InitAddress;

        unsigned int SelStart;
        unsigned int SelEnd;
        unsigned int SelRow;
        bool LMouseDown;

        unsigned int SaveAddress;
        unsigned int SaveLines;

        bool Monitor;
        bool InEdit;
        bool Freeze;
        char EditValue;

        unsigned char *Data;
        int DataLines;
        unsigned int a_history[256];
        unsigned int a_pos;


} rampage;

typedef struct cpupage_t {

        TDrawGrid *page;

        unsigned int BaseAddress;
        unsigned int Size;
        unsigned int Type;
        unsigned int InitAddress;

        unsigned int SelStart;
        unsigned int SelEnd;
        unsigned int SelRow;
        bool LMouseDown;

        unsigned int SaveAddress;
        unsigned int SaveLines;

        unsigned char *Data;
        int DataLines;
        bool Thumb;
        int InsWidth;
        bool Freeze;
        unsigned int target[256];
        unsigned int adrtgt[256];
        char adrdata[256][40];
        unsigned int a_history[256];
        unsigned int a_pos;

        unsigned int chash_a[64];
        char chash_t[64][40];
        int chash_pos;


} cpupage;

#define WTYPE_BYTE 0
#define WTYPE_HALFWORD 1
#define WTYPE_WORD 2
#define WTYPE_STRING 3
#define WTYPE_UNICODE 4

typedef struct watchitem_t {

        int Ref;
        unsigned int Address;
        unsigned int Offset[4];
        int Type;
        int Length;

        bool first;
        bool Valid;
        bool handled;
        unsigned int realAddress;
        unsigned char data[256];
        unsigned int s_realAddress;
        unsigned char s_data[256];

} watchitem;

typedef struct watchpage_t {

        TDrawGrid *page;
        TStaticText *Sticker;
        bool Visible;

        int count;
        struct watchitem_t *items;

        bool Monitor;

} watchpage;

extern asmpage AP[9], *APC;
extern rampage RP[2], *RPC;
extern watchpage WP;
extern cpupage CP;

void apInit();

unsigned int rpGetOffset(rampage *rp);
unsigned int rpGetAddress(rampage *rp);
void rpSetAddress(rampage *rp, unsigned int addr, bool save);
void rpFollowAddress(rampage *rps, rampage *rpd);
void rpGoto(rampage *rp, int val);
void rpHandleKeypress(rampage *RPC, char &Key);

void cHashAdd(unsigned int addr, char *text);
void cHashGet(unsigned int addr, char *text);

unsigned int cpGetAddress();
unsigned int cpGetOffset();
void cpSetAddress(unsigned int addr, bool save);
void cpFollowAddress();
void cpGoto();

int regnum(int col, int row);
void setRegisters(unsigned int r[16], unsigned int cpsr, int count, unsigned int stack[16]);
unsigned int getRegister(int num);
unsigned int getStackEntry(int num);
void editRegister(int col, int row);

void dump_reload(rampage *rp, bool unconditional);
void watches_reload();
void cpu_reload();
void reload_dumps();

void drawRPcell(rampage *rpc, int acol, int arow, TRect &rect, TGridDrawState state);
void drawCPcell(int acol, int arow, TRect &rect, TGridDrawState state);
void drawWPcell(watchpage *wpc, int acol, int arow, TRect &rect, TGridDrawState state);
void drawRegCell(int acol, int arow, TRect &rect, TGridDrawState state);

void copyRPCdata();
void pasteRPCdata();
void copyAsmText();
void copyAsmBinary();
void pasteAsmBinary();

#endif