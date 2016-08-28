//---------------------------------------------------------------------------

#ifndef VkpH
#define VkpH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Buttons.hpp>
#include <Dialogs.hpp>
//---------------------------------------------------------------------------
class TVkpForm : public TForm
{
__published:	// IDE-managed Components
        TMemo *VkpText;
        TButton *Button1;
        TButton *Button2;
        TSpeedButton *SpeedButton1;
        TSpeedButton *SpeedButton2;
        TSpeedButton *SpeedButton3;
        TOpenDialog *OpenVkpDlg;
        void __fastcall SpeedButton3Click(TObject *Sender);
        void __fastcall SpeedButton1Click(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TVkpForm(TComponent* Owner);
        int Invoke(int line);
};
//---------------------------------------------------------------------------
extern PACKAGE TVkpForm *VkpForm;
//---------------------------------------------------------------------------
#endif
