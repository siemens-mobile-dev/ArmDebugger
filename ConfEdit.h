//---------------------------------------------------------------------------

#ifndef ConfEditH
#define ConfEditH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <Dialogs.hpp>
#include <Buttons.hpp>
//---------------------------------------------------------------------------
class TConfigForm : public TForm
{
__published:	// IDE-managed Components
        TButton *bOk;
        TButton *bCancel;
        TFontDialog *EditorFontDlg;
        TGroupBox *GroupBox1;
        TComboBox *cbMethod;
        TComboBox *cbSpeed;
        TLabel *Label2;
        TLabel *Label1;
        TEdit *eMonIntv;
        TLabel *Label4;
        TLabel *Label3;
        TGroupBox *GroupBox2;
        TButton *bEditFont;
        TLabel *Label5;
        TLabel *Label6;
        TButton *bDbgFont;
        TGroupBox *GroupBox3;
        TCheckBox *ckEvents;
        TCheckBox *ckDebug;
        TCheckBox *ckWire;
        TCheckBox *ckComm;
        TGroupBox *GroupBox4;
        TEdit *eArmPath;
        TLabel *Label7;
        TEdit *eAsmArgs;
        TLabel *Label8;
        TEdit *eCompArgs;
        TLabel *Label9;
        TFontDialog *DebuggerFontDlg;
        TEdit *eLinkArgs;
        TLabel *Label10;
        TEdit *eExecTm;
        TLabel *Label11;
        TLabel *Label12;
        void __fastcall bEditFontClick(TObject *Sender);
        void __fastcall bDbgFontClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TConfigForm(TComponent* Owner);
        void Invoke();
};
//---------------------------------------------------------------------------
extern PACKAGE TConfigForm *ConfigForm;
//---------------------------------------------------------------------------
#endif
