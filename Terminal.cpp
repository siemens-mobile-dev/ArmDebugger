//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "DForm.h"
#include "stdio.h"
#include "comm.h"
#include "Utility.h"
#include "Terminal.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

TTerminalForm *TerminalForm;

static char *profile_bits[] = {

        // 1
        "Profile download",
        "SMS-PP data download",
        "Cell Broadcast data download",
        "Menu selection",
        "'9EXX' response code for SIM data download error",
        "Timer expiration",
        "USSD string data object supported in Call Control",
        "Envelope Call Control always sent to the SIM during automatic redial mode",

        // 2
        "Command result",
        "Call Control by SIM",
        "Cell identity included in Call Control by SIM",
        "MO short message control by SIM",
        "Handling of the alpha identifier",
        "UCS2 Entry supported",
        "UCS2 Display supported",
        "Display of the extension text",

        // 3
        "Proactive SIM: DISPLAY TEXT",
        "Proactive SIM: GET INKEY",
        "Proactive SIM: GET INPUT",
        "Proactive SIM: MORE TIME",
        "Proactive SIM: PLAY TONE",
        "Proactive SIM: POLL INTERVAL",
        "Proactive SIM: POLLING OFF",
        "Proactive SIM: REFRESH",

        // 4
        "Proactive SIM: SELECT ITEM",
        "Proactive SIM: SEND SHORT MESSAGE",
        "Proactive SIM: SEND SS",
        "Proactive SIM: SEND USSD",
        "Proactive SIM: SET UP CALL",
        "Proactive SIM: SET UP MENU",
        "Proactive SIM: PROVIDE LOCAL INFORMATION",
        "Proactive SIM: PROVIDE LOCAL INFORMATION (NMR)",

        // 5
        "Proactive SIM: SET UP EVENT LIST",
        "Event: MT call",
        "Event: Call connected",
        "Event: Call disconnected",
        "Event: Location status",
        "Event: User activity",
        "Event: Idle screen available",
        "Event: Card reader status",

        // 6
        "Event: Language selection",
        "Browser Termination",
        "Data available",
        "Event: Channel status",
        "",
        "",
        "",
        "",

        // 7
        "Proactive SIM: POWER ON CARD",
        "Proactive SIM: POWER OFF CARD",
        "Proactive SIM: PERFORM CARD APDU",
        "Proactive SIM: GET READER STATUS",
        "Proactive SIM: GET READER STATUS (id)",
        "",
        "",
        "",

        // 8
        "Proactive SIM: TIMER MANAGEMENT (start, stop)",
        "Proactive SIM: TIMER MANAGEMENT (get current value)",
        "Proactive SIM: PROVIDE LOCAL INFORMATION (date, time)",
        "Binary choice in GET INKEY",
        "SET UP IDLE MODE TEXT",
        "RUN AT COMMAND",
        "2nd alpha identifier in SET UP CALL",
        "2nd capability configuration parameter",

        // 9
        "Sustained DISPLAY TEXT",
        "SEND DTMF command",
        "Proactive SIM: PROVIDE LOCAL INFORMATION - BCCH Channel List",
        "Proactive SIM: PROVIDE LOCAL INFORMATION (language)",
        "Proactive SIM: PROVIDE LOCAL INFORMATION (TA)",
        "Proactive SIM: LANGUAGE NOTIFICATION",
        "Proactive SIM: LAUNCH BROWSER",

        // 10
        "Soft keys support for SELECT ITEM",
        "Soft Keys support for SET UP MENU",
        "",
        "",
        "",
        "",
        "",
        "",
        "",

        // 11
        "8#Maximum number of soft keys available",

        // 12
        "Proactive SIM: OPEN CHANNEL",
        "Proactive SIM: CLOSE CHANNEL",
        "Proactive SIM: RECEIVE DATA",
        "Proactive SIM: SEND DATA",
        "Proactive SIM: GET CHANNEL STATUS",
        "",
        "",
        "",

        // 13
        "CSD supported by ME",
        "GPRS supported by ME",
        "",
        "",
        "",
        "3#Number of channels supported by ME",

        // 14
        "5#Number of characters supported down the ME display",
        "",
        "",
        "Screen Sizing Parameters supported",

        // 15
        "7#Number of characters supported across the ME display",
        "Variable size fonts Supported",

        // 16
        "Display can be resized",
        "Text Wrapping supported",
        "Text Scrolling supported",
        "",
        "",
        "3#Width reduction when in a menu",

        // 17
        "TCP protocol supported",
        "UDP protocol supported",
        "",
        "",
        "",
        "",
        "",
        "",

};

