#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Debug.h"
#include "Config.h"
#include "Disarm.h"
#include "EventLog.h"
#include "Wire.h"
#include "Pages.h"
#include "Bookmarks.h"
#include "comm.h"
#include "IniFile.h"
#include "Utility.h"
#include "Globals.h"

#include "asm\arm_readCP15.h"
#include "asm\arm_fillL2T.h"
#include "asm\arm_checkL2T.h"
#include "asm\arm_signature.h"
#include "asm\arm_internal.h"

static unsigned char arm_getsplr[] = {
        0x0d, 0x00, 0xa0, 0xe1, 0x0e, 0x10, 0xa0, 0xe1, 0x1e, 0xff, 0x2f, 0xe1
};

static int SessionID;
static bool after_init = false;

static struct patch **PP = NULL;
static int pp_count = 0;
static struct aname **LL = NULL;
static int ll_count = 0;
static struct aname **NN = NULL;
static int nn_count = 0;
static struct snap *SN[16];

static unsigned char *Changed = NULL;
static unsigned int ChangedSize = 0;

unsigned int BootROMaddress=0, BootROMsize=0;
unsigned int FlashAddress=0, FlashSize=0, FlashPageCount=0;
unsigned int IntRAM1address=0, IntRAM1size=0;
unsigned int IntRAM2address=0, IntRAM2size=0;
unsigned int IntRAM3address=0, IntRAM3size=0;
unsigned int ExtRAMaddress=0, ExtRAMsize=0xfffffffc, ExtRAMpageCount=1048576;
unsigned int IOaddress=0, IOsize=0;
unsigned int ShadowFlashAddress=0, ShadowRAMaddress=0;
unsigned int NucleusLow=0, NucleusHigh=0;

unsigned int mem_addr[16];
unsigned int mem_len[16];
unsigned int mem_type[16];

int PoolCount=0;
unsigned int PoolAddress[32];
int PoolPages[32];
int PoolFree=0;
unsigned int MallocAddress=0;
int MallocPages=0;

char *DeviceName;
unsigned int UseRAM;
unsigned int HeaderAddr;
unsigned int RelBitsAddr;
unsigned int SnapDataAddr;
unsigned int HolderAddr;
unsigned int InternalCodeAddr;
unsigned int LoadableCodeAddr;
unsigned int TempExecAddr;
unsigned int MallocPoolAddr;
bool SimpleBoot;

static unsigned int Header[8];

static unsigned int *RelAddrList = NULL;
static int RelCount = 0;
static unsigned char RelBits[2048];
static int RelBitsSize = 0;

unsigned int ConfigRegister;
unsigned int TransTableAddr;
unsigned int ZeroPage = NONE;
unsigned int LogPage = NONE;
static unsigned int CurrentL2T = 0;
static int CurrentL2Tpage = 0;

unsigned int WatchpointAddr = 0;
unsigned int WatchpointLen = 0;
int LastRead = 31;

int SystemState = SS_DISCONNECTED;
unsigned int CurrentAddr = NONE;
bool CurrentThumb = false;
unsigned int CurrentRs[16];
unsigned int CurrentCPSR;
unsigned int CurrentStack[16];
unsigned int Break0addr = NONE;
bool Break0thumb = false;
bool SubsR15 = false;
unsigned int SubsAddress = NONE;
bool ThumbAware = false;
bool ReloadQuery = false;

static int SnapTime = 0;

char *NuSleepSignature = "00?08de500009?e50030a0e30020a0e30210a0e3??????eb";

unsigned int bitMask(unsigned int addr) {

        unsigned int r, x, y;
        int i;

        r = addr - 1;
        for (x=0x80000000,y=0; x!=0; x>>=1,y>>=1) {
                y |= r & x;
                r |= y;
        }

        return ~r;

}

unsigned char bitShift(unsigned int addr) {

        unsigned int r = 0;

        addr -= 1;
        while (addr != 0) { r++; addr >>= 1; }
        return r;

}

unsigned int shadow(unsigned int a) {

        if (a >= ExtRAMaddress && a <= (ExtRAMaddress+ExtRAMsize-1)) {
                return a + (ShadowRAMaddress-ExtRAMaddress);
        } else if (a >= FlashAddress && a <= (FlashAddress+FlashSize-1)) {
                return a + (ShadowFlashAddress-FlashAddress);
        } else {
                return a;
        }
}

void fillRelocationAddressList() {

        RelCount = 0;
        for (int i=0; i<PoolCount; i++) {
                unsigned int a = PoolAddress[i];
                RelAddrList = (unsigned int *)realloc(RelAddrList, (RelCount+PoolPages[i]) * sizeof(unsigned int *));
                for (int j=0; j<PoolPages[i]; j++) {
                        RelAddrList[RelCount+j] = a + j*4096;
                }
                RelCount += PoolPages[i];
        }
        PoolFree = RelCount;
        VD("Pool pages: %i", RelCount);

}

// initialize state variables for a new session
void initialize(int sid) {

        SessionID = sid;
        after_init = true;

        VD("initialize {");
        emptyPatchList();
        emptyNamesList(false);
        emptyNamesList(true);
        emptySnapList();
        emptyWatchpoint();
        emptySidebar();

        // temporary values
        mem_addr[0] = 0; mem_len[0] = 0xfffffffc; mem_type[0] = M_RAM;
        VD("} initialize");

}

