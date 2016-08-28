#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alloc.h>
#include "IniFile.h"

static char *IniFileName = NULL;
static bool ichanged;

static isec *sections;
static int maxsections = 0;
static int nsections = 0;
static char nbuf[SMALLBUFFER];
static char *wbuf = NULL;
static ikey *rskey = NULL;


static void freewbuf() {

        if (wbuf != NULL) { free(wbuf); wbuf = NULL; }

}

static char *getbuf(int len) {

        if (len < SMALLBUFFER-1) {
                return nbuf;
        } else {
                freewbuf();
                return wbuf = (char *)malloc(len + 2);
        }

}

static char *escape(char *val) {

        int vlen;
        char c, *p, *pp, *buf;

        vlen = strlen(val);
        buf = getbuf(vlen*2);

        p = val;
        pp = buf;
        while (*p) {
                c = *(p++);
                switch (c) {
                        case '\n': *(pp++) = '\\'; *(pp++) = 'n'; break;
                        case '\r': *(pp++) = '\\'; *(pp++) = 'r'; break;
                        case '\t': *(pp++) = '\\'; *(pp++) = 't'; break;
                        case ';': *(pp++) = '\\'; *(pp++) = ';'; break;
                        case '\\': *(pp++) = '\\'; *(pp++) = '\\'; break;
                        default: *(pp++) = c; break;
                }
        }
        *pp = 0;
        return buf;

}

static char *unescape(char *val) {

        int vlen;
        char c, *p, *pp, *buf;

        vlen = strlen(val);
        buf = getbuf(vlen);

        p = val;
        pp = buf;
        while (*p) {
                c = *(p++);
                if (c == '\\') {
                        switch (*p) {
                                case 'n': *(pp++) = '\n'; break;
                                case 'r': *(pp++) = '\r'; break;
                                case 't': *(pp++) = '\t'; break;
                                case ';': *(pp++) = ';'; break;
                                case '\\': *(pp++) = '\\'; break;
                                case 0: *(pp++) = c; p--; break;
                                default: *(pp++) = c; *(pp++) = *p; break;
                        }
                        p++;
                } else {
                        *(pp++) = c;
                }
        }
        *pp = 0;
        return buf;

}

static isec *newsection(char *name) {

        if (nsections >= maxsections) {
                maxsections = 16 + maxsections + (maxsections >> 1);
                sections = (isec *)realloc(sections, maxsections * sizeof(isec));
        }

        sections[nsections].name = strdup(name);
        sections[nsections].first = NULL;
        sections[nsections].ins = NULL;
        sections[nsections].last = NULL;
        nsections++;

        return &sections[nsections-1];

}

static void clearsec(isec *sec) {

        ikey *key, *pkey;

        key = sec->first;
        while (key != NULL) {
                pkey = key;
                key = key->next;
                if (pkey->data) free(pkey->data);
                free(pkey);
        }
        sec->first = sec->ins = sec->last = NULL;

}

static void addline(isec *sec, char *s, int keylen, int valuepos, int valuelen) {

        ikey *key;
        int len;
        char *p;

        len = strlen(s) + 1;
        key = (ikey *)malloc(sizeof(ikey));
        key->data = (char *)malloc(len);
        key->next = NULL;
        memcpy(key->data, s, len);
        key->keylen = keylen;
        key->valuepos = valuepos;
        key->valuelen = valuelen;
        if (keylen != 0) {
                p = s + valuepos + valuelen;
                key->qchar = *p;
                *p = 0;
        } else {
                key->qchar = 0;
        }

        if (sec->last == NULL) {
                sec->first = key;
        } else {
                sec->last->next = key;
        }
        sec->last = key;
        if (keylen != 0) sec->ins = key;

}

static isec *findsec(char *section, bool create) {

        int i;

        for (i=0; i<nsections; i++) {
                if (stricmp(sections[i].name, section) == 0) {
                        return &sections[i];
                }
        }

        if (! create) return NULL;
        return newsection(section);

}

