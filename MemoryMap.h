//---------------------------------------------------------------------------

#ifndef MemoryMapH
#define MemoryMapH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Grids.hpp>
//---------------------------------------------------------------------------
class TMMapView : public TForm
{
__published:	// IDE-managed Components
        TPanel *pReading;
        TTimer *Timer1;
        TPanel *Panel1;
        TStringGrid *L1grid;
        TSplitter *Splitter1;
        TPanel *Panel2;
        TStringGrid *L2grid;
        TTimer *Timer2;
        void __fastcall Timer1Timer(TObject *Sender);
        void __fastcall L1gridSelectCell(TObject *Sender, int ACol,
          int ARow, bool &CanSelect);
        void __fastcall Timer2Timer(TObject *Sender);
        void __fastcall L1gridClick(TObject *Sender);
        void __fastcall L2gridClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TMMapView(TComponent* Owner);
        void Invoke();
};
//---------------------------------------------------------------------------
extern PACKAGE TMMapView *MMapView;
//---------------------------------------------------------------------------
#endif