// check whether current session is loaded into mobile, load it
bool synchronize0() {

        unsigned int buf[4096], tt2[4096], *ip;
        unsigned char ramstate[4096], *ipb;
        char tbuf[2048], tbuf2[2048], *s;
        unsigned int a, tta, nsects, r[16], cpsr, r0, v;
        unsigned int e_addr, e_count, tt_free, mask, changes;
        int i, j, n, rs, magic, sid, css, newstate;
        memoryinfo mi;

        strcpy(tbuf, BaseDir);
        strcat(tbuf, "\\devices.ini");
        openIniFile(tbuf);

        s = exec_at("AT+CGMI");
        if (s[0] == '\0') s = "UNKNOWN";
        strcpy(tbuf, s);
        s = exec_at("AT+CGMM");
        if (s[0] == '\0') s = "UNKNOWN";
        strcat(tbuf, ":");
        strcat(tbuf, s);
        s = exec_at("AT+CGMR");
        if (s[0] == '\0') s = "UNKNOWN";
        if (isdigit(s[0]) && strlen(s) > 16) {
                for (i=0; isdigit(s[i]); i++) ;
        } else {
                for (i=0; i<16; i++) {
                        if (s[i] == '\0')break;
                        if (! isalnum(s[i])) s[i] = '_';
                }
        }
        s[i] = '\0';
        strcat(tbuf, ":");
        strcat(tbuf, s);
        VD("device=[%s]", tbuf);

        DeviceName = getString(tbuf, "Name", "");
        if (DeviceName[0] == '\0')
                return WarningMessage("No device information in database\nDevice:  [%s]\nPlease add information to devices.ini", tbuf);

        UseRAM = getInt(tbuf, "UseRAM", 0);
        VD("UseRAM=0x%08X", UseRAM);
        if (UseRAM == 0) return WarningMessage("Missing parameter in device information: UseRAM");

        BootROMaddress = getInt(tbuf, "BootROMaddress", 0);
        BootROMsize = getInt(tbuf, "BootROMsize", 0);
        FlashAddress = getInt(tbuf, "FlashAddress", 0xa0000000);
        FlashSize = getInt(tbuf, "FlashSize", 0x02000000);
        FlashPageCount = FlashSize / 4096;
        IntRAM1address = getInt(tbuf, "IntRAM1address", 0);
        IntRAM1size = getInt(tbuf, "IntRAM1size", 0);
        IntRAM2address = getInt(tbuf, "IntRAM2address", 0);
        IntRAM2size = getInt(tbuf, "IntRAM2size", 0);
        IntRAM3address = getInt(tbuf, "IntRAM3address", 0);
        IntRAM3size = getInt(tbuf, "IntRAM3size", 0);
        ExtRAMaddress = getInt(tbuf, "ExtRAMaddress", 0xa8000000);
        ExtRAMsize = getInt(tbuf, "ExtRAMsize", 0x00800000);
        ExtRAMpageCount = ExtRAMsize / 4096;
        IOaddress = getInt(tbuf, "IOaddress", 0);
        IOsize = getInt(tbuf, "IOsize", 0);
        ShadowFlashAddress = getInt(tbuf, "ShadowFlashAddress", 0x10000000);
        ShadowRAMaddress = getInt(tbuf, "ShadowRAMaddress", 0x18000000);
        NuSleepSignature = strdup(getString(tbuf, "NuSleepSignature", NuSleepSignature));

        VD("BootROMaddress=0x%08X", BootROMaddress);
        VD("BootROMsize=0x%08X", BootROMsize);
        VD("FlashAddress=0x%08X", FlashAddress);
        VD("FlashSize=0x%08X", FlashSize);
        VD("FlashPageCount=%i", FlashPageCount);
        VD("IntRAM1address=0x%08X", IntRAM1address);
        VD("IntRAM1size=0x%08X", IntRAM1size);
        VD("IntRAM2address=0x%08X", IntRAM2address);
        VD("IntRAM2size=0x%08X", IntRAM2size);
        VD("IntRAM3address=0x%08X", IntRAM3address);
        VD("IntRAM3size=0x%08X", IntRAM3size);
        VD("ExtRAMaddress=0x%08X", ExtRAMaddress);
        VD("ExtRAMsize=0x%08X", ExtRAMsize);
        VD("ExtRAMpageCount=%i", ExtRAMpageCount);
        VD("IOaddress=0x%08X", IOaddress);
        VD("IOsize=0x%08X", IOsize);
        VD("ShadowFlashAddress=0x%08X", ShadowFlashAddress);
        VD("ShadowRAMaddress=0x%08X", ShadowRAMaddress);
        VD("NuSleepSignature=%s", NuSleepSignature);

        PoolCount = 0;
        for (i=1; i<=32; i++) {
                sprintf(tbuf2, "Pool%iaddress", i);
                a = getInt(tbuf, tbuf2, 0);
                if (a == 0) continue;
                sprintf(tbuf2, "Pool%ipages", i);
                n = getInt(tbuf, tbuf2, 0);
                if (n == 0) continue;
                PoolAddress[PoolCount] = a;
                PoolPages[PoolCount] = n;
                VD("Pool%iaddress=0x%08X", i, a);
                VD("Pool%ipages=%i", i, n);
                PoolCount++;
        }

        MallocAddress = getInt(tbuf, "MallocAddress", 0);
        MallocPages = getInt(tbuf, "MallocPages", 0);

        if (MallocAddress != 0 && MallocPages != 0) {
                PoolAddress[PoolCount] = NONE;
                PoolPages[PoolCount] = MallocPages;
                PoolCount += 1;
        }

        i = 0;
        if (BootROMsize != 0) { mem_addr[i] = BootROMaddress; mem_len[i] = BootROMsize; mem_type[i] = M_BOOT; i++; }
        if (FlashSize != 0) { mem_addr[i] = FlashAddress; mem_len[i] = FlashSize; mem_type[i] = M_FLASH; i++; }
        if (IntRAM1size != 0) { mem_addr[i] = IntRAM1address; mem_len[i] = IntRAM1size; mem_type[i] = M_RAM; i++; }
        if (IntRAM2size != 0) { mem_addr[i] = IntRAM2address; mem_len[i] = IntRAM2size; mem_type[i] = M_RAM; i++; }
        if (IntRAM3size != 0) { mem_addr[i] = IntRAM3address; mem_len[i] = IntRAM3size; mem_type[i] = M_RAM; i++; }
        if (ExtRAMsize != 0) { mem_addr[i] = ExtRAMaddress; mem_len[i] = ExtRAMsize; mem_type[i] = M_RAM; i++; }
        if (ExtRAMsize != 0) { mem_addr[i] = ShadowRAMaddress; mem_len[i] = ExtRAMsize; mem_type[i] = M_RAM; i++; }
        if (IOsize != 0) { mem_addr[i] = IOaddress; mem_len[i] = IOsize; mem_type[i] = M_IO; i++; }
        mem_type[i] = 0;

        ExtraFeatures = true;
        if (SimpleBoot || PoolCount == 0) {
                ExtraFeatures = false;
                RelCount = 0;
        }
        VD("ExtraFeatures=%s", ExtraFeatures ? "true" : "false");

        if (ExtraFeatures) {
                HeaderAddr = UseRAM;
                RelBitsAddr = UseRAM + 0x28;
                RelBitsSize = (ExtRAMsize / 4096) / 8;
                SnapDataAddr = UseRAM + 0x28 + RelBitsSize;
                HolderAddr = UseRAM + 0x28 + RelBitsSize + 0x300;
                InternalCodeAddr = UseRAM + 0x28 + RelBitsSize + 0x300 + 0x20;
                LoadableCodeAddr = UseRAM + 0x28 + RelBitsSize + 0x300 + 0x20 + sizeof(arm_internal);
        } else {
                HeaderAddr = UseRAM;
                RelBitsAddr = NONE;
                SnapDataAddr = NONE;
                HolderAddr = NONE;
                InternalCodeAddr = NONE;
                LoadableCodeAddr = NONE;
        }
        VD("HeaderAddr=0x%08X", HeaderAddr);
        VD("RelBitsAddr=0x%08X", RelBitsAddr);
        VD("RelBitsSize=%i", RelBitsSize);
        VD("SnapDataAddr=0x%08X", SnapDataAddr);
        VD("InternalCodeAddr=0x%08X", InternalCodeAddr);

        if (Changed == NULL || ChangedSize != FlashSize) {
                initChangesArray(FlashSize);
        }

        if (after_init) {
                rpSetAddress(&RP[0], RP[0].InitAddress == NONE ? FlashAddress : RP[0].InitAddress, false);
                rpSetAddress(&RP[1], RP[1].InitAddress == NONE ? ExtRAMaddress : RP[1].InitAddress, false);
                cpSetAddress(CP.InitAddress == NONE ? FlashAddress : CP.InitAddress, false);
                after_init = false;
        }

        VD("reading A65D header...");
        if (! readMemory(HeaderAddr, (unsigned char *)Header, 32))
                return WarningMessage("No debug protocol working\nThe patch is not installed or an old version");

        magic = Header[0];
        sid = Header[1];
        ZeroPage = Header[2];
        LogPage = Header[3];
        TempExecAddr = Header[4];
        MallocPoolAddr = Header[5];

        VD("magic=0x%X", magic);

        if (magic == MAGIC) {
                VD("magic OK");
                VD("sid=0x%X", sid);
                VD("ZeroPage=0x%X", ZeroPage);
                VD("LogPage=0x%X", LogPage);
        } else {
                if (magic & 0xffff0000 == 0xa65d0000) {
                        return WarningMessage("Old version of internal code loaded.\nPlease reboot the phone");
                }
                VD("magic doesn't match");
                TempExecAddr = UseRAM + 0x20;
        }
        VD("TempExecAddr=0x%X", TempExecAddr);

        VD("reading CP15(0,1)...");
        if (! execute(arm_readCP15, sizeof(arm_readCP15), r, 0, cpsr, 1000))
                return WarningMessage("Cannot read CP15 registers");

        // r0 = config register
        // r1 = translation table base address
        // r3 = sp

        VD("CP15(config)=0x%X", r[0]);
        VD("CP15(ttbase)=0x%X", r[1]);
        VD("SP=0x%X", r[3]);

        ConfigRegister = r[0];
        TransTableAddr = r[1];

        if (ExtraFeatures) {

                if ((r[0] & 1) == 0)
                        return WarningMessage("Non-MMU configurations not supported");

                if (((r[0] >> 7) & 1) != 0)
                        return WarningMessage("Big-endian configurations not supported");

                if (((r[0] >> 8) & 3) != 0x2) {
                        newEvent('!', "Unsupported configuration of system protection bits.\n"
                                        "Watchpoint on memory write may not work properly");
                }

                CurrentL2T = NONE;
                CurrentL2Tpage = 0;
        }

        if (magic == MAGIC) {
                if (sid != SessionID) {
                        return WarningMessage("Another session is loaded.\nPlease reboot the phone");
                } else {
                        newstate = SS_READY;
                        if (ExtraFeatures && LogPage != NONE) {

                                if (MallocPoolAddr != NONE) {
                                        PoolAddress[PoolCount-1] = MallocPoolAddr;
                                        if (PoolPages[PoolCount-1] != MallocPages) {
                                                return WarningMessage("Malloc page count don't match");
                                        }
                                } else if (MallocAddress != 0 && MallocPages != 0) {
                                        return WarningMessage("Malloc pool not allocated");
                                }

                                fillRelocationAddressList();

                                VD("reading relocation bits...");
                                if (! readMemory(RelBitsAddr, RelBits, RelBitsSize)) {
                                        return WarningMessage("Cannot read relocation table");
                                }
                                for (i=0; i<RelCount; i++) {
                                        if (isUsed(RelAddrList[i])) PoolFree--;
                                }

                                // initialize LogCurrent, LogLastRead, LostCount
                                readMemory(InternalCodeAddr+IC_LOGDATA*4,  (unsigned char *)buf, 8);
                                LastRead = buf[1];
                                buf[2] = 0;
                                css = (buf[0] >> 8) & 0xff;
                                if (css == 0) {
                                        newstate = SS_READY;
                                } else if (css == 1) {
                                        if (CurrentAddr == NONE) {
                                                return WarningMessage("Unknown debugger state.\nPlease reboot the phone");
                                        }
                                        newstate = SS_INTBRK;
                                } else if (css == 2) {
                                        if (CurrentAddr == NONE) {
                                                buf[0] &= ~0x0000ff00;
                                                newstate = SS_READY;
                                        } else {
                                                newstate = SS_EXTBRK;
                                        }
                                } else if (css == 3) {
                                        return WarningMessage("Unknown debugger state.\nPlease reboot the phone");
                                }
                                VD("initializing LogCurrent=0, LastRead=31, LostCount=0");
                                setMemory(InternalCodeAddr+IC_LOGDATA*4,  (unsigned char *)buf, 12, SM_INTERNAL);
                        } else {
                                VD("snappoints not available.");
                                ExtraFeatures = false;
                        }
                        setSystemState(newstate);
                        newEvent('+', "Connected (device is ready)");
                        return true;
                }
        }

        ZeroPage = Header[2] = NONE;
        LogPage = Header[3] = NONE;

        if (ExtraFeatures) {

                newEvent(' ', "Loading session...");
                repaintEventLog();

                MallocPoolAddr = NONE;
                if (MallocAddress != 0 && MallocPages != 0) {
                        // allocate pool by malloc
                        r[0] = MallocPages * 4096 + 4092;
                        if (! armCall(MallocAddress, r, 1, cpsr, 200)) {
                                return WarningMessage("Error allocating pool area. Check malloc address");
                        }
                        if (r[0] == 0) {
                                return WarningMessage("Error allocating pool area (malloc returns NULL)");
                        }
                        PoolAddress[PoolCount-1] = MallocPoolAddr = (r[0] + 4092) & ~0xfff;
                        if (PoolPages[PoolCount-1] != MallocPages) {
                                return WarningMessage("Malloc page count don't match");
                        }
                }

                fillRelocationAddressList();

                // setup relocation pool
                VD("clearing relocation table...");
                memset(RelBits, 0, RelBitsSize);
                if (! setMemory(RelBitsAddr, RelBits, RelBitsSize, SM_INTERNAL)) {
                        return WarningMessage("Cannot write relocation table");
                }

                VD("setting up shadow areas...");
                a = TransTableAddr + (ShadowFlashAddress / 1048576) * 4;
                n = FlashSize / 1048576;
                for (i=0; i<n; i++) {
                        buf[i] = (FlashAddress + i * 1048576) | 0x12;
                }
                VD("shadow flash tt entry: 0x%08X, size=0x%08X", a, n*4);
                if (! setMemory(a, (unsigned char *)buf, n * 4, SM_INTERNAL)) {
                        return WarningMessage("Cannot write shadow flash area translation table");
                }

                a = TransTableAddr + (ShadowRAMaddress / 1048576) * 4;
                n = ExtRAMsize / 1048576;
                for (i=0; i<n; i++) {
                        buf[i] = (ExtRAMaddress + i * 1048576) | 0xc12;
                }
                VD("shadow ram tt entry: 0x%08X, size=0x%08X", a, n*4);
                if (! setMemory(a, (unsigned char *)buf, n * 4, SM_INTERNAL)) {
                        return WarningMessage("Cannot write shadow RAM area translation table");
                }

                // acquiring zero page

                if (ZeroPage == NONE) {
                        ZeroPage = acquirePage(false);
                        if (ZeroPage == NONE) {
                                return WarningMessage("Cannot acquire zero page");
                        }
                        Header[2] = ZeroPage;
                        VD("acquired zero page: 0x%X", ZeroPage);
                }

                // for 75 series: change mapping scheme to 65-style
                ensureRAMisCoarse();

                // now we can lock zero page
                lockPage(ZeroPage);

                LogPage = acquireVirtualPage();
                TempExecAddr = acquireVirtualPage();
                VD("LogPage=0x%08X", LogPage);
                VD("TempExecAddr=0x%08X", TempExecAddr);

                ip = (unsigned int *)arm_internal;
                ipb = (unsigned char *)arm_internal;
                ip[IC_FLASHBASE] = FlashAddress >> bitShift(FlashSize);
                ipb[ICB_FLASHSHIFT] = bitShift(FlashSize);
                getMemoryInfo(SnapDataAddr, &mi);
                ip[IC_QRAMBASE] = mi.addr >> bitShift(mi.len);
                ipb[ICB_QRAMSHIFT] = bitShift(mi.len);
                ip[IC_XRAMBASE] = ExtRAMaddress >> bitShift(ExtRAMsize);
                ipb[ICB_XRAMSHIFT] = bitShift(ExtRAMsize);
                ip[IC_LOGBUFFER] = LogPage;
                ipb[ICB_LOGCURRENT] = 0;
                ipb[ICB_INBREAK] = 0;
                ip[IC_LOGLASTREAD] = LastRead = 31;
                ip[IC_LOSTCOUNT] = 0;
                ip[IC_SNAPDATA] = SnapDataAddr;
                ip[IC_SNAPMASK] = 0;
                ip[IC_TEMPEXECPAGE] = TempExecAddr;
                ip[IC_HOLDERPOS] = HolderAddr;

                VD_("IC=[");
                for (i=0; i<IC_END; i++) {
                        _VD_("%08X,", ip[i]);
                        if ((i % 8) == 7) _VD_("\n    ");
                }
                _VD_("]\n");
                VD("loading internal code...");
                if (! setMemory(InternalCodeAddr, arm_internal, sizeof(arm_internal), SM_INTERNAL)) {
                        return WarningMessage("Cannot load internal code");
                }

                setAddressNonCacheable(HolderAddr);

                // request field for bkpt 0
                v = 0x00000d00;
                setMemory(SnapDataAddr+0x10, (unsigned char *)&v, 4, SM_INTERNAL);

                // reinstall patches

                VD("reinstalling patches...");
                for (i=0; i<pp_count; i++) {
                        readMemory(PP[i]->address, PP[i]->olddata, PP[i]->length);
                        if (! PP[i]->disabled) {
                                VD("Patch: address=0x%08X, len=0x%08X", PP[i]->address, PP[i]->length);
                                if (! setMemory(PP[i]->address, PP[i]->newdata, PP[i]->length, SM_REINSTALL)) {
                                        sprintf(tbuf, "Cannot set patch at 0x%08X", PP[i]->address);
                                        newEvent('!', tbuf);
                                }
                        }
                }

                // reinstall snaps

                VD("reinstalling snappoints...");
                for (i=1; i<16; i++) {
                        if (SN[i] != NULL) {
                                VD("Snap: num=%i address=0x%08X", i, SN[i]->addr);
                                addSnap(SN[i]->addr, SN[i]->thumb, SN[i]->text, SN[i]->condition, i);
                        }
                }

                // reinstall watchpoint

                if (WatchpointAddr != 0) {
                        VD("reinstalling watchpoint... address=%x08X", WatchpointAddr);
                        setWatchpoint(WatchpointAddr, WatchpointAddr+WatchpointLen);
                }

                // setup exception vectors

                VD("loading exception vectors...");
                r0 = service(IC_READ, 0, 0x10);
                VD("[00000010]=0x%08X", r0);
                readMemory(0, (unsigned char *)buf, 256);
                VD_("b: ");
                for (i=0; i<64; i++) {
                        _VD_("%08X ", buf[i]);
                        if ((i % 8) == 7) _VD_("\n   ");
                }
                _VD_("]\n");

                if (((r0 & 0xfffff000) != 0xe59ff000) ||
                    ((buf[1] &0xfffff000) != 0xe59ff000) ||
                    ((buf[3] &0xfffff000) != 0xe59ff000)
                ) {
                        WarningMessage("Unknown format of exception vector table\nSnappoints and exception handling will not be available");
                } else {
                        // prefetch abort
                        a = 0x0c + (buf[3] & 0xfff) + 8;
                        v = InternalCodeAddr + IC_PFVECTOR*4;
                        service(IC_IMB, v, a);
                        // data abort
                        a = 0x10 + (r0 & 0xfff) + 8;
                        v = InternalCodeAddr + IC_DAVECTOR*4;
                        service(IC_IMB, v, a);
                        // undefined instruction
                        a = 0x04 + (buf[1] & 0xfff) + 8;
                        v = InternalCodeAddr + IC_UIVECTOR*4;
                        service(IC_IMB, v, a);
                }
        } else {
                if (! SimpleBoot) {
                        VD("not enough relocation pool entries.");
                        WarningMessage(
                                "To use full functionality, debugger needs\n"
                                "at least 5 pages in relocation pool.\n"
                                "Please use RAM access analyzer tool to find\n"
                                "unused memory pages"
                        );
                }
                TempExecAddr = UseRAM + 0x20;
        }
        setSystemState(SS_READY);

        Header[0] = MAGIC;
        Header[1] = SessionID;
        Header[2] = ZeroPage;
        Header[3] = LogPage;
        Header[4] = TempExecAddr;
        Header[5] = MallocPoolAddr;
        VD("writing header [%08X,%08X,%08X,%08X,%08X,%08X]...", MAGIC, SessionID, ZeroPage, LogPage, TempExecAddr, MallocPoolAddr);
        setMemory(HeaderAddr, (unsigned char *)Header, 32, SM_INTERNAL);

        newEvent('+', "Connected");

        return true;
}

bool synchronize() {

        bool r;

        VD("synchronize {");
        r = synchronize0();
        VD("} synchronize = %i", r);
        return r;

}

void setSystemState(int state) {

        SystemState = state;
        switch (state) {
          case SS_DISCONNECTED:
                setupMainUI(clBtnFace, "", " No connection", false, false);
                break;
          case SS_CONNECTING:
                setupMainUI(clMaroon, "", " Connecting...", true, false);
                break;
          case SS_READY:
                setupMainUI(clLime, "", " Ready", true, true);
                break;
          case SS_INTBRK:
                setupMainUI(clLime, "I", " Stopped (int.bkpt)", true, true);
                break;
          case SS_EXTBRK:
                setupMainUI(clLime, "E", " Stopped (ext.bkpt)", true, true);
                break;
        }
}

