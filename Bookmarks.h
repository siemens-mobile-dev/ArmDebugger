//---------------------------------------------------------------------------

#ifndef BookmarksH
#define BookmarksH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Grids.hpp>
#include <Menus.hpp>
#include <Dialogs.hpp>
#include <ImgList.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
#define ST_SOURCE 1
#define ST_BINARY 2
#define ST_TSEARCH 3
#define ST_HSEARCH 4
#define ST_CODE 5
#define ST_DATA 6
#define ST_PATCH 7

typedef struct sb_entry_t {

        int type;
        char *name;
        unsigned int address;
        char *text;
        unsigned char *data;
        int len;

} sb_entry;

class TBkForm : public TForm
{
__published:	// IDE-managed Components
        TDrawGrid *Grid;
        TPopupMenu *Menu;
        TMenuItem *Delete1;
        TMenuItem *Open1;
        TMenuItem *N1;
        TMenuItem *NewSource1;
        TMenuItem *NewBinary1;
        TMenuItem *NewCodepoint1;
        TMenuItem *NewDatapoint1;
        TMenuItem *NewPatch1;
        TMenuItem *NewTextSearch1;
        TMenuItem *N2;
        TMenuItem *Stayontop1;
        TOpenDialog *OpenSrcDlg;
        TOpenDialog *OpenBinDlg;
        TImageList *TypeImages;
        TTimer *Timer1;
        TMenuItem *Open21;
        TMenuItem *NewSearchhex1;
        void __fastcall Stayontop1Click(TObject *Sender);
        void __fastcall NewSource1Click(TObject *Sender);
        void __fastcall NewBinary1Click(TObject *Sender);
        void __fastcall NewCodepoint1Click(TObject *Sender);
        void __fastcall NewDatapoint1Click(TObject *Sender);
        void __fastcall GridDrawCell(TObject *Sender, int ACol, int ARow,
          TRect &Rect, TGridDrawState State);
        void __fastcall FormResize(TObject *Sender);
        void __fastcall MenuPopup(TObject *Sender);
        void __fastcall Delete1Click(TObject *Sender);
        void __fastcall Grid1DblClick(TObject *Sender);
        void __fastcall NewTextSearch1Click(TObject *Sender);
        void __fastcall NewSearchhex1Click(TObject *Sender);
        void __fastcall GridMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
        void __fastcall Open21Click(TObject *Sender);
        void __fastcall Open1Click(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TBkForm(TComponent* Owner);
};

extern sb_entry *sb_list;
extern int sb_count;

void emptySidebar();
void addSideEntryFast(int type, unsigned int addr, char *text, unsigned char *data, int len);
void addSideEntry(int type, unsigned int addr, char *text, unsigned char *data, int len);
void updateSidebar();

//---------------------------------------------------------------------------
extern PACKAGE TBkForm *BkForm;
//---------------------------------------------------------------------------
#endif