static struct {
        int cmd;
        char *text;
} commands[] = {
        { 0xd0, "Proactive SIM" },
        { 0xd1, "SMS-PP download" },
        { 0xd2, "Cell broadcast download" },
        { 0xd3, "Menu selection" },
        { 0xd4, "Call control" },
        { 0xd5, "MO short message control" },
        { 0xd6, "Event download" },
        { 0xd7, "Timer expiration" },
        { -1, "" }
};

static struct {
        int cmd;
        char *text;
} types_of_command[] = {
        { 0x01, "REFRESH" },
        { 0x02, "MORE TIME" },
        { 0x03, "POLL INTERVAL" },
        { 0x04, "POLLING OFF" },
        { 0x05, "SET UP EVENT LIST" },
        { 0x10, "SET UP CALL" },
        { 0x11, "SEND SS" },
        { 0x12, "SEND USSD" },
        { 0x13, "SEND SHORT MESSAGE" },
        { 0x14, "SEND DTMF" },
        { 0x15, "LAUNCH BROWSER" },
        { 0x20, "PLAY TONE" },
        { 0x21, "DISPLAY TEXT" },
        { 0x22, "GET INKEY" },
        { 0x23, "GET INPUT" },
        { 0x24, "SELECT ITEM" },
        { 0x25, "SET UP MENU" },
        { 0x26, "PROVIDE LOCAL INFORMATION" },
        { 0x27, "TIMER MANAGEMENT" },
        { 0x28, "SET UP IDLE MODE TEXT" },
        { 0x30, "PERFORM CARD APDU" },
        { 0x31, "POWER ON CARD" },
        { 0x32, "POWER OFF CARD" },
        { 0x33, "GET READER STATUS" },
        { 0x34, "RUN AT COMMAND" },
        { 0x35, "LANGUAGE NOTIFICATION" },
        { 0x40, "OPEN CHANNEL" },
        { 0x41, "CLOSE CHANNEL" },
        { 0x42, "RECEIVE DATA" },
        { 0x43, "SEND DATA" },
        { 0x44, "GET CHANNEL STATUS" },
        { 0x81, "End of proactive session" },
        { -1, "" }
};

static struct {
        int tag;
        char *text;
} simple_tlv_list[] = {
        { 0x01, "Command details" },
        { 0x02, "Device identity" },
        { 0x03, "Result" },
        { 0x04, "Duration" },
        { 0x05, "Alpha identifier" },
        { 0x06, "Address" },
        { 0x07, "Capability configuration parameters" },
        { 0x08, "Called party subaddress" },
        { 0x09, "SS string" },
        { 0x0a, "USSD string" },
        { 0x0b, "SMS TPDU" },
        { 0x0c, "Cell Broadcast page" },
        { 0x0d, "Text string" },
        { 0x0e, "Tone" },
        { 0x0f, "Item" },
        { 0x10, "Item identifier" },
        { 0x11, "Response length" },
        { 0x12, "File List" },
        { 0x13, "Location Information" },
        { 0x14, "IMEI" },
        { 0x15, "Help request" },
        { 0x16, "Network Measurement Results" },
        { 0x17, "Default Text" },
        { 0x18, "Items Next Action Indicator" },
        { 0x19, "Event list" },
        { 0x1a, "Cause" },
        { 0x1b, "Location status" },
        { 0x1c, "Transaction identifier" },
        { 0x1d, "BCCH channel list" },
        { 0x1e, "Icon identifier" },
        { 0x1f, "Item Icon identifier list" },
        { 0x20, "Card reader status" },
        { 0x21, "Card ATR" },
        { 0x22, "C-APDU" },
        { 0x23, "R-APDU" },
        { 0x24, "Timer identifier" },
        { 0x25, "Timer value" },
        { 0x26, "Date-Time and Time zone" },
        { 0x27, "Call control requested action" },
        { 0x28, "AT Command" },
        { 0x29, "AT Response" },
        { 0x2a, "BC Repeat Indicator" },
        { 0x2b, "Immediate response" },
        { 0x2c, "DTMF string" },
        { 0x2d, "Language" },
        { 0x2e, "Timing Advance" },
        { 0x2f, "reserved for 3GPP" },
        { 0x30, "Browser Identity" },
        { 0x31, "URL" },
        { 0x32, "Bearer" },
        { 0x33, "Provisioning Reference File" },
        { 0x34, "Browser Termination Cause" },
        { 0x35, "Bearer description" },
        { 0x36, "Channel data" },
        { 0x37, "Channel data length" },
        { 0x38, "Channel status" },
        { 0x39, "Buffer size" },
        { 0x3a, "Card reader identifier" },
        { 0x3b, "Text String (User password)" },
        { 0x3c, "SIM/ME interface transport level" },
        { 0x3d, "URL (data destination address)" },
        { 0x3e, "Other address (data destination address)" },
        { -1, "" }
};