static ikey *findkey(char *section, char *keyname) {

        isec *csec;
        ikey *ckey;
        int len;

        csec = findsec(section, false);
        if (csec == NULL) return NULL;
        ckey = csec->first;
        len = strlen(keyname);
        while (ckey != NULL) {
                if (ckey->keylen == len && ckey->valuelen != -1) {
                        if (strnicmp(ckey->data, keyname, len) == 0) {
                                return ckey;
                        }
                }
                ckey = ckey->next;
        }
        return NULL;

}


void openIniFile(char *name) {

        isec *csec;
        char *buf;
        char *p, *pp, *pn, *pv, *pc;
        FILE *F;

        if (IniFileName != NULL) closeIniFile();
        buf = (char *)malloc(MAXLINELENGTH+2);
        IniFileName = strdup(name);
        ichanged = false;
        csec = newsection("");

        F = fopen(name, "r");
        while (F != NULL && fgets(buf, MAXLINELENGTH, F) != NULL) {

                p = buf;
                while (*p == ' ' || *p == '\t') p++;
                pp = p + strlen(p) - 1;
                while (pp >= p && (*pp & 0xe0) == 0) *(pp--) = 0;

                if (*p == '[') {
                        pp = strchr(p, ']');
                        if (pp != NULL) *pp = 0;
                        csec = newsection(p+1);
                        continue;
                }

                for (pc=p; *pc; pc++) {
                        if (*pc == ';' && (pc == p || *(pc-1) != '\\')) {
                                break;
                        }
                }
                while (pc > p && (*(pc-1) == ' ' || *(pc - 1) == '\t')) pc--;

                pp = strchr(p, '=');
                if (pp == NULL || pp > pc) {
                        addline(csec, p, 0, 0, 0);
                } else {
                        pn = pp;
                        while (pn > p && (*(pn-1) == ' ' || *(pn-1) == '\t')) pn--;
                        pv = pp+1;
                        while (pv < pc && (*pv == ' ' || *pv == '\t')) pv++;
                        addline(csec, p, pn-p, pv-p, pc-pv);
                }

        }

        free(buf);

}

void closeIniFile() {

        isec *csec;
        ikey *ckey, *pkey;
        int i;
        FILE *F;

        if (IniFileName == NULL) return;

        if (ichanged) {

                F = fopen(IniFileName, "w");
                if (F == NULL) return;

                for (i=0; i<nsections; i++) {
                        csec = &sections[i];
                        if (csec->name[0] != 0) fprintf(F, "[%s]\n", csec->name);
                        ckey = csec->first;
                        while (ckey != NULL) {
                                if (ckey->valuelen != -1) {
                                        if (ckey->qchar != 0) {
                                                ckey->data[ckey->valuepos+ckey->valuelen] = ckey->qchar;
                                        }
                                        fprintf(F, "%s\n", ckey->data);
                                }
                                ckey = ckey->next;
                        }
                }

                fclose(F);

        }

        for (i=0; i<nsections; i++) {
                clearsec(&sections[i]);
        }
        free(sections);
        sections = NULL;
        nsections = maxsections = 0;
        free(IniFileName);
        freewbuf();
        IniFileName = NULL;

}

int getInt(char *section, char *keyname, int deflt) {

        ikey *ckey;
        char *p;

        ckey = findkey(section, keyname);
        if (ckey == NULL) return deflt;
        p = ckey->data + ckey->valuepos;
        return (*p == '-') ? strtol(p, NULL, 0) : strtoul(p, NULL, 0);

}

char *getString(char *section, char *keyname, char *deflt) {

        ikey *ckey;

        ckey = findkey(section, keyname);
        if (ckey == NULL) return deflt;
        return ckey->data + ckey->valuepos;

}

