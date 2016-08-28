//---------------------------------------------------------------------------

#ifndef PatchWindowH
#define PatchWindowH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Grids.hpp>
//---------------------------------------------------------------------------
class TPatchForm : public TForm
{
__published:	// IDE-managed Components
        TDrawGrid *Dump;
        TLabel *Label1;
        TButton *Button1;
        TButton *Button2;
        void __fastcall DumpDrawCell(TObject *Sender, int ACol, int ARow,
          TRect &Rect, TGridDrawState State);
        void __fastcall DumpSelectCell(TObject *Sender, int ACol, int ARow,
          bool &CanSelect);
        void __fastcall DumpKeyPress(TObject *Sender, char &Key);
private:	// User declarations
public:		// User declarations
        __fastcall TPatchForm(TComponent* Owner);
        int __fastcall Invoke(unsigned int addr);
        int __fastcall TPatchForm::Invoke(unsigned int addr, unsigned char *data, int len);


        unsigned int Address;
        unsigned int Length;
        unsigned char Data[1024];

};
//---------------------------------------------------------------------------
extern PACKAGE TPatchForm *PatchForm;
//---------------------------------------------------------------------------
#endif
