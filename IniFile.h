#ifndef InifileH
#define InifileH

#define SMALLBUFFER 4096
#define MAXLINELENGTH 200000

typedef struct ikey_s {

        struct ikey_s *next;
        char *data;
        int valuelen;
        char keylen;
        char valuepos;
        char qchar;

} ikey;

typedef struct isec_s {

        char *name;
        struct ikey_s *first;
        struct ikey_s *ins;
        struct ikey_s *last;

} isec;

void openIniFile(char *file);
void closeIniFile();
int getInt(char *section, char *key, int deflt);
char *getString(char *section, char *keyname, char *deflt);
char *getStringEscaped(char *section, char *keyname, char *deflt);
void addInt(char *section, char *keyname, int val);
void addString(char *section, char *keyname, char *val);
void addStringEscaped(char *section, char *keyname, char *val);
void setInt(char *section, char *keyname, int val);
void setString(char *section, char *keyname, char *val);
void setStringEscaped(char *section, char *keyname, char *val);
void deleteKey(char *section, char *keyname);
void clearSection(char *section);
void deleteSection(char *section);
void startReadSection(char *section);
char *nextPair();

#endif
