//---------------------------------------------------------------------------

#ifndef SearchProgressWinH
#define SearchProgressWinH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TSearchProgress : public TForm
{
__published:	// IDE-managed Components
        TProgressBar *PBar;
        TLabel *Label1;
        TButton *Button1;
        TTimer *Timer1;
        void __fastcall Button1Click(TObject *Sender);
        void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);
        void __fastcall Timer1Timer(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TSearchProgress(TComponent* Owner);
        unsigned int TSearchProgress::Invoke(unsigned char *what, unsigned char *mask,
                        int len, unsigned int region_s, unsigned int region_l,
                        unsigned int start, bool tabulate);
        void TSearchProgress::InvokeAgain();
        unsigned int * TSearchProgress::getResults();
        int TSearchProgress::getResultsCount();
        void TSearchProgress::freeResults();
};
//---------------------------------------------------------------------------
extern PACKAGE TSearchProgress *SearchProgress;
//---------------------------------------------------------------------------
#endif
