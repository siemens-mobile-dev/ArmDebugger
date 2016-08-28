//---------------------------------------------------------------------------

#ifndef FlashMapH
#define FlashMapH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Menus.hpp>
#include <Dialogs.hpp>
#include <Buttons.hpp>
//---------------------------------------------------------------------------
class TMapForm : public TForm
{
__published:	// IDE-managed Components
        TScrollBox *SB;
        TImage *Image;
        TStaticText *Info;
        TPopupMenu *PopupMenu1;
        TMenuItem *OpenasData1;
        TMenuItem *OpenasCode1;
        TMenuItem *N1;
        TMenuItem *OpenFullFlash1;
        TOpenDialog *OpenFlashDlg;
        TTimer *Timer1;
        TMenuItem *OpenSecondFullFlash1;
        TSpeedButton *SpeedButton1;
        TSpeedButton *SpeedButton2;
        TSpeedButton *SpeedButton3;
        TPanel *Legend;
        TShape *Shape1;
        TLabel *Label1;
        TLabel *Label2;
        TLabel *Label3;
        TLabel *Label4;
        TLabel *Label5;
        TLabel *Label6;
        TLabel *Label7;
        TShape *Shape2;
        TShape *Shape3;
        TShape *Shape4;
        TShape *Shape5;
        TShape *Shape6;
        TShape *Shape7;
        TShape *Shape8;
        TLabel *Label8;
        TShape *Shape9;
        TLabel *Label9;
        TShape *Shape10;
        TLabel *Label10;
        void __fastcall ImageMouseMove(TObject *Sender, TShiftState Shift,
          int X, int Y);
        void __fastcall Timer1Timer(TObject *Sender);
        void __fastcall OpenFullFlash1Click(TObject *Sender);
        void __fastcall OpenSecondFullFlash1Click(TObject *Sender);
        void __fastcall ImageMouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
        void __fastcall ImageDblClick(TObject *Sender);
        void __fastcall Button1Click(TObject *Sender);
        void __fastcall Button2Click(TObject *Sender);
        void __fastcall SpeedButton3Click(TObject *Sender);
        void __fastcall LegendMouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
        void __fastcall SBMouseWheelDown(TObject *Sender,
          TShiftState Shift, TPoint &MousePos, bool &Handled);
        void __fastcall SBMouseWheelUp(TObject *Sender, TShiftState Shift,
          TPoint &MousePos, bool &Handled);
private:	// User declarations
public:		// User declarations
        __fastcall TMapForm(TComponent* Owner);
        void Invoke();
};
//---------------------------------------------------------------------------
extern PACKAGE TMapForm *MapForm;
//---------------------------------------------------------------------------
#endif
