//---------------------------------------------------------------------------

#ifndef SnappointsH
#define SnappointsH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Grids.hpp>
#include <Menus.hpp>
//---------------------------------------------------------------------------
class TSnapsForm : public TForm
{
__published:	// IDE-managed Components
        TStringGrid *SnapGrid;
        TPopupMenu *PopupMenu1;
        TMenuItem *Goto1;
        TMenuItem *Edit1;
        TMenuItem *Remove1;
        TMenuItem *Removeall1;
        TMenuItem *close1;
        void __fastcall Goto1Click(TObject *Sender);
        void __fastcall Edit1Click(TObject *Sender);
        void __fastcall Remove1Click(TObject *Sender);
        void __fastcall Removeall1Click(TObject *Sender);
        void __fastcall close1Click(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TSnapsForm(TComponent* Owner);
        void Invoke();
        void updateSnaps();
};
//---------------------------------------------------------------------------
extern PACKAGE TSnapsForm *SnapsForm;
//---------------------------------------------------------------------------
#endif
