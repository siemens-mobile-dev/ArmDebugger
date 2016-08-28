//---------------------------------------------------------------------------

#ifndef SaveProgressWinH
#define SaveProgressWinH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include <stdio.h>
//---------------------------------------------------------------------------
class TSaveProgress : public TForm
{
__published:	// IDE-managed Components
        TProgressBar *PBar;
        TButton *Button1;
        TLabel *Label;
        TTimer *Timer1;
        void __fastcall Button1Click(TObject *Sender);
        void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);
        void __fastcall Timer1Timer(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TSaveProgress(TComponent* Owner);

        void initiate(FILE *F, int addr, int len);
};
//---------------------------------------------------------------------------
extern PACKAGE TSaveProgress *SaveProgress;
//---------------------------------------------------------------------------
#endif
