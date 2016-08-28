//---------------------------------------------------------------------------

#ifndef AssemblerH
#define AssemblerH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
class TAssemblerForm : public TForm
{
__published:	// IDE-managed Components
        TEdit *EditLine;
        TStaticText *Status;
        TButton *Button1;
        TButton *Button2;
private:	// User declarations
public:		// User declarations
        __fastcall TAssemblerForm(TComponent* Owner);
        int Invoke(unsigned int addr, char *ctext);
};
//---------------------------------------------------------------------------
extern PACKAGE TAssemblerForm *AssemblerForm;
//---------------------------------------------------------------------------
#endif