void eventMonitor(bool user) {

        unsigned char buf[4096], *cp, nr;
        char *mbuf, c, *p, *pp, *sp;
        unsigned int rb, lc, ls, lca, lsa, le, z=0;
        unsigned int *xp, xp15, new_r15, dif;
        unsigned short *ssp;
        int i, j, n, t, len, ulen, is, inbreak;
        snap **sn;

        if (! is_online()) return;
        if (! ExtraFeatures) return;
        if (inprocess) return;

        VD("eventTimer - checking...");

        ls = (LastRead+1) & 0x1f;
        if (! readMemory(InternalCodeAddr + IC_LOGDATA*4, (unsigned char *)&rb, 4)) return;
        lc = rb & 0xff;
        inbreak = (rb >> 8) & 0xff;
        if (ls == lc) return;

        VD("eventTimer (%i,%i) {", ls, lc);

        lsa = LogPage + ls * 128;
        lca = LogPage + lc * 128;
        if (lsa < lca) {
                readMemory(lsa, buf, lca-lsa);
                n = lc - ls;
        } else {
                len = LogPage+4096-lsa;
                readMemory(lsa, buf, len);
                readMemory(LogPage, buf+len, lca-LogPage);
                n = lc + 32 - ls;
        }

        sn = getSnapList();
        mbuf = (char *)malloc(32768);
        MainForm->LogWindow->Lines->BeginUpdate();
        for (i=0; i<n; i++) {
                cp = buf + i * 128;
                xp = (unsigned int *)cp;
                xp15 = xp[15];
                is = (xp15 >> 24) & 0xf;

                if (xp15 == 0) {
                        // watchpoint
                        sprintf(mbuf, "Write access to %08X, PC=%08X", xp[0], xp[14]-4);
                        newEvent('*', mbuf);
                        continue;
                }

                // snappoints

                if (is != 0 && sn[is] == NULL) {
                        sprintf(mbuf, "Unexpected snappoint %i", is);
                        newEvent('!', mbuf);
                        continue;
                }

                if (is == 0 || sn[is]->isbreak) {
                        // breakpoint
                        if (inbreak == 0) {
                                newEvent('!', "Spurious breakpoint event (mobile state error)");
                                continue;
                        }
                        memcpy(CurrentRs, xp, 16*4);
                        memcpy(CurrentStack, &xp[16], 16*4);
                        if (is == 0) {
                                CurrentAddr = CurrentRs[15] = Break0addr;
                                CurrentThumb = Break0thumb;
                                if (SubsR15 && SubsAddress != NONE) {
                                        readFromRAM(SubsAddress, (unsigned char *)&new_r15, 4);
                                        if (ThumbAware) CurrentThumb = ((new_r15 & 1) != 0);
                                        CurrentAddr = CurrentRs[15] = CurrentThumb ? (new_r15 & ~1) : (new_r15 & ~3);
                                        SubsR15 = false;
                                        SubsAddress = NONE;
                                        updateRegister(15, CurrentThumb ? (new_r15 | 1) : new_r15);
                                }
                        } else {
                                CurrentAddr = CurrentRs[15] = sn[is]->addr;
                                CurrentThumb = sn[is]->thumb;
                        }

                        if (user) {
                                CurrentCPSR = xp15 & 0xf0000000;
                                setSystemState((inbreak == 1) ? SS_INTBRK : SS_EXTBRK);
                                cpSetAddress(CurrentThumb ? CurrentAddr|1 : CurrentAddr, true);
                                setRegisters(CurrentRs, CurrentCPSR, 16, CurrentStack);
                                MainForm->Pages->ActivePage = MainForm->SheetCPU;
                                MainForm->vCPU->SetFocus();
                                if (is != 0 || ReloadQuery) {
                                        ReloadQuery = false;
                                        reload_dumps();
                                }
                        }
                        continue;
                }

                p = sn[is]->compiled;
                pp = mbuf;
                while (*p) {
                        c = *(p++);
                        switch (c) {
                                case 0x01:
                                        pp += sprintf(pp, "%X", xp[*p-0x30]);
                                        p++;
                                        break;
                                case 0x02:
                                        pp += sprintf(pp, "%i", xp[*p-0x30]);
                                        p++;
                                        break;
                                case 0x03:
                                        *(pp++) = (xp15 & 0x80000000) ? 'N' : 'n';
                                        *(pp++) = (xp15 & 0x40000000) ? 'Z' : 'z';
                                        *(pp++) = (xp15 & 0x20000000) ? 'C' : 'c';
                                        *(pp++) = (xp15 & 0x10000000) ? 'V' : 'v';
                                        break;
                                case 0x04:
                                        t = (xp15 & 0xffffff) - SnapTime;
                                        if (t < 0) t += 0x1000000;
                                        pp += sprintf(pp, "%i.%03i", t / 1000, t % 1000);
                                        break;
                                case 0x05:
                                case 0x06:
                                        nr = *(p++);
                                        if (nr != 0xff && xp[nr-0x30] == 0) {
                                                pp += sprintf(pp, "NULL");
                                                p += 2;
                                                break;
                                        }
                                        dif = *(p+1) - 0x30;
                                        switch (*p) {
                                                case 0x30:
                                                        pp += sprintf(pp, (c==5)?"%X":"%i", *(cp+64+dif));
                                                        break;
                                                case 0x31:
                                                        pp += sprintf(pp, (c==5)?"%X":"%i", *((unsigned int *)(cp+64+dif)));
                                                        break;
                                                case 0x32:
                                                        *(pp++) = '\"';
                                                        sp = (char *)(cp+64+dif);
                                                        while (*sp && sp-cp < 128) {
                                                                if (*sp >= 0x20) {
                                                                        *(pp++) = *(sp++);
                                                                } else {
                                                                        *(pp++) = '_';
                                                                        sp++;
                                                                }
                                                        }
                                                        *(pp++) = '\"';
                                                        break;
                                                case 0x33:
                                                        *(pp++) = '\"';
                                                        ssp = (unsigned short *)(cp+64+dif);
                                                        while (*ssp && ((char *)ssp)-cp < 128) {
                                                                if (*ssp < 0x20) {
                                                                        *(pp++) = '_';
                                                                        ssp++;
                                                                } else if (*ssp < 0x80) {
                                                                        *(pp++) = *(ssp++);
                                                                } else {
                                                                        *(pp++) = '?';
                                                                        ssp++;
                                                                }
                                                        }
                                                        *(pp++) = '\"';
                                                        break;
                                                case 0x2f:
                                                        *(pp++) = '\"';
                                                        ssp = (unsigned short *)(cp+64);
                                                        ulen = *(ssp++);
                                                        while (ulen > 0) {
                                                                if (*ssp < 0x20) {
                                                                        *(pp++) = '_';
                                                                        ssp++;
                                                                } else if (*ssp < 0x80) {
                                                                        *(pp++) = *(ssp++);
                                                                } else {
                                                                        *(pp++) = '?';
                                                                        ssp++;
                                                                }
                                                                ulen--;
                                                        }
                                                        *(pp++) = '\"';
                                                        break;
                                                default:
                                                        len = *p - 0x30;
                                                        for (j=0; j<len; j++) {
                                                                pp += sprintf(pp, (c==5)?"%02X ":"%i ", *(cp+64+dif+j));
                                                                if ((j % 16) == 15) pp += sprintf(pp, "\r\n");
                                                        }
                                                        break;
                                        }
                                        p += 2;
                                        break;
                                case 0x07:
                                        pp += sprintf(pp, "%08X,%08X,%08X,%08X,%08X,%08X,%08X,%08X",
                                                xp[16], xp[17], xp[18], xp[19], xp[20], xp[21], xp[22], xp[23]
                                        );
                                        break;
                                case 0x08:
                                        pp += sprintf(pp,
                                                "R0=%08X  R4=%08X  R8=%08X R12=%08X\n"
                                                "R1=%08X  R5=%08X  R9=%08X  LR=%08X\n"
                                                "R2=%08X  R6=%08X R10=%08X\n"
                                                "R3=%08X  R7=%08X R11=%08X\n",
                                                xp[0], xp[4], xp[8], xp[12],
                                                xp[1], xp[5], xp[9], xp[14],
                                                xp[2], xp[6], xp[10],
                                                xp[3], xp[7], xp[11]
                                        );
                                        break;
                                default:
                                        *(pp++) = c;
                                        break;
                        }
                }
                SnapTime = xp15 & 0xffffff;
                *pp = 0;
                newEvent('*', mbuf);
        }

        if (n == 31) {
                readMemory(InternalCodeAddr + IC_LOSTCOUNT*4, (unsigned char *)&le, 4);
                setMemory(InternalCodeAddr + IC_LOSTCOUNT*4, (unsigned char *)&z, 4, SM_INTERNAL);
                if (le > 0) {
                        sprintf(mbuf, "%i event(s) lost", le);
                        newEvent('!', mbuf);
                }
        }

        LastRead = ls = (lc - 1) & 0x1f;
        setMemory(InternalCodeAddr + IC_LOGLASTREAD*4, (unsigned char *)&ls, 4, SM_INTERNAL);

        MainForm->LogWindow->Lines->EndUpdate();
        free(mbuf);

        VD("} eventTimer");

}

// -------------------------------------------------------------------


unsigned int acquirePage(bool lock) {

        int i;

        VD("acquirePage(%i) {", lock);

        for (i=0; i<RelCount; i++) {
                if (! isUsed(RelAddrList[i])) break;
        }
        if (i == RelCount) {
                WarningMessage("No free pages in relocation pool");
                VD("} acquirePage (no free pages)");
                return NONE;
        }

        unsigned int a = RelAddrList[i];
        VD("acquire=0x%08X", a);
        if (lock && ! lockPage(a)) {
                VD("} acquirePage (lockpage failed)");
                return NONE;
        }
        setUsed(a);
        PoolFree--;
        updateFreePoolEntries(PoolFree);

        VD("} acquirePage");
        return a;

}

bool setDivPage(unsigned int addr, unsigned int newaddr) {

        unsigned int secentry[16], buf[4096];

        VD("setDivPage(0x%08X, 0x%08X) {", addr, newaddr);

        unsigned int secptr = TransTableAddr + (addr / 1048576) * 4;
        if (! readFromRAM(secptr, (unsigned char *)secentry, 4)) {
                VD("} setDivPage (readFromRAM failed)");
                return false;
        }
        VD("secentry: [0x%08X]=0x%08X", secptr, secentry[0]);
        if ((secentry[0] & 0x3) != 0x1) {
                VD("} setDivPage (secentry & 3 != 1)");
                WarningMessage("setDivPage: error in memory mapping");
                return false;
        }
        unsigned int a = (secentry[0] & ~0x3ff) + ((addr % 1048576) / 4096) * 4;
        VD("ram=0x%08X", a);
        a = shadow(a);
        if (! readFromRAM(a, (unsigned char *)buf, 4)) {
                VD("} setDivPage (readFromRAM failed)");
                return false;
        }
        buf[0] = (buf[0] & 0xfff) | (newaddr & ~0xfff);
        if (! writeToRAM(a, (unsigned char *)buf, 4)) {
                VD("} setDivPage (writeToRAM failed)");
                return false;
        }
        VD("} setDivPage");
        return true;

}

bool splitPage(unsigned int addr, unsigned int l2taddr) {

        unsigned int secptr, secentry[16], buf[4096];
        unsigned int a, r0, r1;

        VD("splitPage(0x%08X, 0x%08X) {", addr, l2taddr);

        secptr = TransTableAddr + (addr / 1048576) * 4;
        if (! readFromRAM(secptr, (unsigned char *)secentry, 4)) {
                VD("} splitPage (readFromRAM failed)");
                return false;
        }
        VD("secentry: [0x%08X]=0x%08X", secptr, secentry[0]);

        if ((secentry[0] & 0x3) != 0x1) {
                if ((secentry[0] & 0x3) != 0x2) {
                        VD("} splitPage (secentry & 3 != 2)");
                        WarningMessage("Unknown memory mapping scheme");
                        return false;
                }

                if (l2taddr == 0) {

                        // section descriptor
                        if (CurrentL2T == NONE) {
                                CurrentL2T = acquirePage(true);
                                if (CurrentL2T == NONE) {
                                        VD("} splitPage (cannot acquire page)");
                                        return false;
                                }
                                CurrentL2Tpage = 0;
                                setDivPage(CurrentL2T, ZeroPage);
                        }
                        VD("CurrentL2T=0x%08X, page=%i", CurrentL2T, CurrentL2Tpage);

                        l2taddr = CurrentL2T + CurrentL2Tpage * 1024;

                }

                unsigned int apbits = (secentry[0] >> 10) & 0x3;
                unsigned int domain = (secentry[0] >> 5) & 0xf;
                unsigned int cbbits = (secentry[0] >> 2) & 0x3;
                unsigned int base = (secentry[0] & ~0xfffff);

                r1 = shadow(l2taddr);
                r0 = base | (apbits << 10) | (apbits << 8) | (apbits << 6) | (apbits << 4) | (cbbits << 2) | 0x2;
                VD("filling L2T(0x%08X, 0x%08X)...", r0, r1);
                service(IC_FILLL2T, r0, r1);

                secentry[0] = l2taddr | (domain << 5) | 0x11;
                VD("writing secentry (0x%08X)...", secentry[0]);
                if (! writeToRAM(secptr, (unsigned char *)secentry, 4)) {
                        VD("} getDivPage (writeToRAM failed)");
                        return false;
                }
                // need to clear TLB

                CurrentL2Tpage++;
                if (CurrentL2Tpage == 4) {
                        CurrentL2T = NONE;
                        CurrentL2Tpage = 0;
                }
        }

        VD("} splitPage");
        return true;

}


unsigned int getDivPage(unsigned int addr) {

        unsigned int secptr, secentry[16], buf[16];
        unsigned int a, r0, r1;

        VD("getDivPage(0x%08X) {", addr);

        if (! splitPage(addr, 0)) return NONE;

        secptr = TransTableAddr + (addr / 1048576) * 4;
        if (! readFromRAM(secptr, (unsigned char *)secentry, 4)) {
                VD("} getDivPage (readFromRAM failed)");
                return NONE;
        }
        a = (secentry[0] & ~0x3ff) + ((addr % 1048576) / 4096) * 4;
        VD("reading tt2 entry (at 0x%X)...", a);
        if (! readFromRAM(shadow(a), (unsigned char *)buf, 4)) {
                VD("} getDivPage (readFromRAM failed)");
                return NONE;
        }

        a = buf[0] & ~0xfff;
        VD("} getDivPage = 0x%08X", a);
        return a;

}

