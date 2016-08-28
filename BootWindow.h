//---------------------------------------------------------------------------

#ifndef BootWindowH
#define BootWindowH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Grids.hpp>
#include <Dialogs.hpp>
#include <ComCtrls.hpp>
//---------------------------------------------------------------------------
class TBootForm : public TForm
{
__published:	// IDE-managed Components
        TButton *bClose;
        TTimer *Timer1;
        TTimer *Timer2;
        TButton *bSave;
        TSaveDialog *BootSave;
        TStatusBar *StatusBar1;
        TButton *bReset;
        TEdit *BootCmd0;
        TButton *ExecCmd0;
        TEdit *BootCmd1;
        TButton *ExecCmd1;
        TEdit *BootCmd2;
        TButton *ExecCmd2;
        TEdit *BootCmd3;
        TButton *ExecCmd3;
        TEdit *BootCmd4;
        TButton *ExecCmd4;
        TLabel *Label1;
        TMemo *Memo1;
        TDrawGrid *Answer;
        void __fastcall bCloseClick(TObject *Sender);
        void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);
        void __fastcall Timer1Timer(TObject *Sender);
        void __fastcall Timer2Timer(TObject *Sender);
        void __fastcall bSaveClick(TObject *Sender);
        void __fastcall bResetClick(TObject *Sender);
        void __fastcall AnswerDrawCell(TObject *Sender, int ACol, int ARow,
          TRect &Rect, TGridDrawState State);
        void __fastcall ExecCmd0Click(TObject *Sender);
        void __fastcall BootCmd0KeyPress(TObject *Sender, char &Key);
private:	// User declarations
public:		// User declarations
        __fastcall TBootForm(TComponent* Owner);

        void Invoke(unsigned char *buf, int len);

};
//---------------------------------------------------------------------------
typedef struct dataline_s {

        char direction, rsv1, rsv2, rsv3;
        unsigned char data[16];
        int count;

} dataline;

extern PACKAGE TBootForm *BootForm;

extern char *BootCmd[5];
//---------------------------------------------------------------------------
#endif
