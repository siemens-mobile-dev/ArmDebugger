//---------------------------------------------------------------------------

#ifndef WatchpointH
#define WatchpointH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TWptForm : public TForm
{
__published:	// IDE-managed Components
        TRichEdit *RtEdit;
        TButton *bOk;
        TButton *bCancel;
        TBevel *Bevel1;
        TButton *Button1;
        TMemo *Memo4;
        TLabel *Label1;
        TLabel *Label2;
        TEdit *eCond;
        void __fastcall RtEditKeyPress(TObject *Sender, char &Key);
private:	// User declarations
public:		// User declarations
        __fastcall TWptForm(TComponent* Owner);
        bool Invoke(unsigned int addr, bool is_thumb, int type);
};
//---------------------------------------------------------------------------
extern PACKAGE TWptForm *WptForm;
//---------------------------------------------------------------------------
#endif
