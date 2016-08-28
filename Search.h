//---------------------------------------------------------------------------

#ifndef SearchH
#define SearchH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TSearchForm : public TForm
{
__published:	// IDE-managed Components
        TEdit *What;
        TButton *Button1;
        TButton *Button2;
        TCheckBox *cbCase;
        TRadioButton *rbForward;
        TRadioButton *rbBackward;
        TBevel *Bevel1;
        void __fastcall NextSearch(TObject *Sender);
        void __fastcall Button2Click(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TSearchForm(TComponent* Owner);
        void __fastcall Open();
};
//---------------------------------------------------------------------------
extern PACKAGE TSearchForm *SearchForm;
//---------------------------------------------------------------------------
#endif
