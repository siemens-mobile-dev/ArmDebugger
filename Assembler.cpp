//---------------------------------------------------------------------------

#include <vcl.h>
#include "Pages.h"
#include "Disarm.h"
#include "Debug.h"
#pragma hdrstop

#include "Assembler.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TAssemblerForm *AssemblerForm;
//---------------------------------------------------------------------------
__fastcall TAssemblerForm::TAssemblerForm(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------
int TAssemblerForm::Invoke(unsigned int addr, char *ctext) {

        unsigned int buf[2];
        char tbuf[64];
        instruction *ins;
        unsigned int r[18];
        unsigned int i;

        if (ctext == NULL) {
                if (! readMemory(addr, (unsigned char *)buf, 4)) return mbCancel;

                ins = (! CP.Thumb) ? disarm(addr, buf[0], r) : disthumb(addr, buf[0], r);
                strcpy(tbuf, ins->resolved);
                for (i=0; i<strlen(tbuf); i++) if (tbuf[i] == '\t') tbuf[i] = ' ';
                EditLine->Text = tbuf;
        } else {
                EditLine->Text = ctext;
        }

        ActiveControl = EditLine;
        EditLine->SelectAll();
        return ShowModal();

}
