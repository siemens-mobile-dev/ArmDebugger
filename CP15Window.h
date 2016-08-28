//---------------------------------------------------------------------------

#ifndef CP15WindowH
#define CP15WindowH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Grids.hpp>
#include <Menus.hpp>
//---------------------------------------------------------------------------
class TCP15Form : public TForm
{
__published:	// IDE-managed Components
        TStringGrid *List;
        TPopupMenu *PopupMenu1;
        TMenuItem *Edit1;
        TMenuItem *Refresh1;
        void __fastcall Refresh1Click(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TCP15Form(TComponent* Owner);
        void __fastcall Invoke();
        bool __fastcall reload();

};
//---------------------------------------------------------------------------
extern PACKAGE TCP15Form *CP15Form;
//---------------------------------------------------------------------------
#endif
