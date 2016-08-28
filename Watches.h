//---------------------------------------------------------------------------

#ifndef WatchesH
#define WatchesH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Grids.hpp>
#include <Menus.hpp>
//---------------------------------------------------------------------------
class TWatchesForm : public TForm
{
__published:	// IDE-managed Components
        TPopupMenu *PopupMenu1;
        TMenuItem *Edit1;
        TMenuItem *Newwatch1;
        TMenuItem *Delete1;
        TMenuItem *N1;
        TMenuItem *Refresh1;
        TMenuItem *Monitor1;
        TDrawGrid *List;
        TMenuItem *View1;
        TMenuItem *ChangeWatch1;
        TMenuItem *N2;
        TMenuItem *Stayontop1;
        TMenuItem *Hide1;
        void __fastcall FormCreate(TObject *Sender);
        void __fastcall FormHide(TObject *Sender);
        void __fastcall FormShow(TObject *Sender);
        void __fastcall Refresh1Click(TObject *Sender);
        void __fastcall Monitor1Click(TObject *Sender);
        void __fastcall View1Click(TObject *Sender);
        void __fastcall Hide1Click(TObject *Sender);
        void __fastcall Stayontop1Click(TObject *Sender);
        void __fastcall Newwatch1Click(TObject *Sender);
        void __fastcall ListDrawCell(TObject *Sender, int ACol, int ARow,
          TRect &Rect, TGridDrawState State);
        void __fastcall ChangeWatch1Click(TObject *Sender);
        void __fastcall Delete1Click(TObject *Sender);
        void __fastcall Edit1Click(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TWatchesForm(TComponent* Owner);
        void Invoke();

};
//---------------------------------------------------------------------------
extern PACKAGE TWatchesForm *WatchesForm;
//---------------------------------------------------------------------------
#endif
