//---------------------------------------------------------------------------

#ifndef NewWatchH
#define NewWatchH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TNewWatchForm : public TForm
{
__published:	// IDE-managed Components
        TComboBox *cbRef;
        TEdit *eAddress;
        TLabel *Label1;
        TEdit *eOff1;
        TLabel *Label2;
        TLabel *lOff1;
        TEdit *eOff2;
        TLabel *lOff2;
        TEdit *eOff3;
        TLabel *lOff3;
        TEdit *eOff4;
        TLabel *lOff4;
        TButton *bOk;
        TButton *bCancel;
        TEdit *eLength;
        TLabel *Label8;
        TRadioGroup *rType;
        TBevel *Bevel1;
        void __fastcall cbRefChange(TObject *Sender);
        void __fastcall rTypeClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TNewWatchForm(TComponent* Owner);
        void Invoke(watchitem *wi);
        void ui_sync();
        bool validate();
};
//---------------------------------------------------------------------------
extern PACKAGE TNewWatchForm *NewWatchForm;
//---------------------------------------------------------------------------
#endif
