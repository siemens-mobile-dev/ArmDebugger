//---------------------------------------------------------------------------

#ifndef XrefsH
#define XrefsH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <Grids.hpp>
#include <ExtCtrls.hpp>
#include <Menus.hpp>
//---------------------------------------------------------------------------
class TXrefForm : public TForm
{
__published:	// IDE-managed Components
        TStringGrid *Grid;
        TStatusBar *StatusBar1;
        TTimer *XTimer;
        TPopupMenu *PopupMenu1;
        TMenuItem *Goto1;
        void __fastcall XTimerTimer(TObject *Sender);
        void __fastcall FormKeyPress(TObject *Sender, char &Key);
        void __fastcall Goto1Click(TObject *Sender);
        void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
private:	// User declarations
public:		// User declarations
        __fastcall TXrefForm(TComponent* Owner);
        bool Invoke(unsigned int t);
};
//---------------------------------------------------------------------------
extern PACKAGE TXrefForm *XrefForm;
//---------------------------------------------------------------------------
#endif