void setAddressNonCacheable(unsigned int addr) {

        unsigned int secentry[16], buf[4096];

        VD("setAddressNonCacheable(0x%08X) {", addr);

        unsigned int secptr = TransTableAddr + (addr / 1048576) * 4;
        if (! readFromRAM(secptr, (unsigned char *)secentry, 4)) {
                VD("} setAddressNonCacheable (readFromRAM failed)");
                return;
        }
        VD("secentry: [0x%08X]=0x%08X", secptr, secentry[0]);

        if ((secentry[0] & 0x3) == 0x1) {               // coarse

                unsigned int a = (secentry[0] & ~0x3ff) + ((addr % 1048576) / 4096) * 4;
                VD("reading tt2 entry (at 0x%X)...", a);
                if (! readFromRAM(shadow(a), (unsigned char *)buf, 4)) {
                        VD("} setAddressNonCacheable (readFromRAM failed)");
                        return;
                }
                buf[0] = (buf[0] & ~0x08);      /////
                if ((buf[0] & 0x3) == 0x3) {
                        buf[0] = buf[0] | 0x30;
                } else {
                        buf[0] = buf[0] | 0xff0;
                }
                VD("writing tt2 entry (=0x%X)...", buf[0]);
                if (! writeToRAM(shadow(a), (unsigned char *)buf, 4)) {
                        VD("} setAddressNonCacheable (writeToRAM failed)");
                        return;
                }

        } else if ((secentry[0] & 0x3) == 0x2) {        // section

                secentry[0] = (secentry[0] & ~0x08); /////
                secentry[0] = (secentry[0] | 0xc00);
                VD("writing tt entry (=0x%X)...", secentry[0]);
                if (! writeToRAM(secptr, (unsigned char *)secentry, 4)) {
                        VD("} setAddressNonCacheable (writeToRAM failed)");
                        return;
                }

        } else {
                VD("} setAddressNonCacheable (unsupported memory mapping)");
                return;
        }

        VD("} setAddressNonCacheable");

}

bool setPagePermissions(unsigned int addr, unsigned int ap) {

        unsigned int secentry[16], buf[4096];

        VD("setPagePermissions(0x%08X, %X) {", addr, ap);

        unsigned int secptr = TransTableAddr + (addr / 1048576) * 4;
        if (! readFromRAM(secptr, (unsigned char *)secentry, 4)) {
                VD("} setPagePermissions (readFromRAM failed)");
                return false;
        }
        VD("secentry: [0x%08X]=0x%08X", secptr, secentry[0]);

        if ((secentry[0] & 0x3) != 0x1) {
                VD("} setPagePermissions (secentry & 3 != 1)");
                WarningMessage("lockPage: cannot lock page in this area\n(unsupported memory mapping type)");
                return false;
        }
        unsigned int a = (secentry[0] & ~0x3ff) + ((addr % 1048576) / 4096) * 4;
        VD("reading tt2 entry (at 0x%X)...", a);
        if (! readFromRAM(shadow(a), (unsigned char *)buf, 4)) {
                VD("} setPagePermissions (readFromRAM failed)");
                return false;
        }
        buf[0] = (buf[0] & ~0xff0) | ap;
        VD("writing tt2 entry (=0x%X)...", buf[0]);
        if (! writeToRAM(shadow(a), (unsigned char *)buf, 4)) {
                VD("} setPagePermissions (writeToRAM failed)");
                return false;
        }

        VD("} setPagePermissions");
        return true;

}

bool lockPage(unsigned int addr) { return setPagePermissions(addr, 0); }
bool lockPageUser(unsigned int addr) { return setPagePermissions(addr, 0xaa0); }
bool unlockPage(unsigned int addr) { return setPagePermissions(addr, 0xff0); }

unsigned int acquireVirtualPage() {

        unsigned int a, r;

        VD("acquireVirtualPage {");
        a = acquirePage(true);
        if (a == NONE) {
                r = NONE;
        } else {
                setDivPage(a, ZeroPage);
                r = shadow(a);
        }
        VD("} acquireVirtualPage = %08X", r);
        return r;

}

bool relocatePage(unsigned int addr) {

        unsigned int r[16], cpsr;

        VD("relocatePage(0x%08X) {", addr);

        if (ZeroPage == NONE) {
                VD("} relocatePage (ZeroPage=-1)");
                return false;
        }

        unsigned int ap = getDivPage(addr);
        VD("ap(0x%08X)=0x%08X", addr, ap);
        if (ap == NONE) {
                VD("} relocatePage (ap=-1)");
                return false;
        }

        if (ap == (addr & ~0xfff)) {
                unsigned int apr = acquirePage(true);
                if (apr == NONE) {
                        VD("} relocatePage (apr=-1)", addr);
                        return false;
                }
                VD("setting page to Z...");
                setDivPage(apr, ZeroPage);
                VD("copying from 0x%08X to 0x%08X...", ap, apr);
                if (! copyData(shadow(apr), ap, 4096)) {
                        VD("} relocatePage (copyData failed)", addr);
                        return false;
                }
                VD("setting ap to apr...");
                setDivPage(ap, apr);
        }
        service(IC_CLEARTLB, 0, 0);

        VD("} relocatePage", addr);
        return true;
}

bool ensureRAMisCoarse() {

        unsigned int e_addr, a, mask, changes, cpage, csubpage, tt2a;
        int e_count;
        unsigned int buf[4096], tt2[4096], pages[16];
        unsigned int r[16], cpsr;
        int i, j, n, npages;

        VD("ensureRAMisCoarse {");

        if (ExtRAMsize != 0) {
                e_addr = TransTableAddr + (ExtRAMaddress / 0x100000) * 4;
                e_count = (ExtRAMsize / 0x100000);
                changes = 0;

                VD("reading ram tt...");
                if (! readMemory(e_addr, (unsigned char *)buf, e_count * 4)) {
                        VD("} ensureRAMisCoarse (error reading ram)");
                        return false;
                }

                n = 0;
                for (i=0; i<e_count; i++) {
                        if ((buf[i] & 0x03) == 0x02) n++;
                }

                npages = (n + 3) / 4;
                for (i=0; i<npages; i++) {
                        pages[i] = acquirePage(false);
                        if (pages[i] == NONE) {
                                VD("} ensureRAMisCoarse (cannot acquire page)");
                                return false;
                        }
                }
                cpage = 0;
                csubpage = 0;

//                for (i=0; i<e_count; i++) {
                for (i=e_count-1; i>=0; i--) {
                        if ((buf[i] & 0x03) == 0x02) {
                                // section
                                a = buf[i] & 0xfff00000;
                                mask = (buf[i] & 0xc00);
                                mask |= (mask >> 2);
                                mask |= (mask >> 4);
                                mask |= (buf[i] & 0x0c);
                                mask |= 0x02;

                                tt2a = pages[cpage] + csubpage*1024;

                                VD("coarsing: %08X -> %08X", a, tt2a);
                                r[0] = a | mask;
                                r[1] = tt2a;
                                execute(arm_fillL2T, sizeof(arm_fillL2T), r, 2, cpsr, 200);
/*
                                for (j=0; j<256; j++) {
                                        tt2[j] = (a + j * 0x1000) | mask;
                                }
                                VD("coarsing: %08X -> %08X", a, tt_free);
                                setMemory(tt_free,  (unsigned char *)tt2, 0x400, SM_INTERNAL);
*/
                                buf[i] = tt2a | 0x11;
                                changes = 1;

                                csubpage++;
                                if (csubpage == 4) { cpage++; csubpage = 0; }

                        } else if ((buf[i] & 0x03) == 0x01) {
                                // coarse
                                r[0] = ZeroPage | 0xffe;
                                r[1] = buf[i] & ~0x3ff;
                                execute(arm_checkL2T, sizeof(arm_checkL2T), r, 2, cpsr, 200);
                        }
                }
                if (changes) {
                        VD("updating ram tt (0x%08x)...", e_addr);
                        if (!setMemory(e_addr,  (unsigned char *)buf, e_count * 4, SM_INTERNAL)) {
                                VD("} ensureRAMisCoarse (error writing tt1)");
                                return false;
                        }
                }

                for (i=0; i<npages; i++) {
                        // setDivPage(pages[i], ZeroPage);
                        lockPage(pages[i]);
                }

        }

        VD("} ensureRAMisCoarse");
        return true;
}

// ---------------------------------------------------------------------------

void initChangesArray(int size) {

        VD("initChangesArray(%08X);", size);

        if (Changed != NULL) free(Changed);
        if (size != 0) {
                Changed = (unsigned char *)calloc(size/8, 1);
                ChangedSize = size;
        } else {
                Changed = NULL;
                ChangedSize = 0;
        }

}

void setChanged(unsigned int addr, unsigned int size) {

        unsigned int a1, a2;
        unsigned char m1, m2;
        unsigned int i;

        VD("setChanged(0x%08X, %i);", addr, size);

        if (addr < FlashAddress || addr >= FlashAddress+FlashSize-1) return;
        a1 = (addr - FlashAddress) >> 3;
        m1 = 0xff << (addr & 0x7);
        a2 = (addr + size - FlashAddress - 1) >> 3;
        m2 = 0xff >> (7 - ((addr + size - FlashAddress - 1) & 0x7));

        if (a1 == a2) {
                Changed[a1] |= (m1 & m2);
        } else {
                Changed[a1] |= m1;
                for (i=a1+1; i<a2; i++) Changed[i] = 0xff;
                Changed[a2] |= m2;
        }

}

void clearChanged(unsigned int addr, unsigned int size) {

        unsigned int a1, a2;
        unsigned char m1, m2;
        unsigned int i;

        VD("clearChanged(0x%08X, %i);", addr, size);

        if (addr < FlashAddress || addr >= FlashAddress+FlashSize-1) return;
        a1 = (addr - FlashAddress) >> 3;
        m1 = 0xff << (addr & 0x7);
        a2 = (addr + size - FlashAddress - 1) >> 3;
        m2 = 0xff >> (7 - ((addr + size - FlashAddress - 1) & 0x7));

        if (a1 == a2) {
                Changed[a1] &= ~(m1 & m2);
        } else {
                Changed[a1] &= ~m1;
                for (i=a1+1; i<a2; i++) Changed[i] = 0x00;
                Changed[a2] &= ~m2;
        }

}

bool isChanged(unsigned int addr) {

        if (Changed == NULL || addr < FlashAddress || addr >= FlashAddress+FlashSize-1) return false;
        unsigned int a = addr - FlashAddress;
        return Changed[a >> 3] & (1 << (a & 0x7));

}

bool isChanged4(unsigned int addr) {

        unsigned int a, r;

        if (Changed == NULL || addr < FlashAddress || addr >= FlashAddress+FlashSize-1) return false;
        a = (addr & ~0x3) - FlashAddress;
        r = Changed[a >> 3] & (1 << (a & 0x7)); a++;
        r |= Changed[a >> 3] & (1 << (a & 0x7)); a++;
        r |= Changed[a >> 3] & (1 << (a & 0x7)); a++;
        r |= Changed[a >> 3] & (1 << (a & 0x7));
        return r;

}



// ---------------------------------------------------------------------------

void emptyPatchList() {

        VD("emptyPatchList();");

        int i;
        if (PP == NULL) return;
        for (i=0; i<pp_count; i++) {
                free(PP[i]->olddata);
                free(PP[i]->newdata);
                free(PP[i]);
        }
        free(PP);
        PP = NULL;
        pp_count = 0;

}

//  add patch to internal patches repository
patch *addPatch(unsigned int addr, unsigned char *olddata, unsigned char *newdata, unsigned int len, int stype) {

        unsigned char *olddata_orig = olddata;
        unsigned char *newdata_orig = newdata;
        patch *p;
        unsigned int ca, cl, newlen;
        unsigned char *cndata, *codata;
        unsigned char *nbuf, *obuf;
        int i, stick, order;

        VD("addPatch(addr=0x%08X, len=0x%08X, stype=%i) {", addr, len, stype);

        olddata = (unsigned char *)malloc(len);
        newdata = (unsigned char *)malloc(len);
        memcpy(olddata, olddata_orig, len);
        memcpy(newdata, newdata_orig, len);
        stick = (stype == SM_STICKY) ? 17 : 0;

        for (i=0; i<pp_count; i++) {

                p = PP[i];
                ca = p->address;
                cl = p->length;
                cndata = p->newdata;
                codata = p->olddata;

                if (ca <= addr && ca + cl + stick > addr) {

                        newlen = (addr - ca) + len;
                        if (newlen < cl) newlen = cl;
                        nbuf = (unsigned char *)malloc(newlen);
                        obuf = (unsigned char *)malloc(newlen);
                        memcpy(nbuf, cndata, cl);
                        if (addr > ca+cl) {
                                readMemory(ca+cl, nbuf+cl, addr-(ca+cl));
                                memcpy(obuf+cl, nbuf+cl, addr-(ca+cl));
                        }
                        memcpy(nbuf+(addr-ca), newdata, len);
                        memcpy(obuf+(addr-ca), olddata, len);
                        memcpy(obuf, codata, cl);
                        free(olddata);
                        free(newdata);
                        addr = ca;
                        olddata = obuf;
                        newdata = nbuf;
                        len = newlen;
                        deletePatch(ca);
                        i--;

                } else if (addr < ca && addr + len + stick > ca) {

                        newlen = (ca - addr) + cl;
                        if (newlen < len) newlen = len;
                        nbuf = (unsigned char *)malloc(newlen);
                        obuf = (unsigned char *)malloc(newlen);
                        memcpy(nbuf+(ca-addr), cndata, cl);
                        if (ca > addr+len) {
                                readMemory(addr+len, nbuf+len, ca-(addr+len));
                                memcpy(obuf+len, nbuf+len, ca-(addr+len));
                        }
                        memcpy(nbuf, newdata, len);
                        memcpy(obuf, olddata, len);
                        memcpy(obuf+(ca-addr), codata, cl);
                        free(olddata);
                        free(newdata);
                        olddata = obuf;
                        newdata = nbuf;
                        len = newlen;
                        deletePatch(ca);
                        i--;

                }
        }

        i = pp_count;
        pp_count++;
        int s = sizeof(struct patch *);
        PP = (struct patch **)realloc(PP, pp_count * s);
        if (pp_count-i-1 > 0) memmove(&PP[i+1], &PP[i], (pp_count-i-1)*s);
        PP[i] = (struct patch *)malloc(sizeof(struct patch));
        PP[i]->address = addr;
        PP[i]->length = len;
        PP[i]->olddata = olddata;
        PP[i]->newdata = newdata;
        PP[i]->disabled = false;

        setChanged(addr, len);

        VD("} addPatch");
        return PP[i];

}

