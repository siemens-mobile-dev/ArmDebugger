#include "Wire.h"
#include "comm.h"
#include "Debug.h"
#include "EventLog.h"
#include "Config.h"
#include "Utility.h"
#include <stdio.h>

static bool online = false;
static bool in_sync = false;
static int ExtraChars1, ExtraChars2;
static bool Echo;


bool ping_cgsn() {

        unsigned char buf[64];
        int n;

        VW("ping {");

        if (! is_connected()) {
                VW("} ping (not connected)");
                return false;
        }

        comm_flush();
        setTimeout(100);
        sprintf(buf, "AT%sATE0%s", EOL, EOL);
        writecomsz(buf);
        n = readcom(buf, 63);
        if (n > 0) {
                buf[n] = '\0';
                if (strstr((char *)buf, "OK") != NULL) {
                        VW("} ping ok");
                        return true;
                }
        }
        VW("} ping (no answer)");
        return false;

}


void trackConnection() {

        static bool flip;
        unsigned char tbuf[128], buf[128], *p;
        int i, n;

        if (in_sync) return;
        if (!is_connected()) return;
        if (online) return;
        if (inprocess) return;

        flip = ! flip;

        setSpeed(flip ? 115200 : Baudrate);
        if (ping()) {

                setTimeout(200);
                sprintf(tbuf, "AT+CGSN:00000000%s", EOL);
                writecomsz(tbuf);
                ExtraChars1 = ExtraChars2 = -1;
                Echo = false;
                n = readcom(buf, 128);
                for (i=0; i<n; i++) {
                        if (buf[i] == 0xA1) {
                                ExtraChars1 = i;

                                ExtraChars2 = n - (ExtraChars1 + 5);
                                if (strncmp(buf, tbuf, strlen(tbuf)) == 0) {
                                        ExtraChars1 -= strlen(tbuf);
                                        Echo = true;
                                }
                                break;
                        }
                }
                if (ExtraChars1 < 0 || ExtraChars2 < 0) {
                        disconnect();
                        WarningMessage("CGSN patch not installed or wrong patch version\nPlease generate and apply CGSN patch");
                }

                if (flip && Baudrate != 115200) {
                        sprintf(buf, "AT+IPR=%i", Baudrate);
                        exec_at(buf);
                        setSpeed(Baudrate);
                }

                in_sync = true;
                if (! synchronize()) {
                        disconnect();
                        in_sync = false;
                        return;
                }
                in_sync = false;
                online = true;
                updateFreePoolEntries(getFreePoolEntries());
                VW("trackConnection: connected");

                reload_dumps();
        }

}

void lostConnection(bool f) {

        VW("lostConnection");
        setSystemState(SS_CONNECTING);
        if (f && online) {
                newEvent('-', "Lost connection to mobile");
        }
        online = false;

}

bool is_online() {

        return online;

}

