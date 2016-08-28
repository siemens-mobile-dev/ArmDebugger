//---------------------------------------------------------------------------

#ifndef CallAddressH
#define CallAddressH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>

typedef struct _callrec {

        unsigned int addr;
        bool thumb;
        unsigned int r[8];

} callrec;

//---------------------------------------------------------------------------
class TCallForm : public TForm
{
__published:	// IDE-managed Components
        TEdit *eR0;
        TEdit *eR1;
        TEdit *eR2;
        TEdit *eR3;
        TLabel *Label1;
        TLabel *Label2;
        TLabel *Label3;
        TLabel *Label4;
        TButton *bCall;
        TLabel *Label9;
        TEdit *eAddr;
        TCheckBox *cbThumb;
        TListBox *LastCalled;
        TLabel *Label10;
        TShape *Shape1;
        TShape *Shape3;
        TEdit *eRes;
        TShape *Shape4;
        TLabel *Label11;
        TEdit *eSP0;
        TEdit *eSP4;
        TShape *Shape5;
        TLabel *Label12;
        TLabel *Label13;
        void __fastcall bCallClick(TObject *Sender);
        void __fastcall LastCalledData(TWinControl *Control, int Index,
          AnsiString &Data);
        void __fastcall LastCalledClick(TObject *Sender);
        void __fastcall LastCalledDblClick(TObject *Sender);
        void __fastcall LastCalledMouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
        void __fastcall FormKeyPress(TObject *Sender, char &Key);
private:	// User declarations
public:		// User declarations
        __fastcall TCallForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TCallForm *CallForm;
//---------------------------------------------------------------------------
#endif