//  remove patch from internal patches repository
void deletePatch(unsigned int addr) {

        int i;
        unsigned int a, l;

        VD("deletePatch(addr=0x%08X) {", addr);

        for (i=0; i<pp_count; i++) {
                if (PP[i]->address == addr) break;
        }

        if (i == pp_count) return;

        a = PP[i]->address;
        l = PP[i]->length;
        clearChanged(a, l);

        free(PP[i]->olddata);
        free(PP[i]->newdata);
        free(PP[i]);

        int s = sizeof(struct patch *);
        if (pp_count-i-1 > 0) memmove(&PP[i], &PP[i+1], (pp_count-i-1)*s);
        pp_count--;

        VD("} deletePatch");
}

patch *getPatchByAddress(unsigned int addr) {

        int i;

        for (i=0; i<pp_count; i++) {
                if (PP[i]->address <= addr && (PP[i]->address + PP[i]->length - 1) >= addr) break;
        }

        if (i == pp_count) return NULL;

        return PP[i];

}

int getPatchCount() {
        return pp_count;
}

patch **getPatchList() {
        return PP;
}

// -------------------------------------------------------------------

int a2find(aname **xx, int n, unsigned int addr) {

        int i = n/2;
        int k = n/4;

        if (n == 0) return 0;
        while (k > 0) {
                i = (xx[i]->address >= addr) ? i-k : i+k;
                k /= 2;
        }
        while (i < n && xx[i]->address < addr) i++;
        while (i > 0 && xx[i-1]->address >= addr) i--;

        return i;

}

void emptyNamesList(bool is_global) {

        int i;

        VD("emptyNamesList();");

        if (NN && is_global) {
                for (i=0; i<nn_count; i++) {
                        free(NN[i]->name);
                        free(NN[i]);
                }
                free(NN);
                nn_count = 0;
                NN = NULL;
        }
        if (LL && ! is_global) {
                for (i=0; i<ll_count; i++) {
                        free(LL[i]->name);
                        free(LL[i]);
                }
                free(LL);
                ll_count = 0;
                LL = NULL;
        }
}

void addNameFast(char *name, unsigned int addr, bool is_global) {

        aname **xx;
        int i, k, n;

        removeName(addr);

        xx = is_global ? NN : LL;
        n = is_global ? nn_count : ll_count;

        i = a2find(xx, n, addr);

        xx = (aname **)realloc(xx, ((n+1) * sizeof(aname *) + 4096) | 4095);
        if (i < n) {
                memmove(&xx[i+1], &xx[i], (n-i) * sizeof(aname *));
        }
        n++;
        xx[i] = (aname *)malloc(sizeof(aname));
        xx[i]->name = strdup(name);
        xx[i]->address = addr;

        if (is_global) NN=xx; else LL=xx;
        if (is_global) nn_count=n; else ll_count=n;

}

void addName(char *name, unsigned int addr, bool is_global) {

        if (addressByName(name) != NONE) {
                WarningMessage("Name is not unique");
                return;
        }
        addNameFast(name, addr, is_global);

}

void removeName(char *name) {

        unsigned int a;

        a = addressByName(name);
        if (a == NONE) return;
        removeName(a);

}

void removeName(unsigned int a) {

        aname **xx;
        int i, n;
        bool g;

        i = a2find(LL, ll_count, a);
        if (i < ll_count && LL[i]->address == a) {
                xx = LL;
                n = ll_count;
                g = false;
        } else {
                i = a2find(NN, nn_count, a);
                if (i < nn_count && NN[i]->address == a) {
                        xx = NN;
                        n = nn_count;
                        g = true;
                } else {
                        return;
                }
        }

        free(xx[i]->name);
        free(xx[i]);
        memmove(&xx[i], &xx[i+1], (n-i) * sizeof(aname *));
        n--;

        if (g) NN=xx; else LL=xx;
        if (g) nn_count=n; else ll_count=n;

}

char *nameByAddress(unsigned int addr) {

        int i;

        i = a2find(LL, ll_count, addr);
        if (i < ll_count && LL[i]->address == addr) {
                return LL[i]->name;
        }

        i = a2find(NN, nn_count, addr);
        if (i < nn_count && NN[i]->address == addr) {
                return NN[i]->name;
        }

        return NULL;

}

unsigned int addressByName(char *name) {

        int i;

        for (i=0; i<ll_count; i++) {
                if (stricmp(LL[i]->name, name) == 0) return LL[i]->address;
        }
        for (i=0; i<nn_count; i++) {
                if (stricmp(NN[i]->name, name) == 0) return NN[i]->address;
        }

        return NONE;

}

unsigned int addressForText(char *p) {

        char *reg_t[] = { "r0","r1","r2","r3","r4","r5","r6","r7","r8","r9","r10",
                          "r11","r12","r13","r14","r15","sp","lr","pc",NULL
        };
        int reg_n[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
                          11, 12, 13, 14, 15, 13, 14, 15
        };
        unsigned int addr;
        char *pp;
        int i, l, n;

        if (*p == '\0') return NONE;
        for (i=0; reg_t[i]!=NULL; i++) {
                l = strlen(reg_t[i]);
                if (strnicmp(p, reg_t[i], l) == 0 && p[l] <= ' ') {
                        n = reg_n[i];
                        return getRegister(n);
                }
        }
        addr = strtoul(p, &pp, 16);
        if (*pp <= ' ') {
                return addr;
        } else {
                return addressByName(p);
        }
}

int getNameCount(bool is_global) {
        return is_global ? nn_count : ll_count;
}

aname **getNameList(bool is_global) {
        return is_global ? NN : LL;
}

// -------------------------------------------------------------------

void emptySnapList() {

        int i;

        VD("emptySnapList();");

        for (i=0; i<16; i++) {
                if (SN[i] != NULL) {
                        free(SN[i]->text);
                        free(SN[i]->compiled);
                }
                free(SN[i]);
                SN[i] = NULL;
        }
}

int freeSnap() {

        int i;

        for (i=1; i<16; i++) {
                if (SN[i] == NULL) return i;
        }
        return -1;
}

bool snapEMsg(char *text, char *cp, char *message) {

        int x=1, y=1;

        char *p = text;
        while (p < cp) {
                x++;
                if (*p == 0x0a) { y++; x=1; }
                p++;
        }

        return WarningMessage("Parse error at line %i, col %i:\n%s", y, x, message);

}

bool compileSnap(unsigned int addr, char *text, char *compiled, unsigned int *request) {

        char c, cc, *p, *pp, *v;
        unsigned int n, dif;
        unsigned char nr;
        int offset;
        bool hx, double_ptr;

        VD("compileSnap(0x%08X, \"%s\");", addr, text);

        if (text[0] == '~') {
                // breakpoint
                strcpy(compiled, text);
                *request = 0x00000d00;
                return true;
        }

        *request = 0xffffffff;
        p = text;
        pp = compiled;
        while (*p) {
                c = *(p++);
                if (c == '%' || c == '$' || c == '@' || c == '&') {
                        hx = (c != '$');
                        cc = *(p++);
                        if (c == '@' || c == '&') { cc = '['; p--; }
                        switch (cc) {
                                case 'A':
                                case 'a':
                                        pp += sprintf(pp, hx?"%08X":"%i", addr);
                                        break;
                                case 'C':
                                case 'c':
                                        *(pp++) = 0x03;
                                        break;
                                case 'R':
                                case 'r':
                                        if (*p == 'R' || *p == 'r') {
                                                *(pp++) = 0x08;
                                                p++;
                                        } else if (*p >= '0' && *p <= '9') {
                                                n = strtoul(p, &v, 10);
                                                if (v == p || n > 15) return snapEMsg(text, p, "Register name expected");
                                                if (n == 15) return snapEMsg(text, p, "Value of PC not accessible, use %A instead");
                                                *(pp++) = hx?1:2;
                                                *(pp++) = 0x30+n;
                                                p = v;
                                        } else {
                                                return snapEMsg(text, p, "Register number expected");
                                        }
                                        break;
                                case 'Q':
                                case 'q':
                                        return snapEMsg(text, p, "%Q is not supported anymore");
                                case 'S':
                                case 's':
                                        if (*request != 0xffffffff) return snapEMsg(text, p, "%%S/%%[...] can be used only once");
                                        *request = 0x00000d00;
                                        *(pp++) = 7;
                                        break;
                                case 'T':
                                case 't':
                                        *(pp++) = 4;
                                        break;
                                case '[':
                                        if (*request != 0xffffffff) return snapEMsg(text, p, "%%S/%%[...] can be used only once");
                                        double_ptr = 0;
                                        if (*p == '*') { p++; double_ptr = 1; }
                                        if ((*p == 'R' || *p == 'r') && *(p+1) >= '0' && *(p+1) <= '9') {
                                                p++;
                                                n = strtoul(p, &v, 10);
                                                if (v == p || n > 15) return snapEMsg(text, p, "Register name expected");
                                                if (n == 15) return snapEMsg(text, p, "Value of PC not accessible, use direct address");
                                                nr = n;
                                                p = v;
                                                cc = *(p++);
                                                offset = 0;
                                                if (cc == '+' || cc == '-') {
                                                        offset = strtoul(p, &v, 16);
                                                        if (v == p) return snapEMsg(text, p, "Offset value expected");
                                                        if (offset > 510) return snapEMsg(text, p, "Offset should be in range -510..+510");
                                                        if (cc == '-') offset = -offset;
                                                        p = v;
                                                        cc = *(p++);
                                                }
                                                *request = (n << 8) | ((offset >> 2) & 0xff);
                                                dif = offset & 3;
                                                if (double_ptr) { *request |= 0x1000; dif = 0; }
                                        } else if (isxdigit(*p)) {
                                                nr = 0xff;
                                                n = strtoul(p, &v, 16);
                                                if (n < 0x1000) return snapEMsg(text, p, "Address should be above 0x2000");
                                                if (double_ptr) return snapEMsg(text, p, "Double pointers work only with registers");
                                                *request = n & ~3;
                                                dif = n & 3;
                                                p = v;
                                                cc = *(p++);
                                        } else {
                                                return snapEMsg(text, p, "Address or register name expected");
                                        }
                                        *(pp++) = hx?5:6;
                                        *(pp++) = 0x30 + nr;
                                        if (c != '@' && c != '&') {
                                                if (cc != ':') return snapEMsg(text, p, "':' expected");
                                                cc = *(p++);
                                        } else {
                                                cc = (c == '&') ? 'S' : 'Z';
                                                p--;
                                        }
                                        switch (cc) {
                                                case 'B':
                                                case 'b':
                                                        *(pp++) = 0x30;
                                                        break;
                                                case 'W':
                                                case 'w':
                                                        *(pp++) = 0x31;
                                                        break;
                                                case 'S':
                                                case 's':
                                                        *(pp++) = 0x32;
                                                        break;
                                                case 'U':
                                                case 'u':
                                                        *(pp++) = 0x33;
                                                        break;
                                                case 'E':
                                                case 'e':
                                                        *(pp++) = 0x2f;
                                                        break;
                                                case 'Z':
                                                case 'z':
                                                        *(pp++) = 0x30 + 64;
                                                        break;
                                                default:
                                                        p--;
                                                        if (! isdigit(cc)) return snapEMsg(text, p, "Unknown output format\n(expected B, W, S, U or 4..64");
                                                        n = strtoul(p, &v, 10);
                                                        if (n < 4 || n > 64) return snapEMsg(text, p, "Unknown output format\n(expected B, W, S, U or 4..64");
                                                        if (n + dif > 64) n = 64 - dif;
                                                        p = v;
                                                        *(pp++) = 0x30 + n;
                                                        break;
                                        }
                                        *(pp++) = 0x30 + dif;
                                        if (c != '@' && c != '&') {
                                                cc = *(p++);
                                                if (cc != ']') return snapEMsg(text, p, "']' expected");
                                        }
                                        break;

                                default:
                                        return snapEMsg(text, p, "Unknown format character");
                        }
                } else {
                        *(pp++) = c;
                }
        }
        *(pp++) = 0;

        return true;

}

bool compileCondition(char *text, unsigned int *data) {

        char *p, *v;
        unsigned int n, off, U, B;

        p = text;

        while (*p != 0 && *p <= 0x20) p++;

        if (*p == 0) {
                data[0] = 0xe28ef004; // add pc, lr, #4
                return true;
        }

        if (*p == 'R' || *p == 'r') {

                // register
                p++;
                n = strtoul(p, &v, 10);
                if (v == p || n > 15) {
                        return WarningMessage("Parse error in condition: register number expected");
                }
                if (n > 12) {
                        return WarningMessage("Sorry, only R0-R12 allowed in condition");
                }
                p = v;

                data[0] = 0xe1a00000 + n;

        } else if (*p == '[') {

                // [register]
                p++;
                while (*p != 0 && *p <= 0x20) p++;
                if (*p != 'R' && *p != 'r') {
                        return WarningMessage("Parse error in condition: R0..R12 expected");
                }
                p++;
                n = strtoul(p, &v, 10);
                if (v == p || n > 15) {
                        return WarningMessage("Parse error in condition: register number expected");
                }
                if (n > 12) {
                        return WarningMessage("Sorry, only R0-R12 allowed in condition");
                }
                p = v;

                while (*p != 0 && *p <= 0x20) p++;
                U = 1;
                off = 0;
                if (*p == '+' || *p == '-') {
                        U = (*p == '+') ? 1 : 0;
                        p++;
                        off = strtoul(p, &v, 16);
                        if (v == p) {
                                return WarningMessage("Parse error in condition: offset value expected");
                        }
                        if (off > 4094) {
                                return WarningMessage("Parse error in condition: offset should be in range -4094..+4094");
                        }
                        p = v;
                }
                while (*p != 0 && *p <= 0x20) p++;
                B = 0;
                if (*p == ':') {
                        p++;
                        if (*p != 'b' && *p != 'B') {
                                return WarningMessage("Parse error in condition");
                        }
                        p++;
                        B = 1;
                }
                while (*p != 0 && *p <= 0x20) p++;
                if (*p != ']') {
                        return WarningMessage("Parse error in condition");
                }
                p++;

                data[0] = 0xe5100000 | (n << 16) | (U << 23) | (B << 22) | off;

        } else {
                return WarningMessage("Parse error in condition");
        }

        data[1] = 0xe59f1008; // ldr r1, [pc,#8]
        data[2] = 0xe1500001; // cmp r0, r1

        while (*p != 0 && *p <= 0x20) p++;

        if (strncmp(p, "==", 2) == 0) {
                data[3] = 0x028ef004;
                p += 2;
        } else if (strncmp(p, "!=", 2) == 0) {
                data[3] = 0x128ef004;
                p += 2;
        } else if (strncmp(p, ">=", 2) == 0) {
                data[3] = 0x228ef004;
                p += 2;
        } else if (strncmp(p, "<=", 2) == 0) {
                data[3] = 0x928ef004;
                p += 2;
        } else if (*p == '>') {
                data[3] = 0x828ef004;
                p += 1;
        } else if (*p == '<') {
                data[3] = 0x328ef004;
                p += 1;
        } else {
                return WarningMessage("Parse error in condition");
        }

        data[4] = 0xe12fff1e; // bx lr

        while (*p != 0 && *p <= 0x20) p++;

        if (! isdigit(*p)) return WarningMessage("Right side should be constant value");
        data[5] = strtoul(p, &v, 0);
        p = v;

        while (*p != 0 && *p <= 0x20) p++;

        if (*p != 0) {
                return WarningMessage("Parse error in condition");
        }

        return true;

}

