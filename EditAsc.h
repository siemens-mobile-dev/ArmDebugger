//---------------------------------------------------------------------------

#ifndef EditAscH
#define EditAscH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
class TEditAscForm : public TForm
{
__published:	// IDE-managed Components
        TButton *bOk;
        TButton *bCancel;
        TEdit *eLine;
        void __fastcall eLineChange(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TEditAscForm(TComponent* Owner);
        char *Invoke(char *title, char *buf, bool isNumber, int maxlen);
};
//---------------------------------------------------------------------------
extern PACKAGE TEditAscForm *EditAscForm;
//---------------------------------------------------------------------------
#endif
