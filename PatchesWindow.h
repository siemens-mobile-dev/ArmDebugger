//---------------------------------------------------------------------------

#ifndef PatchesWindowH
#define PatchesWindowH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Grids.hpp>
#include <Menus.hpp>
#include <Dialogs.hpp>
//---------------------------------------------------------------------------
class TPatchesForm : public TForm
{
__published:	// IDE-managed Components
        TStringGrid *List;
        TPopupMenu *PatchesPopup;
        TMenuItem *Edit1;
        TMenuItem *Undo1;
        TMenuItem *Saveasvkp1;
        TMenuItem *Saveallasvkp1;
        TMenuItem *Goto1;
        TMenuItem *Close1;
        TSaveDialog *SaveVkpDlg;
        TMenuItem *Decreaseorder1;
        TMenuItem *Increaseorder1;
        TMenuItem *Enable1;
        TMenuItem *Disable1;
        TMenuItem *EnableAll1;
        TMenuItem *DisableAll1;
        void __fastcall Close1Click(TObject *Sender);
        void __fastcall Goto1Click(TObject *Sender);
        void __fastcall Edit1Click(TObject *Sender);
        void __fastcall Undo1Click(TObject *Sender);
        void __fastcall Saveasvkp1Click(TObject *Sender);
        void __fastcall ListDblClick(TObject *Sender);
        void __fastcall Saveallasvkp1Click(TObject *Sender);
        void __fastcall Decreaseorder1Click(TObject *Sender);
        void __fastcall Increaseorder1Click(TObject *Sender);
        void __fastcall Enable1Click(TObject *Sender);
        void __fastcall Disable1Click(TObject *Sender);
        void __fastcall EnableAll1Click(TObject *Sender);
        void __fastcall DisableAll1Click(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TPatchesForm(TComponent* Owner);
        void __fastcall Invoke();
        void reloadList();


};
//---------------------------------------------------------------------------
extern PACKAGE TPatchesForm *PatchesForm;
//---------------------------------------------------------------------------
#endif