bool canSetSnap(unsigned int ins, bool is_thumb) {

        unsigned int cond, cls, opcode, opc;
        unsigned int Rn, Rd, Rm, S, b4, b5, b7, b8, b11, b15, b24;
        unsigned int cls3, cls4, cls5, cls6, cls7;
        bool ok;

        b4 = (ins >> 4) & 1;
        b5 = (ins >> 5) & 1;
        b7 = (ins >> 7) & 1;
        b8 = (ins >> 8) & 1;
        b11 = (ins >> 11) & 1;
        b15 = (ins >> 15) & 1;
        b24 = (ins >> 24) & 1;
        ok = true;
        if (! is_thumb) {
                cond = (ins >> 28) & 0xf;
                cls = (ins >> 25) & 0x7;
                opcode = (ins >> 21) & 0xf;
                Rn = (ins >> 16) & 0xf;
                Rd = (ins >> 12) & 0xf;
                Rm = ins & 0xf;
                S = (ins >> 20) & 1;

                // data processing + shift
                if (cls == 0 && !(((opcode & 0xc) == 0x8) && (S == 0)) && (Rn == 15 || Rm == 15 || Rd == 15)) ok = false;
                // data processing immediate
                if (cls == 1 && (Rn == 15 || Rd == 15)) ok = false;
                // load/store immediate offset
                if (cls == 2 && Rn == 15) ok = false;
                // exception - ldr rn, =...
                if ((ins & 0x0f7f0000) == 0x051f0000) ok = false;
                // load/store halfword/byte
                if (cls == 0 && b4 == 1 && b7 == 1 && Rn == 15) ok = false;
                // load/store register offset
                if (cls == 3 && (Rn == 15 || Rm == 15)) ok = false;
                // load/store multiple
                /* if (cls == 4 && b15 == 1) ok = false; */
                // branch
                /* if (cls == 5 && cond != 0xf && b24 == 0) ok = false; */
                // conditional branch-with-link
                if (cls == 5 && cond != 0xf && cond != 0xe && b24 == 1) ok = false;
                // misc branch/exchange
                /* if (cls == 0 && opcode == 9 && S == 0 && b4 == 1 && b5 == 0) ok = false; */
        } else {
                cls3 = (ins >> 13) & 0x7;
                cls4 = (ins >> 12) & 0xf;
                cls5 = (ins >> 11) & 0x1f;
                cls6 = (ins >> 10) & 0x3f;
                cls7 = (ins >> 9) & 0x7f;
                opc = (ins >> 8) & 0x3;
                Rn = (ins >> 3) & 0x7;
                Rd = ins & 0x7;

                // special data processing
                if (cls6 == 0x11 && opc != 0x3 && (Rn == 7 || Rd == 7)) ok = false;
                // branch/exchange
                /* if (cls6 == 0x11 && opc == 0x3 && b7 == 0) ok = false; */
                // load from literal pool
                /* if (cls5 == 0x09) ok = false; */
                // add to PC
                /* if (cls4 == 0xa && b11 == 0) ok = false; */
                // conditional branch
                if (cls4 == 0xd) ok = false;
                // unconditional branch
                /* if (cls5 == 0x1c) ok = false; */
                // pop incl. PC
                /* if (cls7 == 0x5e && b8 == 1) ok = false; */
                // bl/blx suffix
                if (cls5 == 0x1d || cls5 == 0x1f) ok = false;
        }

        return ok;
}

bool addSnap(unsigned int addr, bool is_thumb, char *text, char *condition, int position) {

        char buf[4096];
        int sn, n, width;
        unsigned int ins0 = 0xe1200070;
        unsigned int request = 0xffffffff;
        unsigned int snapaddr, snapinfo[12], snapmask, ins, newins, imm24, target, b23, b24, cond, cls5;
        bool isbreak;

        VD("addSnap(addr=%08X, thumb=%i, \"%s\", \"%s\", pos=%i) {", addr, is_thumb, text, condition, position);

        if (position == SP_NEW) {
                sn = freeSnap();
                if (sn == -1) {
                        VD("} addsnap (no more snaps)");
                        return WarningMessage("Cannot set more than 15 snappoints");
                }
        } else {
                sn = position;
        }

        if (! compileSnap(addr, text, buf, &request)) {
                VD("} addsnap (compileSnap failed)");
                return false;
        }

        if (! compileCondition(condition, &snapinfo[6])) {
                VD("} addsnap (compileCondition failed)");
                return false;
        }

        snapaddr = SnapDataAddr + sn * 48;
        snapinfo[4] = request;
        readMemory(addr, (unsigned char *)&ins, 4);
        if (is_thumb) {
                cls5 = (ins >> 11) & 0x1f;
                if (cls5 == 0x1e) {
                        // bl/blx
                        width = 4;
                        snapinfo[0] = ((ins >> 16) & 0xffff) | (0x4778 << 16);
                        snapinfo[1] = 0xe59ff000;
                        snapinfo[3] = addr+4+1;
                        newins = (ins & 0xffff) | ((0xbe00 + sn) << 16);
                } else if (cls5 == 0x14) {
                        // add rn, pc, #...
                        width = 2;
                        target = (addr & ~3) + 4 + ((ins & 0xff) << 2);
                        snapinfo[0] = (0x4801 | (ins & 0x0700)) | (0x4778 << 16);
                        snapinfo[1] = 0xe59ff000;
                        snapinfo[2] = target;
                        snapinfo[3] = addr+2+1;
                        newins = (0xbe00 + sn) | (ins & 0xffff0000);
                } else if (cls5 == 0x09) {
                        // load from literal pool
                        width = 2;
                        target = (addr & ~3) + 4 + ((ins & 0xff) << 2);
                        snapinfo[0] = (0x4801 | (ins & 0x0700)) | (0x4778 << 16);
                        snapinfo[1] = 0xe59ff000;
                        readMemory(target, (unsigned char *)&snapinfo[2], 4);
                        snapinfo[3] = addr+2+1;
                        newins = (0xbe00 + sn) | (ins & 0xffff0000);
                } else if (cls5 == 0x1c) {
                        // unconditional branch
                        width = 2;
                        target = ins & 0x7ff;
                        if ((target & 0x400) != 0) target |= 0xfffff800;
                        target = addr + 4 + (target << 1) + 1;
                        snapinfo[0] = 0x47784778;
                        snapinfo[1] = 0xe59ff000;
                        snapinfo[3] = target;
                        newins = (0xbe00 + sn) | (ins & 0xffff0000);
                } else {
                        // any other instruction
                        width = 2;
                        snapinfo[0] = (ins & 0xffff) | (0x4778 << 16);
                        snapinfo[1] = 0xe59ff000;
                        snapinfo[3] = addr+2+1;
                        newins = (0xbe00 + sn) | (ins & 0xffff0000);
                }
        } else {
                if (((ins >> 25) & 0x7) == 0x5) {

                        cond = (ins >> 28) & 0xf;
                        b24 = (ins >> 24) & 1;
                        imm24 = ins & 0xffffff;
                        if (imm24 & 0x800000) imm24 |= 0xff000000;
                        target = addr + 8 + (imm24 << 2);
                        if (cond == 0xf) target = target + (b24 << 1) + 1;

                        if ((cond != 0xf && b24 == 1) || cond == 0xf) {
                                // bl or blx
                                snapinfo[0] = 0xe59fe000;
                                snapinfo[1] = 0xe59ff000;
                                snapinfo[2] = addr+4;
                                snapinfo[3] = target;
                        } else {
                                // bxx
                                snapinfo[0] = 0x059ff000 | (cond << 28);
                                snapinfo[1] = 0xe59ff000;
                                snapinfo[2] = target;
                                snapinfo[3] = addr+4;
                        }
                } else if ((ins & 0x0f7f0000) == 0x051f0000) {
                        // ldr rn, =...
                        b23 = (ins >> 23) & 1;
                        target = ins & 0xfff;
                        target = addr + 8 + ((b23 == 0) ? 0-target : target);
                        snapinfo[0] = 0x059f0000 | (ins & 0xf000f000);
                        snapinfo[1] = 0xe59ff000;
                        readMemory(target, (unsigned char *)&snapinfo[2], 4);
                        snapinfo[3] = addr+4;
                } else {
                        snapinfo[0] = ins;
                        snapinfo[1] = 0xe51ff004;
                        snapinfo[2] = addr+4;
                }
                width = 4;
                newins = (0xe1200070 + sn);
        }

        readMemory(InternalCodeAddr+IC_SNAPMASK*4, (unsigned char *)&snapmask, 4);
        if (text[0] == '~') {
                isbreak = true;
                snapmask |= (1 << sn);
        } else {
                isbreak = false;
                snapmask &= ~(1 << sn);
        }
        snapmask |= 1;
        if (! setMemory(InternalCodeAddr+IC_SNAPMASK*4, (unsigned char *)&snapmask, 4, SM_INTERNAL)) {
                VD("} addSnap (setMemory1 failed)");
                return WarningMessage("Failed to install snappoint");
        }

        if (! setMemory(snapaddr, (unsigned char *)snapinfo, 48, SM_INTERNAL)) {
                VD("} addSnap (setMemory2 failed)");
                return WarningMessage("Failed to install snappoint");
        }
        service(IC_IMB, *snapinfo, snapaddr);
        if (! setMemory(addr, (unsigned char *)&newins, 4, SM_BREAKPOINT)) {
                VD("} addSnap (setMemory3 failed)");
                return WarningMessage("Failed to install snappoint");
        }

        if (position == SP_NEW) {
                SN[sn] = (snap *)malloc(sizeof(snap));
                SN[sn]->num = sn;
                SN[sn]->addr = addr;
                SN[sn]->width = width;
                SN[sn]->thumb = is_thumb;
                SN[sn]->text = strdup(text);
                SN[sn]->condition = strdup(condition);
                SN[sn]->instruction = ins;
                SN[sn]->active = true;
                SN[sn]->isbreak = isbreak;
                SN[sn]->compiled = strdup(buf);
                SN[sn]->request = request;
        }

        VD("} addSnap");
        return true;

}

bool deleteSnap(int num) {

        VD("deleteSnap(%i) {", num);

        if (SN[num] == NULL) {
                VD("} deleteSnap (no such snap)");
                return true;
        }

        if (is_online()) {
                if (! setMemory(SN[num]->addr, (unsigned char *)&(SN[num]->instruction), SN[num]->width, SM_BREAKPOINT)) {
                        WarningMessage("Cannot remove snappoint %i", num);
                }
        }

        free(SN[num]->text);
        free(SN[num]->condition);
        free(SN[num]->compiled);
        free(SN[num]);
        SN[num] = NULL;

        VD("} deleteSnap");
        return true;

}

void deleteSnapAt(unsigned int addr) {

        snap *sn;

        sn = snapAtAddress(addr);
        if (sn != NULL) deleteSnap(sn->num);

}

snap *snapAtAddress(unsigned int addr) {

        int i;

        for (i=1; i<16; i++) {
                if (SN[i] != NULL && SN[i]->addr == addr) {
                        return SN[i];
                }
        }
        return NULL;

}

snap **getSnapList() { return SN; }

// -------------------------------------------------------------------

void emptyWatchpoint() {

        VD("emptyWatchpoint();");
        WatchpointAddr = 0;
        WatchpointLen = 0;

}

bool setWatchpoint(unsigned int as, unsigned int ae) {

        unsigned int buf[3];

        VD("setWatchpoint(as=%08X, ae=%08X) {", as, ae);

        buf[0] = as >> 12;
        buf[1] = as & ~0x3;
        buf[2] = ae & ~0x3;

        if (WatchpointAddr != 0)
                removeWatchpoint();

        if (! setMemory(InternalCodeAddr + IC_WATCHPAGE*4, (unsigned char *)buf, 12, SM_INTERNAL)) {
                VD("} setWatchpoint (setMemory failed)");
                return WarningMessage("Cannot set watchpoint");
        }

        // lockPageUser
        if (! lockPage(as & ~0xfff)) {
                VD("} setWatchpoint (lockPage failed)");
                return WarningMessage("Cannot set lock on page");
        }

        WatchpointAddr = as;
        WatchpointLen = ae - as;
        VD("} setWatchpoint");
        return true;

}

void removeWatchpoint() {

        VD("removeWatchpoint() {");

        if (WatchpointAddr != 0) {
                if (is_online()) {
                        unlockPage(WatchpointAddr & ~0xfff);
                }
                WatchpointAddr = 0;
                WatchpointLen = 0;
        }
        VD("} removeWatchpoint");

}

// -------------------------------------------------------------------

void setUsed(unsigned int addr) {

        unsigned int off = ((addr - ExtRAMaddress) / 4096) / 8;
        unsigned char mask = 1 << (((addr - ExtRAMaddress) / 4096) & 0x7);
        RelBits[off] |= mask;
        setMemory(RelBitsAddr+off, RelBits+off, 1, SM_INTERNAL);

}

bool isUsed(unsigned int addr) {

        unsigned int off = ((addr - ExtRAMaddress) / 4096) / 8;
        unsigned char mask = 1 << (((addr - ExtRAMaddress) / 4096) & 0x7);

        return RelBits[off] & mask;

}