void checkExit(unsigned char *ans, int len) {

        unsigned char buf[4096];
        char mbuf[4096], *s, *fsrtext;
        unsigned int r[16], cpsr, xfsr, xfar, *ip;
        int i, j, n, l, p, type;

        setTimeout(-200);
        memcpy(buf, ans, len);
        n = readcom(buf+len, sizeof(buf)-len);
        if (n > 0) len += n;

        if (TraceWire) {
                VW_("checkExit: ");
                for (i=0; i<len; i++) {
                        _VW_("%02X", buf[i]);
                        if ((i%32)==31) _VW_("\n           ");
                }
                _VW_("\n");
        }

        for (i=0; i<len-4; i++) {
                if (buf[i] == 0xab && buf[i+1] == 0x04 && buf[i+2] == 0x75) {
                        type = buf[i+3];
                        switch (type) {
                                case 0: s = "Prefetch abort"; break;
                                case 1: s = "Data abort"; break;
                                case 2: s = "Undefined instruction"; break;
                                default: s = "Unknown exception"; break;
                        }
                        VW("checkExit: %s", s);
                        ip = (unsigned int *)(buf+i);
                        cpsr = ip[1];
                        xfsr = ip[2];
                        xfar = ip[3];
                        if (buf[i+3] == 1) {
                                switch (xfsr & 0xf) {
                                        case 0: fsrtext = "(vector exception)"; break;
                                        case 1: fsrtext = "(alignment fault)"; break;
                                        case 2: fsrtext = "(terminal exception)"; break;
                                        case 3: fsrtext = "(alignment fault)"; break;
                                        case 4: fsrtext = "(section linefetch)"; break;
                                        case 5: fsrtext = "(section translation fault)"; break;
                                        case 6: fsrtext = "(page linefetch)"; break;
                                        case 7: fsrtext = "(page translation fault)"; break;
                                        case 8: fsrtext = "(section non-linefetch)"; break;
                                        case 9: fsrtext = "(section domain fault)"; break;
                                        case 10: fsrtext = "(page non-linefetch)"; break;
                                        case 11: fsrtext = "(page domain fault)"; break;
                                        case 12: fsrtext = "(ext. abort on 1st-level translation)"; break;
                                        case 13: fsrtext = "(section permission fault)"; break;
                                        case 14: fsrtext = "(ext. abort on 2nd-level translation)"; break;
                                        case 15: fsrtext = "(page permission fault)"; break;
                                }
                        } else {
                                fsrtext = "";
                        }
                        for (j=0; j<16; j++) {
                                r[j] = ip[4+j];
                        }
                        r[15] = r[15] - 4;

                        sprintf(mbuf,   "%s %s at %08X\n"
                                        "CPSR=%08X FSR=%08X FAR=%08X\n"
                                        "R0=%08X  R4=%08X  R8=%08X R12=%08X\n"
                                        "R1=%08X  R5=%08X  R9=%08X  SP=%08X\n"
                                        "R2=%08X  R6=%08X R10=%08X  LR=%08X\n"
                                        "R3=%08X  R7=%08X R11=%08X  PC=%08X\n",

                                        s, fsrtext, r[15], cpsr, xfsr, xfar,
                                        r[0], r[4], r[8],  r[12],
                                        r[1], r[5], r[9],  r[13],
                                        r[2], r[6], r[10], r[14],
                                        r[3], r[7], r[11], r[15]
                        );
                        newEvent('x', mbuf);
                        break;
                }
        }

        lostConnection(true);

}

unsigned char *atquery(unsigned char *cmd, unsigned char *buf, int len, int timeout) {

        int i, n;
        unsigned char *p;

        buf[0] = 0;

        comm_clear();
        writecomsz(cmd);
        setTimeout(timeout);
        n = readcom(buf, len+1+ExtraChars1+ExtraChars2);
        if (n == 0) {
                comm_clear();
                writecomsz(cmd);
                setTimeout(timeout);
                n = readcom(buf, len+1+ExtraChars1+ExtraChars2);
        }

        if (n < len+1+ExtraChars1+ExtraChars2) {
                checkExit(buf, n);
                return NULL;
        }

        p = buf + ExtraChars1;
        if (*p != 0xA1) {
                checkExit(buf, n);
                return NULL;
        }

        return p+1;
}


bool readFromRAM_cgsn(unsigned int addr, unsigned char *buf, int len) {

        unsigned char cbuf[64], tbuf[2100], *tp;
        int i;
        unsigned char c1, c2;

        if (len != 4) {
                sprintf(cbuf, "AT+CGSN:%08X,%08X%s", addr, len, EOL);
        } else {
                sprintf(cbuf, "AT+CGSN:%08X%s", addr, EOL);
        }

        if ((tp = atquery(cbuf, tbuf, len, 750)) == NULL) return false;

        for (i=0; i<len; i++) {
                buf[i] = tp[i];
        }

        return true;

}


bool writeToRAM_cgsn(unsigned int addr, unsigned char *buf, int len) {

        unsigned char cbuf[512], tbuf[2100];
        int i;
        unsigned char c1, c2;

        sprintf(cbuf, "AT+CGSN*%08X", addr);
        for (i=0; i<len; i++) {
                sprintf(cbuf+16+i*2, "%02X", buf[i]);
        }
        strcat(cbuf, EOL);

        if (atquery(cbuf, tbuf, 0, 1000) == NULL) return false;
        return true;

}