static struct {
        int evt;
        char *text;
} event_list[] = {
        { 0x00, "MT call" },
        { 0x01, "Call connected" },
        { 0x02, "Call disconnected" },
        { 0x03, "Location status" },
        { 0x04, "User activity" },
        { 0x05, "Idle screen available" },
        { 0x06, "Card reader status" },
        { 0x07, "Language selection" },
        { 0x08, "Browser Termination" },
        { 0x09, "Data available" },
        { 0x0A, "Channel status" },
        { -1, "" }
};

static struct {
        int code;
        char *text;
} result_codes[] = {
        { 0x00, "Command performed successfully" },
        { 0x01, "Command performed with partial comprehension" },
        { 0x02, "Command performed, with missing information" },
        { 0x03, "REFRESH performed with additional EFs read" },
        { 0x04, "Command performed successfully, but requested icon could not be displayed" },
        { 0x05, "Command performed, but modified by call control by SIM" },
        { 0x06, "Command performed successfully, limited service" },
        { 0x07, "Command performed with modification" },
        { 0x10, "Proactive SIM session terminated by the user" },
        { 0x11, "Backward move in the proactive SIM session requested by the user" },
        { 0x12, "No response from user" },
        { 0x13, "Help information required by the user" },
        { 0x14, "USSD or SS transaction terminated by the user" },
        { 0x20, "ME currently unable to process command" },
        { 0x21, "Network currently unable to process command" },
        { 0x22, "User did not accept call set-up request" },
        { 0x23, "User cleared down call before connection or network release" },
        { 0x24, "Action in contradiction with the current timer state" },
        { 0x25, "Interaction with call control by SIM, temporary problem" },
        { 0x26, "Launch browser generic error code" },
        { 0x30, "Command beyond ME's capabilities" },
        { 0x31, "Command type not understood by ME" },
        { 0x32, "Command data not understood by ME" },
        { 0x33, "Command number not known by ME" },
        { 0x34, "SS Return Error" },
        { 0x35, "SMS RP-ERROR" },
        { 0x36, "Error, required values are missing" },
        { 0x37, "USSD Return Error" },
        { 0x38, "MultipleCard commands error" },
        { 0x39, "Interaction with call control or MO SM control, permanent problem" },
        { 0x3A, "Bearer Independent Protocol error" },
        { -1, "" }
};