bool readMemory(unsigned int addr, unsigned char *buf, int len) {

        unsigned int sa, ea;
        unsigned char *xbuf;
        bool r;

        VD("readMemory(addr=%08X, len=%08X) {", addr, len);

        sa = addr & ~0x3;
        ea = (addr+len-1) & ~0x3;

        if ((addr & 0x3) != 0 || ((addr + len) & 0x3) != 0) {

                xbuf = (unsigned char *)malloc(len+8);
                if (readFromRAM(sa, xbuf, (ea-sa)+4)) {
                        memcpy(buf, xbuf+(addr-sa), len);
                        r = true;
                } else {
                        r = false;
                }
                free(xbuf);

        } else {

                r = readFromRAM(addr, buf, len);

        }

        VD("} readMemory = %i", r);
        return r;

}

bool setMemoryAligned(unsigned int addr, unsigned char *buf, int len, int stype) {

        unsigned int sa, ea, tlen;
        unsigned char *tbuf;
        int mtype = M_UNKNOWN;

        for (int i=0; mem_type[i]!=0; i++) {
                if (mem_addr[i] <= addr && mem_addr[i]+mem_len[i]-1 >= addr) {
                        mtype = mem_type[i];
                        break;
                }
        }

        if (mtype == M_RAM || mtype == M_IO || (mtype == M_UNKNOWN && stype == SM_INTERNAL)) {

                return writeToRAM(addr, buf, len);

        } else if (mtype == M_FLASH) {

                if (stype != SM_ISOLATED && stype != SM_STICKY && stype != SM_REINSTALL && stype != SM_UNDO && stype != SM_BREAKPOINT) {
                        WarningMessage("Cannot write into flash area");
                        return true;
                }

                sa = addr & ~0xfff;
                ea = addr+len-1;
                while (sa <= ea) {
                        relocatePage(sa);
                        sa += 4096;
                }

                return writeToRAM(addr, buf, len);

        } else {
                WarningMessage("BootROM is not writable");
                return true;
        }

}


bool setMemory(unsigned int addr, unsigned char *buf, int len, int stype) {

        unsigned int sa, ea;
        unsigned char *xbuf, *obuf;
        bool r;

        VD("setMemory(addr=%08X, len=%08X, stype=%i) {", addr, len, stype);

        xbuf = (unsigned char *)malloc(len);
        memcpy(xbuf, buf, len);

        if ((! ExtraFeatures) &&(addr >= FlashAddress) && (addr <= FlashAddress+FlashSize-1)) {
                VD("} setMemory (extra features not available)");
                return WarningMessage("Relocation pool has no entries");
        }

        if ((stype == SM_ISOLATED || stype == SM_STICKY) && (addr >= FlashAddress) && (addr <= FlashAddress+FlashSize-1)) {
                obuf = (unsigned char *)malloc(len);
                if (! readMemory(addr, obuf, len)) {
                        free(obuf);
                        free(xbuf);
                        VD("} setMemory (readMemory failed)");
                        return false;
                }
                addPatch(addr, obuf, buf, len, stype);
                free(obuf);
        }

        sa = addr & ~0x3;
        ea = (addr+len-1) & ~0x3;

        if ((addr & 0x3) != 0 || ((addr + len) & 0x3) != 0) {

                xbuf = (unsigned char *)malloc(len+8);
                readFromRAM(sa, xbuf, 4);
                readFromRAM(ea, xbuf+(ea-sa), 4);

                memcpy(xbuf+(addr-sa), buf, len);

        }

        r = setMemoryAligned(sa, xbuf, (ea-sa)+4, stype);
        free(xbuf);

        VD("} setMemory = %i", r);
        return r;

}

bool undoPatch(unsigned int addr) {

        patch *p;
        bool r;

        VD("undoPatch(addr=%08X) {", addr);

        p = getPatchByAddress(addr);
        r = true;
        if (p != NULL) {
                if (is_online() && ! p->disabled) {
                        if (! setMemory(p->address, p->olddata, p->length, SM_UNDO))
                                r = false;
                }
        }

        deletePatch(addr);
        VD("} undoPatch = %i", r);
        return r;

}

bool enablePatch(patch *p) {

        VD("enablePatch(addr=%08X) {", p->address);
        if (p->disabled) {
                setMemory(p->address, p->newdata, p->length, SM_REINSTALL);
               p->disabled = false;
        }
        VD("} enablePatch");
        return true;

}

bool disablePatch(patch *p) {

        VD("disablePatch(addr=%08X) {", p->address);
        if (! p->disabled) {
                setMemory(p->address, p->olddata, p->length, SM_UNDO);
                p->disabled = true;
        }
        VD("} disablePatch");
        return true;

}

bool execute(unsigned char *code, int len, unsigned int r[16], int count, unsigned int &cpsr, int timeout) {

        bool res;

        VD("execute(len=%i, r0=%08X, r1=%08X, timeout=%i) {", len, r[0], r[1], timeout);

        res = setMemory(TempExecAddr, code, len, SM_INTERNAL);
        if (res) res = armCall(TempExecAddr, r, count, cpsr, timeout);

        VD("} execute = %s", res ? "true" : "false");
        return res;

}

bool execute2(unsigned char *code, int len, unsigned int r[16], int count, unsigned int &cpsr, int timeout) {

        bool res;

        VD("execute2(len=%i, r0=%08X, r1=%08X, timeout=%i) {", len, r[0], r[1], timeout);

        res = setMemory(TempExecAddr, code, len, SM_INTERNAL);
        if (res) res = armCall(InternalCodeAddr+IC_EXEC*4, r, count, cpsr, timeout);

        VD("} execute2 = %s", res ? "true" : "false");
        return res;

}

bool startTrace(unsigned char *code, int len, unsigned int r[16], unsigned int &cpsr) {

        unsigned int ld, ald;
        unsigned int r2[16], cpsr2;
        int i;

                if (! execute2(arm_getsplr, sizeof(arm_getsplr), r2, 0, cpsr2, 250)) return false;
                r[13] = r2[0];
                r[14] = r2[1];
                ald = InternalCodeAddr + IC_LOGDATA*4;
                if (! setMemory(TempExecAddr, code, len, SM_INTERNAL)) return false;
                if (! readMemory(ald, (unsigned char *)&ld, 4)) return false;
                ld &= ~0x0000ff00;
                ld |=  0x00000200;
                if (! setMemory(ald, (unsigned char *)&ld, 4, SM_INTERNAL)) return false;
                for (i=0; i<15; i++) CurrentRs[i] = r[i];
                for (i=0; i<15; i++) CurrentStack[i] = 0;
                CurrentCPSR = cpsr;
                CurrentAddr = CurrentRs[15] = TempExecAddr;
                CurrentThumb = false;
                setSystemState(SS_EXTBRK);
                setRegisters(CurrentRs, CurrentCPSR, 16, NULL);
                cpSetAddress(TempExecAddr, true);
                cpu_reload();
                return true;

}

unsigned int service2(unsigned int num, unsigned int r0, unsigned int r1, unsigned int r2, unsigned int r3, unsigned int r4, int timeout) {

        unsigned int r[16], cpsr, a;

        a = InternalCodeAddr + num * 4;
        VD("service(%i(%08X), r0=%08X, r1=%08X, r2=%08X, r3=%08X, r4=%08X, t=%i) {", num, a, r0, r1, r2, r3, r4, timeout);
        r[0] = r0;
        r[1] = r1;
        r[2] = r2;
        r[3] = r3;
        r[4] = r4;

        if (! armCall(a, r, 5, cpsr, timeout)) r[0] = 0xffffffff;
        VD("} service = %08X", r[0]);
        return r[0];

}

unsigned int service(unsigned int num, unsigned int r0, unsigned int r1) {

        unsigned int r[16], cpsr, a;

        a = InternalCodeAddr + num * 4;
        VD("service(%i(%08X), r0=%08X, r1=%08X) {", num, a, r0, r1);
        r[0] = r0;
        r[1] = r1;

        if (! armCall(a, r, 2, cpsr, 250)) r[0] = 0xffffffff;
        VD("} service = %08X", r[0]);
        return r[0];

}

unsigned int phone_off() {

        return service2(IC_FASTOFF, 0, 0, 0, 0, 0, 1);

}

void write4(unsigned char *addr, unsigned int v) {

        addr[0] = (v & 0xff);
        addr[1] = ((v >> 8) & 0xff);
        addr[2] = ((v >> 16) & 0xff);
        addr[3] = ((v >> 24) & 0xff);

}

bool copyData(unsigned int dest, unsigned int src, unsigned int len) {

        unsigned int r1, r2, r3;

        r1 = src;
        r2 = dest;
        r3 = len;
        service2(IC_COPY, 0, r1, r2, r3, 0, 250);

        return true;

}

unsigned int searchMemory(unsigned char *what, unsigned char *mask, int n, unsigned int addr_s, unsigned int addr_e) {

        unsigned int r;
        unsigned char buf[4096];
        unsigned int n4;

        VD("searchMemory(%i bytes, s=%08X, e=%08X) {", n, addr_s, addr_e);

        n4 = (n + 3) & ~0x3;
        memcpy(buf, what, n);
        memcpy(buf+n4, mask, n);

        if (! writeToRAM(TempExecAddr, buf, n4*2)) {
                r = 0xffffffff;
        } else {
                r = service2(IC_SEARCH, TempExecAddr, addr_s, addr_e, n, TempExecAddr+n4, 20000);
        }

        VD("} searchMemory = %08X", r);
        return r;

}

unsigned int searchSignature(unsigned char *sig, unsigned char *mask,
                                int len, int offset,
                                unsigned int startAddress, unsigned int endAddress)
{
        unsigned int *icode, r[16], cpsr, ca, cl;

        ca = startAddress;
        while (ca < endAddress) {
                cl = 1000000;
                if (endAddress-ca < cl) cl = endAddress-ca;
                icode = (unsigned int *) arm_signature;
                icode[2] = ca;
                icode[3] = ca+cl;
                icode[4] = len;
                icode[5] = offset;
                memcpy(arm_signature+24, sig, len);
                memcpy(arm_signature+24+len, mask, len);

                if (! writeToRAM(LoadableCodeAddr, arm_signature, sizeof(arm_signature))) return NONE;
                if (! armCall(LoadableCodeAddr, r, 0, cpsr, 5000)) return NONE;
                if (r[0] != 0) return r[0];

                ca += cl;
        }

        return NONE;

}

void getMemoryInfo(unsigned int addr, struct memoryinfo *mi) {

        for (int i=0; mem_type[i]!=0; i++) {
                if (mem_addr[i] <= addr && mem_addr[i]+mem_len[i]-1 >= addr) {
                        mi->addr = mem_addr[i];
                        mi->len = mem_len[i];
                        mi->type = mem_type[i];
                        return;
                }
        }
        mi->addr = mi->len = mi->type = 0;
        return;
}

bool inSpace(unsigned int addr) {

        for (int i=0; mem_type[i]!=0; i++) {
                if (mem_addr[i] <= addr && mem_addr[i]+mem_len[i]-1 >= addr) {
                        return true;
                }
        }
        return false;

}

unsigned int getTempAddress() {
        return TempExecAddr;
}

int getFreePoolEntries() {
        return PoolFree;
}

bool executeInContext(unsigned int code[8], unsigned int r[16], unsigned int &cpsr, int timeout) {

        unsigned int xcode[32];
        int i;

        if (SystemState == SS_INTBRK) {
                SubsAddress = HolderAddr + 0x1c;
                return writeToRAM(HolderAddr, (unsigned char *)code, 32);
                // maybe IMB should be here
        } else {
                xcode[0x00] = 0xea00000f;       // b start
                for (i=0; i<15; i++) {
                        xcode[i+1] = CurrentRs[i];
                }
                xcode[0x10] = CurrentCPSR;
                xcode[0x11] = 0xe51f000c;       // ldr r0, savecpsr
                xcode[0x12] = 0xe128f000;       // msr cpsr_f, r0
                xcode[0x13] = 0xe24fe050;       // adr lr, registers
                xcode[0x14] = 0xe89e7fff;       // ldmia lr, {r0-lr}
                xcode[0x15] = code[0];
                xcode[0x16] = code[1];
                xcode[0x17] = code[2];
                xcode[0x18] = code[3];
                xcode[0x19] = code[4];
                xcode[0x1a] = code[5];
                xcode[0x1b] = code[6];
                xcode[0x1c] = code[7];

                SubsAddress = LoadableCodeAddr + 0x1c * 4;
                if (! writeToRAM(LoadableCodeAddr, (unsigned char *)xcode, sizeof(xcode))) return false;
                if (! armCall(LoadableCodeAddr, r, 0, cpsr, timeout)) return false;
                return true;
        }

}

bool updateRegister(int num, unsigned int value) {

        unsigned int code[4];
        unsigned int r[16], cpsr;

        if (num == 15) {
                CurrentRs[15] = CurrentAddr = value & ~1;
                CurrentThumb = ((value & 1) != 0);
                setRegisters(CurrentRs, CurrentCPSR, 16, CurrentStack);
                MainForm->vCPU->Repaint();
        } else {
                Break0addr = CurrentAddr;
                Break0thumb = CurrentThumb;
                code[0] = 0xe59f0000 | (num << 12);
                code[1] = 0xe1200070;
                code[2] = value;
                if (! executeInContext(code, r, cpsr, 250)) return false;
                eventMonitor(true);
       }
       return true;
}

bool updateCPSR(unsigned int value) {

        unsigned int code[4];
        unsigned int r[16], cpsr;

//        if (SystemState == SS_INTBRK) {
                Break0addr = CurrentAddr;
                Break0thumb = CurrentThumb;
                code[0] = 0xe328f400 | ((value >> 24) & 0xff);
                code[1] = 0xe1200070;
                executeInContext(code, r, cpsr, 250);
                eventMonitor(true);
//        } else {
//                CurrentCPSR = value;
//                setRegisters(CurrentRs, CurrentCPSR, 16);
//        }
}

void makesubscode(unsigned int addr, unsigned int ins, int reg, unsigned int code[8]) {

        code[0] = 0xe58f0010 | (reg << 12);
        code[1] = 0xe59f0010 | (reg << 12);
        code[2] = ins;
        code[3] = 0xe58f0008 | (reg << 12);
        code[4] = 0xe59f0000 | (reg << 12);
        code[5] = 0xe1200070;
        code[6] = 0;
        code[7] = addr;

}

