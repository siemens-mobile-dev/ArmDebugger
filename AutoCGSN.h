//---------------------------------------------------------------------------

#ifndef AutoCGSNH
#define AutoCGSNH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Buttons.hpp>
#include <Dialogs.hpp>
//---------------------------------------------------------------------------
class TAutoCGSNform : public TForm
{
__published:	// IDE-managed Components
        TEdit *eFile;
        TLabel *Label1;
        TButton *bSearch;
        TLabel *Label2;
        TEdit *eBase;
        TBitBtn *bGenerate;
        TOpenDialog *FlashOpenDlg;
        TSaveDialog *SaveVkpDlg;
        TEdit *ePA;
        TLabel *Label3;
        void __fastcall bSearchClick(TObject *Sender);
        void __fastcall bGenerateClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TAutoCGSNform(TComponent* Owner);
        void Invoke();
};
//---------------------------------------------------------------------------
extern PACKAGE TAutoCGSNform *AutoCGSNform;
//---------------------------------------------------------------------------
#endif
