#include <vcl.h>
#include <stdio.h>

#include "Config.h"
#include "Utility.h"
#include "comm.h"

char *AT = "AT";
char *EOL = "\r";

HANDLE CommHandle = NULL;
int timeout;
bool port_open;
int baudrate;

bool connect(int CommPort, int Baudrate) {

        char buf[16];
        DCB dcb;
        DWORD e;

        baudrate = Baudrate;
        sprintf(buf, "\\\\.\\COM%i", CommPort);

        VC("connect(\"%s\", %i) {", buf, baudrate);

        CommHandle = CreateFile(
                buf,
                GENERIC_READ|GENERIC_WRITE,
                0,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL
        );

        if (CommHandle == INVALID_HANDLE_VALUE) {
                e = GetLastError();
                VC("} connect: CreateFile: error %i", e);
                CommHandle = NULL;
                MessageBox(NULL, "Cannot open port", "Error", MB_OK|MB_ICONERROR);
                return false;
        }

        GetCommState(CommHandle, &dcb);

        dcb.BaudRate = Baudrate;
        dcb.fParity = FALSE;
        dcb.fOutxCtsFlow = FALSE;
        dcb.fOutxDsrFlow = FALSE;
        dcb.fDtrControl = DTR_CONTROL_ENABLE;
        dcb.fDsrSensitivity = FALSE;
        dcb.fOutX = FALSE;
        dcb.fInX = FALSE;
        dcb.fNull = FALSE;
        dcb.fRtsControl = RTS_CONTROL_ENABLE;
        dcb.fAbortOnError = FALSE;
        dcb.ByteSize = 8;
        dcb.Parity = NOPARITY;
        dcb.StopBits = ONESTOPBIT;

        if (! SetCommState(CommHandle, &dcb)) {
                e = GetLastError();
                VC("} connect: SetCommState: error %i", e);
                CloseHandle(CommHandle);
                MessageBox(NULL, "Cannot set port settings", "Error", MB_OK|MB_ICONERROR);
                return false;
        }

        if (! setTimeout(timeout = 200)) {
                e = GetLastError();
                VC("} connect: setTimeout: error %i", e);
                CloseHandle(CommHandle);
                MessageBox(NULL, "Cannot set comm timeouts", "Error", MB_OK|MB_ICONERROR);
                return false;
        }

        VC("} connect: ok");
        port_open = true;
        return true;

}

void disconnect() {

        char buf[64];

        VC("disconnect {");
        port_open = 0;

        if (CommHandle != NULL) {
                sprintf(buf, "AT+IPR=0%s", EOL);
                writecomsz(buf);
                comm_flush();
                CloseHandle(CommHandle);
        }
        CommHandle = NULL;

        VC("} disconnect");

}

bool is_connected() {

        return port_open;

}

unsigned long readcom(unsigned char *buf, int len) {

        unsigned long nr, i;
        DWORD e;

        VC_("readcom(%i): ", len);
        if (! ReadFile(CommHandle, buf, len, &nr, NULL)) {
                e = GetLastError();
                _VC_("ReadFile: error %i\n", e);
                return -1;
        }

        if (TraceComm) {
                for (i=0; i<nr; i++) {
                        if (i > 128) {
                                _VC_("...");
                                break;
                        }
                        _VC_("%02X ", buf[i]);
                }
                _VC_("(%i bytes)\n", nr);
        }
        return nr;
}

unsigned long writecom(unsigned char *buf, int len) {

        unsigned long nw, i;
        int c;
        DWORD e;

        VC_("writecom(%i): ", len);
        if (baudrate < 460800) {
                if (! WriteFile(CommHandle, buf, len, &nw, NULL)) {
                        e = GetLastError();
                        _VC_("WriteFile: error %i\n", e);
                        return -1;
                }
        } else {
                for (c=0; c<len; c++) {
                        if (! WriteFile(CommHandle, buf+c, 1, &nw, NULL)) {
                                e = GetLastError();
                                _VC_("WriteFile: error %i\n", e);
                                return -1;
                        }
                        uSleep(2);
                }
                nw = len;
        }

        if (TraceComm) {
                for (i=0; i<nw; i++) {
                        if (i > 128) {
                                _VC_("...");
                                break;
                        }
                        _VC_("%02X ", buf[i]);
                }
                _VC_("(%i bytes)\n", nw);
        }
        return nw;
}

unsigned long writecomsz(char *buf) {

        VC("writecomsz(\"%s\"):", buf);
        return writecom((unsigned char *)buf, strlen(buf));

}

char *exec_at(char *cmd) {

        static char buf[256], *s, *e;
        int n;

        VC("exec_at(\"%s\") {", cmd);

        writecomsz(cmd);
        writecomsz(EOL);

        setTimeout(100);
        n = readcom(buf, sizeof(buf));
        buf[n] = '\0';
        s = buf;
        while (*s != 0) {
                if (*s == '\r' || *s == '\n') {
                        while ((*s == '\r' || *s == '\n') && *s != 0) s++;
                        if (*s == '+') {
                                while (*s != ':' && *s != 0) s++;
                                while (*s == ' ') s++;
                        }
                        e = s;
                        while (*e != '\r' && *e != '\n') e++;
                        *e = '\0';
                        VC("} exec_at: \"%s\"", s);
                        return s;
                }
                s++;
        }
        VC("} exec_at: \"%s\"", s);
        return s;

}

bool setSpeed(int n) {

        DCB dcb;
        DWORD evt;

        VC("setSpeed(%i);", n);

        if (!GetCommState(CommHandle, &dcb))
                return false;

        dcb.BaudRate = n;

        return SetCommState(CommHandle, &dcb);

}

bool setTimeout(int n) {

        COMMTIMEOUTS ctm;

        VC("setTimeout(%i);", n);

        if (n > 0) {
                timeout = n;
                ctm.ReadIntervalTimeout = n;
                ctm.ReadTotalTimeoutMultiplier = 1;
                ctm.ReadTotalTimeoutConstant = n;
                ctm.WriteTotalTimeoutMultiplier = 0;
                ctm.WriteTotalTimeoutConstant = 0;
        } else {
                timeout = n;
                n = -n;
                ctm.ReadIntervalTimeout = 10;
                ctm.ReadTotalTimeoutMultiplier = 0;
                ctm.ReadTotalTimeoutConstant = n;
                ctm.WriteTotalTimeoutMultiplier = 0;
                ctm.WriteTotalTimeoutConstant = 0;
        }

        return SetCommTimeouts(CommHandle, &ctm);

}

void flushBuffers() {

        DWORD evt;

        FlushFileBuffers(CommHandle);
        Sleep(2);

}

void comm_clear() {

        VC("comm_clear();");
        PurgeComm(CommHandle, PURGE_RXCLEAR);

}


void comm_flush() {

        unsigned char buf[16];
        int t = timeout;
        int i;

        VC("comm_flush();");
        setTimeout(20);
        for (i=0; i<2048; i++) {
                if (readcom(buf, 1) != 1) break;
        }
        PurgeComm(CommHandle, PURGE_RXCLEAR|PURGE_TXCLEAR);
        setTimeout(t);

}