void makeblx(unsigned int addr, unsigned int target, unsigned int cond, bool into, unsigned int code[8]) {

        if (! into) {
                // bl F8
                code[0] = 0x01a0e00f | (cond << 28);
                code[1] = 0x059ff000 | (cond << 28);
                code[2] = 0xe1200070; // bkpt 0
                code[3] = target;
                ReloadQuery = true;
        } else {
                // bl F7
                code[0] = 0x059fe010 | (cond << 28);
                code[1] = 0x058fe010 | (cond << 28);
                code[2] = 0x059fe004 | (cond << 28);
                code[3] = 0xe1200070; // bkpt 0
                code[4] = 0;
                code[5] = addr;
                code[6] = target;
                code[7] = addr;
                SubsR15 = true;
        }
}

void makebx(unsigned int addr, unsigned int target, unsigned int cond, unsigned int code[8]) {

        code[0] = 0xe58f000c;
        code[1] = 0x059f000c | (cond << 28);
        code[2] = 0x058f000c | (cond << 28);
        code[3] = 0xe59f0000;
        code[4] = 0xe1200070; // bkpt 0
        code[5] = 0;
        code[6] = target;
        code[7] = addr;
        SubsR15 = true;
}

void makebxr(unsigned int addr, unsigned int rn, unsigned int cond, unsigned int code[8]) {

        code[0] = 0x058f0014 | (cond << 28) | (rn << 12);
        code[1] = 0xe1200070; // bkpt 0
        code[7] = addr;
        SubsR15 = true;
}

void makeblxr(unsigned int addr, unsigned int rn, unsigned int cond, bool into, unsigned int code[8]) {

        if (! into) {
                // blx F8
                code[0] = 0x012fff30 | (cond << 28) | rn;
                code[1] = 0xe1200070; // bkpt 0
                ReloadQuery = true;
        } else {
                // blx F7
                code[0] = 0x058f0014 | (cond << 28) | (rn << 12);
                code[1] = 0x059fe000 | (cond << 28);
                code[2] = 0xe1200070; // bkpt 0
                code[3] = addr;
                code[7] = addr;
                SubsR15 = true;
        }
}

bool execInstruction(unsigned int addr, unsigned int ins, bool thumb, bool into) {

        unsigned int code[8];
        unsigned int Rn, Rd, Rm, Rs, HRn, HRd, cond, imm24, v, target, ins2, cls5;
        unsigned int b7, b8, b9b8, b11, b14, b15, b20, b24;
        unsigned int r[16], cpsr;
        bool with_rm;
        int i, ic;

        if (! thumb) {

                code[0] = ins;
                code[1] = 0xe1200070; // bkpt 0
                Break0addr = addr + 4;
                Break0thumb = false;
                SubsR15 = false;
                ReloadQuery = false;
                ThumbAware = false;
                with_rm = false;

                Rn = (ins >> 16) & 0x0f;
                Rd = (ins >> 12) & 0x0f;
                Rm = ins & 0x0f;
                Rs = (ins >> 8) & 0x0f;
                b14 = (ins >> 14) & 1;
                b15 = (ins >> 15) & 1;
                b20 = (ins >> 20) & 1;
                b24 = (ins >> 24) & 1;
                cond = (ins >> 28) & 0xf;
                imm24 = ins & 0xffffff;
                if (imm24 & 0x800000) imm24 |= 0xff000000;

                ic = insclass_arm(ins);
                switch (ic) {
                        case ICA_DPROCIS:
                        case ICA_DPROCRS:
                        case ICA_LSR:
                        case ICA_LSHALFR:
                        case ICA_LSDWORDR:
                        case ICA_LSSHBR:
                                with_rm = true;
                                /* passthru */
                        case ICA_LSHALFI:
                        case ICA_LSDWORDI:
                        case ICA_LSSHBI:
                        case ICA_LSI:
                        case ICA_DPROCI:
                                if (Rn == 15 || Rd == 15 || (with_rm && Rm == 15)) {
                                        // register substitution
                                        for (i=0; i<12; i++) {
                                                if (Rn != i && Rd != i && Rm != i && Rs != i) break;
                                        }
                                        if (Rn == 15) ins = (ins & 0xfff0ffff) | (i << 16);
                                        if (Rd == 15) ins = (ins & 0xffff0fff) | (i << 12);
                                        if (with_rm && Rm == 15) ins = (ins & 0xfffffff0) | i;
                                        if (Rd == 15) {
                                                if (cond != 0xe) return WarningMessage("Single-step for this instruction is not supported");
                                                SubsR15 = true;
                                        }
                                        if (ic == ICA_LSI || ic == ICA_LSR) ThumbAware = true;
                                        makesubscode(addr+8, ins, i, code);
                                }
                                break;

                        case ICA_LSM:
                                if (b15 == 1 && b20 == 1) {
                                        if (b14 == 1) return WarningMessage("Single-step for this instruction is not supported");
                                        if (cond != 0xe) return WarningMessage("Single-step for this instruction is not supported");
                                        SubsR15 = true;
                                        ins &= ~0x00008000;
                                        ins |= 0x00004000;
                                        ThumbAware = true;
                                        makesubscode(addr+8, ins, 14, code);
                                }
                                break;

                        case ICA_BBL:
                                target = addr + 8 + (imm24 << 2);
                                if (b24 == 0) {
                                        // b
                                        makebx(addr+4, target, cond, code);
                                        ThumbAware = false;
                                } else {
                                        makeblx(addr+4, target, cond, into, code);
                                        ThumbAware = false;
                                }
                                break;

                        case ICA_BLX:
                                target = (addr + 8 + (imm24 << 2) + (b24 << 1)) | 1;
                                makeblx(addr+4, target, 0xe, into, code);
                                ThumbAware = true;
                                break;

                        case ICA_BXR:
                                makebxr(addr+4, Rm, cond, code);
                                ThumbAware = true;
                                break;

                        case ICA_BLXR:
                                makeblxr(addr+4, Rm, cond, into, code);
                                ThumbAware = true;
                                break;
                }

                if ((ic == ICA_LSI || ic == ICA_LSR || ic == ICA_LSHALFI || ic == ICA_LSHALFR) && b20 == 0) ReloadQuery = true;
                if (ic == ICA_LSM && b20 == 0) ReloadQuery = true;

                return executeInContext(code, r, cpsr, 2000);

        } else {

                code[0] = 0xe58fe010;
                code[1] = 0xe28fe009;
                code[2] = 0xe58fe00c;
                code[3] = 0xe59fe004;
                code[4] = 0xe59ff004;
                code[5] = (ins & 0xffff) | (0xbe00 << 16);
                code[6] = 0;
                code[7] = 0;
                Break0addr = addr + 2;
                Break0thumb = true;
                SubsR15 = false;
                ReloadQuery = false;
                ThumbAware = true;

                Rn = (ins >> 3) & 7;
                HRn = (ins >> 3) & 0xf;
                Rd = ins & 7;
                HRd = ((ins & 0x80) >> 4) | Rd;
                b7 = (ins >> 7) & 1;
                b8 = (ins >> 8) & 1;
                b9b8 = (ins >> 8) & 3;
                b11 = (ins >> 11) & 1;

                ic = insclass_thumb(ins);
                switch (ic) {
                        case ICT_BXR:
                                if (b7 == 0) {
                                        makebxr(addr+3, HRn, 0xe, code);
                                } else {
                                        makeblxr(addr+3, HRn, 0xe, into, code);
                                }
                                ThumbAware = true;
                                break;

                        case ICT_LLP:
                                Rd = (ins >> 8) & 7;
                                target = (addr & ~3) + 4 + (ins & 0xff) * 4;
                                if (! readMemory(target, (unsigned char *)&v, 4)) return false;
                                code[0] = 0xe59f0000 | (Rd << 12);
                                code[1] = 0xe1200070;
                                code[2] = v;
                                break;

                        case ICT_ASPPC:
                                if (b11 == 0) {
                                        Rd = (ins >> 8) & 7;
                                        v = (addr & ~3) + 4 + (ins & 0xff) * 4;
                                        code[0] = 0xe59f0000 | (Rd << 12);
                                        code[1] = 0xe1200070;
                                        code[2] = v;
                                }
                                break;

                        case ICT_CBR:
                                cond = (ins >> 8) & 0xf;
                                target = (ins & 0xff);
                                if ((target & 0x80) != 0) target |= 0xffffff00;
                                target = addr + 4 + (target << 1) + 1;
                                makebx(addr+3, target, cond, code);
                                ThumbAware = true;
                                break;

                        case ICT_UBR:
                                target = (ins & 0x7ff);
                                if ((target & 0x400) != 0) target |= 0xfffff800;
                                target = addr + 4 + (target << 1) + 1;
                                makebx(addr+3, target, 0xe, code);
                                ThumbAware = true;
                                break;

                        case ICT_BLBLXP:
                                Break0addr = addr + 4;
                                ins2 = (ins >> 16) & 0xffff;
                                cls5 = (ins2 >> 11) & 0x1f;
                                target = ins & 0x7ff;
                                if (target & 0x400) target |= 0xfffff800;
                                target = (target << 12) + addr + 4 + ((ins2 & 0x7ff) << 1);
                                if (cls5 == 0x1d) {
                                        target &= ~0x3;
                                } else {
                                        target += 1;
                                        ThumbAware = true;
                                }
                                makeblx(addr+5, target, 0xe, into, code);
                                break;

                        case ICT_PP:
                                if (b11 == 1 && b8 == 1) {
                                        SubsR15 = true;
                                        ins = 0xe8bd4000 | (ins & 0xff);
                                        ThumbAware = true;
                                        makesubscode(addr+4, ins, 14, code);
                                }
                                break;

                        case ICT_SDP:
                                if (HRn == 15 || HRd == 15) {
                                        for (i=0; i<12; i++) {
                                                if (HRn != i && HRd != i) break;
                                        }
                                        if (HRd == 15) SubsR15 = true;
                                        if (HRd == 15) HRd = i;
                                        if (HRn == 15) HRn = i;
                                        if (b9b8 == 2) {
                                                ins = 0xe1a00000 | (HRd << 12) | HRn;
                                        } else if (b9b8 == 0) {
                                                ins = 0xe0800000 | (HRd << 16) | (HRd << 12) | HRn;
                                        } else {
                                                ins = 0xe1500000 | (HRd << 16) | HRn;
                                        }
                                        ThumbAware = false;
                                        makesubscode(addr+4, ins, i, code);
                                }
                                break;

                        case ICT_BLXS:
                        case ICT_BLS:
                        case ICT_UNDEFINED:
                                return WarningMessage("Bad thumb instruction");
                }

                if (ic == ICT_LSR) ReloadQuery = true;
                if ((ic == ICT_LSWBI || ic == ICT_LSHI || ic == ICT_LSST || ic == ICT_LSM) && b11 == 0) ReloadQuery = true;
                if (ic == ICT_PP && b11 == 0) ReloadQuery = true;

                return executeInContext(code, r, cpsr, 2000);
        }
}

void singleStep(bool into) {

        unsigned int a, ins;
        bool thumb;
        snap *sn;
        int i;

        if (SystemState != SS_INTBRK && SystemState != SS_EXTBRK) return;
        a = CurrentAddr;
        thumb = CurrentThumb;

        sn = snapAtAddress(a);
        if (sn != NULL) {
                ins = sn->instruction;
        } else {
                if (! readMemory(a, (unsigned char *)&ins, 4)) {
                        WarningMessage("Cannot read instruction");
                        return;
                }
        }

        if (! execInstruction(a, ins, thumb, into)) return;
        eventMonitor(true);

        cpSetAddress(CurrentThumb ? CurrentAddr|1 : CurrentAddr, true);
        MainForm->Pages->ActivePage = MainForm->SheetCPU;
        MainForm->vCPU->SetFocus();
}

void runFromPoint() {

        char tbuf[128];
        unsigned int code[4];
        unsigned int r[16], cpsr;
        unsigned int a, ins, ald, ld;
        bool thumb;
        int i;
        snap *sn;
        bool res;

        if (SystemState != SS_INTBRK && SystemState != SS_EXTBRK) return;

        sn = snapAtAddress(CurrentAddr);
        if (sn != NULL) {
                execInstruction(CurrentAddr, sn->instruction, CurrentThumb, true);
                eventMonitor(false);
        }

        ald = InternalCodeAddr + IC_LOGDATA*4;
        if (! readMemory(ald, (unsigned char *)&ld, 4)) return;
        ld &= ~0x0000ff00;
        ld |=  0x00000300;
        if (! setMemory(ald, (unsigned char *)&ld, 4, SM_INTERNAL)) return;

        code[0] = 0xe1200070; // bkpt 0
        code[1] = 0;
        code[2] = 0xe51ff004; // ldr pc, [pc,#-4]
        code[3] = CurrentThumb ? CurrentAddr|1 : CurrentAddr;

        if (SystemState == SS_INTBRK) {
                executeInContext(code, r, cpsr, 250);
                setSystemState(SS_READY);
        } else {
                setSystemState(SS_READY);
                res = executeInContext(code, r, cpsr, ExecTimeout);
                if (res) {
                        eventMonitor(true);
                        if (SystemState == SS_READY) {
                                sprintf(tbuf, "Call: R0=%X", r[0]);
                                newEvent('>', tbuf);
                                setRegisters(r, cpsr, 13, NULL);
                                reload_dumps();
                        }
                } else {
                        newEvent('!', "Call failed");
                }
        }
        MainForm->vCPU->Repaint();
}

void stepInto() { singleStep(true); }
void stepOver() { singleStep(false); }

void programReset() {

        unsigned int a, ld;

        if (SystemState == SS_INTBRK) {
                WarningMessage("Cannot reset internal process");
                return;
        } else if (SystemState == SS_EXTBRK) {
                VD("programReset();");
                a = InternalCodeAddr + IC_LOGDATA*4;
                if (! readMemory(a, (unsigned char *)&ld, 4)) return;
                ld &= ~0x0000ff00;
                if (! setMemory(a, (unsigned char *)&ld, 4, SM_INTERNAL)) return;
                setSystemState(SS_READY);
                reload_dumps();
        }
}