char *getStringEscaped(char *section, char *keyname, char *deflt) {

        ikey *ckey;

        ckey = findkey(section, keyname);
        if (ckey == NULL) return deflt;
        return unescape(ckey->data + ckey->valuepos);

}

void addInt(char *section, char *keyname, int val) {

        char buf[34];

        itoa(val, buf, 10);
        addString(section, keyname, buf);

}

void addString(char *section, char *keyname, char *val) {

        isec *csec;
        ikey *key;
        int nlen, vlen;

        csec = findsec(section, true);
        nlen = strlen(keyname);
        vlen = strlen(val);
        key = (ikey *)malloc(sizeof(ikey));
        key->data = (char *)malloc(nlen+vlen+2);
        key->next = NULL;
        memcpy(key->data, keyname, nlen);
        key->data[nlen] = '=';
        memcpy(key->data+nlen+1, val, vlen+1);
        key->keylen = nlen;
        key->valuepos = nlen+1;
        key->valuelen = vlen;
        key->qchar = 0;

        if (csec->ins == NULL) {
                csec->first = key;
                csec->ins = key;
        } else {
                key->next = csec->ins->next;
                csec->ins->next = key;
                csec->ins = key;
        }
        ichanged = true;

}

void addStringEscaped(char *section, char *keyname, char *val) {

        char *s;

        s = escape(val);
        addString(section, keyname, s);
        freewbuf();

}

void setInt(char *section, char *keyname, int val) {

        char buf[34];

        itoa(val, buf, 10);
        setString(section, keyname, buf);

}

void setString(char *section, char *keyname, char *val) {

        ikey *ckey;
        int cvpos, cvlen, vlen, newlen, rest;
        char *data, *newdata;

        ckey = findkey(section, keyname);
        if (ckey == NULL) {
                addString(section, keyname, val);
                return;
        }

        cvpos = ckey->valuepos;
        cvlen = ckey->valuelen;
        vlen = strlen(val);
        data = ckey->data;
        rest = strlen(data+cvpos+cvlen) + 1;
        newlen = cvpos + vlen + rest;
        newdata = (char *)malloc(newlen);
        memcpy(newdata, data, cvpos);
        memcpy(newdata+cvpos, val, vlen);
        memcpy(newdata+cvpos+vlen, data+cvpos+cvlen, rest);
        ckey->data = newdata;
        free(data);
        ckey->valuelen = vlen;
        ichanged = true;

}

void setStringEscaped(char *section, char *keyname, char *val) {

        char *s;

        s = escape(val);
        setString(section, keyname, s);
        freewbuf();

}

void deleteKey(char *section, char *keyname) {

        ikey *ckey;

        ckey = findkey(section, keyname);
        if (ckey == NULL) return;

        free(ckey->data);
        ckey->data = NULL;
        ckey->valuelen = -1;
        ichanged = true;

}

void clearSection(char *section) {

        isec *csec;

        csec = findsec(section, true);
        clearsec(csec);
        ichanged = true;

}

void deleteSection(char *section) {

        isec *csec;

        csec = findsec(section, false);
        if (csec == NULL) return;
        clearsec(csec);
        csec->name[0] = 0;
        ichanged = true;

}

void startReadSection(char *section) {

        isec *csec;

        csec = findsec(section, false);
        if (csec == NULL) {
                rskey = NULL;
        } else {
                rskey = csec->first;
        }

}

char *nextPair() {

        ikey *key;
        char *buf;
        int len;

        while (rskey != NULL) {
                key = rskey;
                rskey = rskey->next;
                if (key->keylen == 0) continue;
                if (key->valuelen == -1) continue;
                len = key->keylen + key->valuelen + 1;
                buf = getbuf(len+2);
                memcpy(buf, key->data, key->keylen);
                buf[key->keylen] = '=';
                memcpy(buf+key->keylen+1, key->data+key->valuepos, key->valuelen);
                buf[len] = 0;
                return buf;
        }
        return NULL;

}


