//---------------------------------------------------------------------------

#ifndef SearchResultsH
#define SearchResultsH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Grids.hpp>
#include <ComCtrls.hpp>
#include <Menus.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TSearchResForm : public TForm
{
__published:	// IDE-managed Components
        TDrawGrid *Grid;
        TStatusBar *StatusBar1;
        TPopupMenu *PopupMenu1;
        TMenuItem *View1;
        TMenuItem *N1;
        TMenuItem *RestartSearch1;
        TMenuItem *Refresh1;
        TPanel *Panel1;
        TTimer *Timer1;
        TMenuItem *ViewasCode1;
        TMenuItem *Removefromlist1;
        void __fastcall Timer1Timer(TObject *Sender);
        void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);
        void __fastcall GridDrawCell(TObject *Sender, int acol, int arow,
          TRect &rect, TGridDrawState state);
        void __fastcall Refresh1Click(TObject *Sender);
        void __fastcall View1Click(TObject *Sender);
        void __fastcall ViewasCode1Click(TObject *Sender);
        void __fastcall GridDblClick(TObject *Sender);
        void __fastcall RestartSearch1Click(TObject *Sender);
        void __fastcall Removefromlist1Click(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TSearchResForm(TComponent* Owner);
        void TSearchResForm::Invoke(unsigned int r_s, unsigned int r_l,
                                unsigned char *what, unsigned char *mask,
                                int size,
                                unsigned int *alist, int acount,
                                int ishex);
        void TSearchResForm::Dismiss();
        void TSearchResForm::ReloadMemory();

};
//---------------------------------------------------------------------------
extern PACKAGE TSearchResForm *SearchResForm;
//---------------------------------------------------------------------------

typedef struct resdata_s {
        unsigned int addr;
        unsigned char *data;
} resdata;

#endif
