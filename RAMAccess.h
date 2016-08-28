//---------------------------------------------------------------------------

#ifndef RAMAccessH
#define RAMAccessH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
class TAccessAnalyzer : public TForm
{
__published:	// IDE-managed Components
        TButton *bInit;
        TStaticText *StaticText1;
        TButton *bAnalyze;
        TButton *bClose;
        TLabel *State;
        TMemo *PageList2;
        void __fastcall bCloseClick(TObject *Sender);
        void __fastcall bInitClick(TObject *Sender);
        void __fastcall bAnalyzeClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TAccessAnalyzer(TComponent* Owner);
        void __fastcall Invoke();
};
//---------------------------------------------------------------------------
extern PACKAGE TAccessAnalyzer *AccessAnalyzer;
//---------------------------------------------------------------------------
#endif