//---------------------------------------------------------------------------
__fastcall TTerminalForm::TTerminalForm(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void TTerminalForm::Invoke() {

        char tbuf[2];

        Visible = true;
        inprocess = true;
        setTimeout(100);
        writecomsz("ate1\r\n");
        while (readcom(tbuf, 1) > 0) ;
        setTimeout(-10);
        ReadTimer->Enabled = true;

}

void __fastcall TTerminalForm::FormCloseQuery(TObject *Sender,
      bool &CanClose)
{
        static unsigned char bk[1] = "\010";
        char tbuf[2];

        Visible = false;
        ReadTimer->Enabled = false;
        setTimeout(200);
        for (int i=0; i<64; i++) writecom(bk, 1);
        writecomsz("\r\n");
        while (readcom(tbuf, 1) > 0) ;
        writecomsz("ate0\r\n");
        while (readcom(tbuf, 1) > 0) ;
        inprocess = false;
        CanClose = false;
}
//---------------------------------------------------------------------------
void __fastcall TTerminalForm::WinKeyPress(TObject *Sender, char &Key)
{
        char bytes[2];

        if (Key > 0 && Key < 255) {
                bytes[0] = Key;
                writecom(bytes, 1);
        }
        Key = 0;
}
//---------------------------------------------------------------------------
void __fastcall TTerminalForm::ReadTimerTimer(TObject *Sender)
{
        char bytes[2];
        bool in_upd;
        AnsiString as;
        int cline;

        bytes[1] = 0;
        in_upd = false;

        while (readcom(bytes, 1) == 1) {
                if (! in_upd) {
                        in_upd = true;
                        if (Win->Lines->Count == 0) Win->Lines->Add("");
                        cline = Win->Lines->Count-1;
                        as = Win->Lines->Strings[cline];
                        Win->Lines->BeginUpdate();
                }
                if (bytes[0] == '\n') {
                        Win->Lines->Strings[cline] = as;
                        Win->Lines->Add("");
                        cline++;
                        as = "";
                } else if (bytes[0] == '\r') {
                        continue;
                } else if (bytes[0] == '\010') {
                        as.Delete(as.Length(), 1);
                } else {
                        as += bytes;
                }
        }
        if (in_upd) {
                Win->Lines->Strings[cline] = as;
                Win->Lines->EndUpdate();
                in_upd = false;
        }
}
//---------------------------------------------------------------------------
void __fastcall TTerminalForm::Copy1Click(TObject *Sender)
{
        Win->CopyToClipboard();        
}
//---------------------------------------------------------------------------
void __fastcall TTerminalForm::Paste1Click(TObject *Sender)
{
        char buf[256];
        int l;

        getClipboard(buf, 256);
        l = strlen(buf);
        if (l > 0) writecom(buf, l);
}
//---------------------------------------------------------------------------
void __fastcall TTerminalForm::ParseSTKmessage1Click(TObject *Sender)
{
        char buf[2048], val[4], *p, *s;
        unsigned char data[1024], cmd;
        int i, n, len, pos;

        n = Win->GetSelTextBuf(buf, 2047);
        p = buf;
        if (stricmp(p, "^SSTK:")) p += 6;
        while (*p == ' ' || *p == '\t') p++;
        buf[n] = 0;

        len = 0;
        while (isxdigit(*p) && isxdigit(*(p+1))) {
                val[0] = *p;
                val[1] = *(p+1);
                val[2] = 0;
                data[len++] = strtol(val, NULL, 16);
        }

        if (len < 2) {
                WarningMessage("Please select an STK message data");
                return;
        }

        pos = 0;
        cmd = data[0];
        if ((cmd & 0xf0) == 0xa0) {

        } else if ((cmd & 0xf0) == 0xd0) {
                s = "unknown";
                for (i=0; commands[i].cmd>=0; i++) {
                        if (commands[i].cmd == cmd) s = commands[i].text;
                }
                //xprintf(0x8080ff, "%s, length %i bytes", s, data[1]);
                pos += 2;
        } else if ((cmd >= 0 && cmd <= 0x3f) || (cmd >= 0x80 && cmd <= 0xbf)) {
                cmd = 0xd0;
        } else if (((cmd & 0xf) == 0xf) && (len >= 17)) {
                cmd = 0xd1;
        } else {
                WarningMessage("Message couldn't be decoded");
                return;
        }

}
//---------------------------------------------------------------------------

void __fastcall TTerminalForm::Clear1Click(TObject *Sender)
{
        Win->Clear();        
}
//---------------------------------------------------------------------------

