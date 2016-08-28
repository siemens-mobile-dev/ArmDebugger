//---------------------------------------------------------------------------

#ifndef CalculatorH
#define CalculatorH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Mask.hpp>
#include <ExtCtrls.hpp>
#include <Menus.hpp>
#include <Buttons.hpp>
//---------------------------------------------------------------------------
class TCalcForm : public TForm
{
__published:	// IDE-managed Components
        TEdit *eDec;
        TEdit *eHex;
        TLabel *Label1;
        TLabel *Label2;
        TLabel *Label3;
        TEdit *eBin;
        TLabel *lSign;
        TPopupMenu *PopupMenu1;
        TMenuItem *Copy1;
        TMenuItem *Clear1;
        TMenuItem *mPlus;
        TMenuItem *mMinus;
        TMenuItem *mMult;
        TMenuItem *mDiv;
        TMenuItem *mEqual;
        TMenuItem *mAnd;
        TMenuItem *mOr;
        TMenuItem *mXor;
        TMenuItem *mNot;
        TMenuItem *mLSH;
        TMenuItem *mRSH;
        TLabel *Label6;
        TLabel *Label4;
        TLabel *Label5;
        TLabel *Label7;
        TLabel *Label8;
        TLabel *Label9;
        TLabel *Label10;
        TMenuItem *mPlus2;
        TMenuItem *mMinus2;
        TMenuItem *mEqual2;
        TSpeedButton *SpeedButton1;
        TSpeedButton *SpeedButton2;
        TSpeedButton *SpeedButton3;
        TSpeedButton *SpeedButton4;
        TSpeedButton *SpeedButton5;
        TSpeedButton *SpeedButton6;
        TSpeedButton *SpeedButton7;
        TSpeedButton *SpeedButton8;
        TSpeedButton *SpeedButton9;
        TSpeedButton *SpeedButton10;
        TMenuItem *Paste1;
        TMenuItem *mDiv2;
        TMenuItem *mMul2;
        TSpeedButton *SpeedButton11;
        TSpeedButton *SpeedButton12;
        TMenuItem *N1;
        TMenuItem *Stayontop1;
        void __fastcall eDecChange(TObject *Sender);
        void __fastcall eHexChange(TObject *Sender);
        void __fastcall eBinChange(TObject *Sender);
        void __fastcall Clear1Click(TObject *Sender);
        void __fastcall mPlusClick(TObject *Sender);
        void __fastcall mMinusClick(TObject *Sender);
        void __fastcall mEqualClick(TObject *Sender);
        void __fastcall mDivClick(TObject *Sender);
        void __fastcall mMultClick(TObject *Sender);
        void __fastcall Copy1Click(TObject *Sender);
        void __fastcall mAndClick(TObject *Sender);
        void __fastcall mOrClick(TObject *Sender);
        void __fastcall mXorClick(TObject *Sender);
        void __fastcall mNotClick(TObject *Sender);
        void __fastcall Paste1Click(TObject *Sender);
        void __fastcall mLSHClick(TObject *Sender);
        void __fastcall mRSHClick(TObject *Sender);
        void __fastcall Stayontop1Click(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TCalcForm(TComponent* Owner);
        void Invoke();
};
//---------------------------------------------------------------------------
extern PACKAGE TCalcForm *CalcForm;
//---------------------------------------------------------------------------
#endif
