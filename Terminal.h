//---------------------------------------------------------------------------

#ifndef TerminalH
#define TerminalH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <ComCtrls.hpp>
#include <Menus.hpp>
//---------------------------------------------------------------------------
class TTerminalForm : public TForm
{
__published:	// IDE-managed Components
        TTimer *ReadTimer;
        TRichEdit *Win;
        TPopupMenu *PopupMenu1;
        TMenuItem *Copy1;
        TMenuItem *Paste1;
        TMenuItem *N1;
        TMenuItem *ParseSTKmessage1;
        TMenuItem *Clear1;
        void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);
        void __fastcall WinKeyPress(TObject *Sender, char &Key);
        void __fastcall ReadTimerTimer(TObject *Sender);
        void __fastcall Copy1Click(TObject *Sender);
        void __fastcall Paste1Click(TObject *Sender);
        void __fastcall ParseSTKmessage1Click(TObject *Sender);
        void __fastcall Clear1Click(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TTerminalForm(TComponent* Owner);
        void Invoke();
};
//---------------------------------------------------------------------------
extern PACKAGE TTerminalForm *TerminalForm;
//---------------------------------------------------------------------------
#endif
