//---------------------------------------------------------------------------

#ifndef NucleusH
#define NucleusH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <Grids.hpp>
#include <ExtCtrls.hpp>

typedef struct signature_s {

        char *name;
        unsigned int address;
        char *sig;
        int offset;

} signature;

//---------------------------------------------------------------------------
class TNuForm : public TForm
{
__published:	// IDE-managed Components
        TPageControl *NuPages;
        TTabSheet *InfoPage;
        TTabSheet *ProcessPage;
        TTabSheet *HisrPage;
        TTabSheet *DynaPage;
        TTabSheet *PartPage;
        TTabSheet *TabSheet7;
        TTabSheet *TabSheet10;
        TStringGrid *sgSymbols;
        TLabel *Label1;
        TStaticText *info1;
        TTimer *FirstTimer;
        TTimer *UpdateTimer;
        TStringGrid *sgProc;
        TLabel *infoProc;
        TStringGrid *sgHisr;
        TLabel *infoHisr;
        TStringGrid *sgDyna;
        TLabel *infoDyna;
        TStringGrid *sgPart;
        TLabel *infoPart;
        void __fastcall FirstTimerTimer(TObject *Sender);
        void __fastcall UpdateTimerTimer(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TNuForm(TComponent* Owner);
        void Invoke();
};
//---------------------------------------------------------------------------
extern PACKAGE TNuForm *NuForm;
//---------------------------------------------------------------------------
#endif
