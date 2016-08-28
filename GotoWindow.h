//---------------------------------------------------------------------------

#ifndef GotoWindowH
#define GotoWindowH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
class TGotoForm : public TForm
{
__published:	// IDE-managed Components
        TButton *Button1;
        TButton *Button2;
        TComboBox *AddressEdit;
        void __fastcall FormShow(TObject *Sender);
        void __fastcall FormHide(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TGotoForm(TComponent* Owner);
        bool Invoke(char *title, char *buf, unsigned int *res);
};
//---------------------------------------------------------------------------
extern PACKAGE TGotoForm *GotoForm;
extern char **AdrSaveList;
//---------------------------------------------------------------------------
#endif
