//---------------------------------------------------------------------------

#ifndef SaveMemoryH
#define SaveMemoryH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Dialogs.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TSaveMem : public TForm
{
__published:	// IDE-managed Components
        TEdit *eAddress;
        TEdit *eLength;
        TLabel *Label1;
        TLabel *Label2;
        TBevel *Bevel1;
        TButton *Button1;
        TButton *Button2;
        TEdit *eFilename;
        TLabel *Label3;
        TButton *Button3;
        TSaveDialog *SaveMemoryDlg;
        void __fastcall Button3Click(TObject *Sender);
        void __fastcall FormShow(TObject *Sender);
        void __fastcall eFilenameChange(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TSaveMem(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TSaveMem *SaveMem;
//---------------------------------------------------------------------------
#endif
