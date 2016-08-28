//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
//---------------------------------------------------------------------------
USEFORM("DForm.cpp", MainForm);
USEFORM("SaveMemory.cpp", SaveMem);
USEFORM("SaveProgressWin.cpp", SaveProgress);
USEFORM("About.cpp", AboutForm);
USEFORM("GotoWindow.cpp", GotoForm);
USEFORM("BootWindow.cpp", BootForm);
USEFORM("Search.cpp", SearchForm);
USEFORM("CallAddress.cpp", CallForm);
USEFORM("PatchWindow.cpp", PatchForm);
USEFORM("PatchesWindow.cpp", PatchesForm);
USEFORM("CP15Window.cpp", CP15Form);
USEFORM("RAMAccess.cpp", AccessAnalyzer);
USEFORM("AutoCGSN.cpp", AutoCGSNform);
USEFORM("NewWatch.cpp", NewWatchForm);
USEFORM("EditAsc.cpp", EditAscForm);
USEFORM("Assembler.cpp", AssemblerForm);
USEFORM("ConfEdit.cpp", ConfigForm);
USEFORM("MemoryMap.cpp", MMapView);
USEFORM("Calculator.cpp", CalcForm);
USEFORM("Xrefs.cpp", XrefForm);
USEFORM("Bookmarks.cpp", BkForm);
USEFORM("Watchpoint.cpp", WptForm);
USEFORM("Snappoints.cpp", SnapsForm);
USEFORM("Nucleus.cpp", NuForm);
USEFORM("FlashMap.cpp", MapForm);
USEFORM("SearchResults.cpp", SearchResForm);
USEFORM("SearchProgressWin.cpp", SearchProgress);
USEFORM("Terminal.cpp", TerminalForm);
USEFORM("Vkp.cpp", VkpForm);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
        try
        {
                 Application->Initialize();
                 Application->CreateForm(__classid(TMainForm), &MainForm);
                 Application->CreateForm(__classid(TSaveMem), &SaveMem);
                 Application->CreateForm(__classid(TSaveProgress), &SaveProgress);
                 Application->CreateForm(__classid(TAboutForm), &AboutForm);
                 Application->CreateForm(__classid(TGotoForm), &GotoForm);
                 Application->CreateForm(__classid(TBootForm), &BootForm);
                 Application->CreateForm(__classid(TSearchForm), &SearchForm);
                 Application->CreateForm(__classid(TCallForm), &CallForm);
                 Application->CreateForm(__classid(TCP15Form), &CP15Form);
                 Application->CreateForm(__classid(TPatchForm), &PatchForm);
                 Application->CreateForm(__classid(TPatchesForm), &PatchesForm);
                 Application->CreateForm(__classid(TAccessAnalyzer), &AccessAnalyzer);
                 Application->CreateForm(__classid(TAutoCGSNform), &AutoCGSNform);
                 Application->CreateForm(__classid(TNewWatchForm), &NewWatchForm);
                 Application->CreateForm(__classid(TEditAscForm), &EditAscForm);
                 Application->CreateForm(__classid(TAssemblerForm), &AssemblerForm);
                 Application->CreateForm(__classid(TConfigForm), &ConfigForm);
                 Application->CreateForm(__classid(TMMapView), &MMapView);
                 Application->CreateForm(__classid(TCalcForm), &CalcForm);
                 Application->CreateForm(__classid(TXrefForm), &XrefForm);
                 Application->CreateForm(__classid(TBkForm), &BkForm);
                 Application->CreateForm(__classid(TWptForm), &WptForm);
                 Application->CreateForm(__classid(TSnapsForm), &SnapsForm);
                 Application->CreateForm(__classid(TNuForm), &NuForm);
                 Application->CreateForm(__classid(TMapForm), &MapForm);
                 Application->CreateForm(__classid(TSearchResForm), &SearchResForm);
                 Application->CreateForm(__classid(TSearchProgress), &SearchProgress);
                 Application->CreateForm(__classid(TTerminalForm), &TerminalForm);
                 Application->CreateForm(__classid(TVkpForm), &VkpForm);
                 Application->Run();
        }
        catch (Exception &exception)
        {
                 Application->ShowException(&exception);
        }
        catch (...)
        {
                 try
                 {
                         throw Exception("");
                 }
                 catch (Exception &exception)
                 {
                         Application->ShowException(&exception);
                 }
        }
        return 0;
}
//---------------------------------------------------------------------------