bool queryRAM_cgsn(unsigned int *addrs, unsigned int *data, int count) {

        unsigned char cbuf[512], tbuf[2100], *tp;
        int i, j;
        unsigned int v;
        unsigned char c;

        strcpy(cbuf, "AT+CGSN%");
        for (i=0; i<count; i++) {
                sprintf(cbuf+8+i*8, "%08X", addrs[i]);
        }
        strcat(cbuf, EOL);

        if ((tp = atquery(cbuf, tbuf, count*4, 400)) == NULL) return false;

        for (i=0; i<count; i++) {
                for (v=0,j=3; j>=0; j--) {
                        v = (v << 8) + tp[i*4+j];
                }
                data[i] = v;
        }

        return true;
}


bool armCall_cgsn(unsigned int addr, unsigned int r[16], int count, unsigned int &cpsr, int timeout) {

        unsigned char cbuf[256], tbuf[256], *tp;
        int i, j, l;
        unsigned int v;
        unsigned char c;

        l = sprintf(cbuf, "AT+CGSN@%08X", addr);
        for (i=0; i<count; i++) {
                l += sprintf(cbuf+l, ",%08X", r[i]);
        }
        strcat(cbuf, EOL);


        if ((tp = atquery(cbuf, tbuf, 56, timeout)) == NULL) return false;

        for (i=0; i<14; i++) {
                for (v=0,j=3; j>=0; j--) {
                        v = (v << 8) + tp[i*4+j];
                }
                r[i] = v;
        }
        cpsr = r[13];
        return true;

}

bool ping() {
        return ping_cgsn();
}

bool readFromRAM(unsigned int addr, unsigned char *buf, int len) {

        int tlen;
        bool r;

        VW("readFromRAM(addr=%08X, len=%08X) {", addr, len);

        while (len > 0) {

                tlen = (len > 256) ? 256 : len;
                r = readFromRAM_cgsn(addr, buf, tlen);
                if (! r) {
                        VW("} readFromRAM (error at %08X)", addr);
                        return false;
                }

                addr += tlen;
                buf += tlen;
                len -= tlen;

        }

        VW("} readFromRAM");
        return true;
}

bool writeToRAM(unsigned int addr, unsigned char *buf, int len) {

        int tlen;
        bool r;

        VW("writeToRAM(addr=%08X, len=%08X) {", addr, len);

        while (len > 0) {

                tlen = (len > 128) ? 128 : len;

                r = writeToRAM_cgsn(addr, buf, tlen);
                if (! r) {
                        VW("} writeToRAM (error at %08X)", addr);
                        return false;
                }
                addr += tlen;
                buf += tlen;
                len -= tlen;

        }

        VW("} writeToRAM");
        return true;
}

bool queryRAM(unsigned int *addrs, unsigned int *data, int count) {

        int i;

        if (TraceWire) {
                VW_("queryRAM(");
                for (i=0; i<count; i++) _VW_("%08X,", addrs[i]);
                _VW_(") {\n");
        }

        while (count > 0) {

                int n = (count > 32) ? 32 : count;

                bool r = queryRAM_cgsn(addrs, data, n);
                if (! r) {
                        VW("} queryRAM (error at %08X)", addrs[0]);
                        return false;
                }

                addrs += n;
                data += n;
                count -= n;

        }

        VW("} queryRAM");
        return true;
}

bool armCall(unsigned int addr, unsigned int r[16], int count, unsigned int &cpsr, int timeout) {

        bool res;

        VW("armCall(addr=%08X, r0=%08X, r1=%08X, timeout=%i) {", addr, r[0], r[1], timeout);
        res = armCall_cgsn(addr, r, count, cpsr, timeout);
        VW("} armCall = %s", res ? "true" : "false");
        return res;
}




