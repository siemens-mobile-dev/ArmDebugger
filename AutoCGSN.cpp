//---------------------------------------------------------------------------

#include <vcl.h>
#include <stdio.h>
#include "Debug.h"
#include "Utility.h"
#pragma hdrstop

#include "AutoCGSN.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TAutoCGSNform *AutoCGSNform;

static unsigned int base;
static unsigned int OriginalAtHandle;
static unsigned int GetAtCommandLine;
static unsigned int AtEmptyBuffer;
static unsigned int StoreASCIIZtoSendLater;
static unsigned int AfterAtCommand;
static unsigned int StoreDataToSendLater;
static unsigned int PatchAddress;
static unsigned int JumpAddress;

#include "asm/cgsn_patch.h"

unsigned int read4(unsigned char *p) {

	return *p | (*(p+1) << 8) | (*(p+2) << 16) | (*(p+3) << 24);

}

void write4(unsigned char *p, unsigned int n) {

	*p = (unsigned char)(n & 0xff);
	*(p+1) = (unsigned char)((n >> 8) & 0xff);
	*(p+2) = (unsigned char)((n >> 16) & 0xff);
	*(p+3) = (unsigned char)((n >> 24) & 0xff);

}

unsigned int findrec(unsigned char *data, int len, unsigned int addr) {

	unsigned int x, y;
	int i;

	for (i=0; i<len; i+=4) {
		x = * ((unsigned int *)(data+i));
		if (x == addr) {
			y = read4(data+i+0x04); if (y < base && y >= base+len) continue;
			y = read4(data+i+0x08); if (y < base && y >= base+len) continue;
			y = read4(data+i+0x0c); if (y > 255) continue;
			y = read4(data+i+0x10); if (y < base && y >= base+len) continue;

			return base+i;
		}
	}
	return 0;

}

int is_push(unsigned char *data) {

	return ( (*(data+1) & 0xfe) == 0xb4 );

}

int is_pop(unsigned char *data) {

        int v = (*(data+1) & 0xfe);
	return ( (*(data+1) & 0xfe) == 0xbc );

}

int is_bl_blx_prefix(unsigned char *data) {

	return ( (*(data+1) & 0xf8) == 0xf0 );

}

unsigned char *find_bl(unsigned char *p) {

	unsigned char *pp = p;

	while (! is_pop(p)) {

		if (is_bl_blx_prefix(p)) return p;

		p += 2;
		if (p - pp > 1024) return NULL;

	}

	return NULL;

}

unsigned int bl_address(unsigned char *p, unsigned int addr) {

	unsigned int a;

	a = (((*p | (*(p+1) << 8)) & 0x7ff) << 12);
        a |= (((*(p+2) | (*(p+3) << 8)) & 0x7ff) << 1);
	if (a & 0x400000) a |= 0xff800000;
        a = addr + 4 + a;
        if ((*(p+3) & 0x10) != 0) a |= 1;

	return a;

}

int check_original_cgsn(unsigned char *code, unsigned int addr) {

	unsigned char *p;

	if (! is_push(code)) return 0;
	p = code;
	p = find_bl(p+2); if (p == NULL) return 0;
	AtEmptyBuffer = bl_address(p, addr+(p-code));
	p = find_bl(p+4); if (p == NULL) return 0;
	p = find_bl(p+4); if (p == NULL) return 0;
	StoreASCIIZtoSendLater = bl_address(p, addr+(p-code));
	p = find_bl(p+4); if (p == NULL) return 0;
	AfterAtCommand = bl_address(p, addr+(p-code));
	p = find_bl(p+4); if (p != NULL) return 0;

	return 1;

}

int check_original_atd(unsigned char *code, unsigned int addr) {

	unsigned char *p;

	if (! is_push(code)) return 0;
	p = code;
	p = find_bl(p+2); if (p == NULL) return 0;
	GetAtCommandLine = bl_address(p, addr+(p-code));

	return 1;

}

int check_original_cimi(unsigned char *code, unsigned int addr) {

	unsigned char *p, *p1, *p2;

	if (! is_push(code)) return 0;
	p = code;
        p1 = p2 = NULL;

        while (true) {
                p2 = p1;
                p1 = p;
                p = find_bl(p+4);
                if (p == NULL) break;
        }

        if (p2 == NULL) return 0;
        StoreDataToSendLater = bl_address(p2, addr+(p2-code));

	return 1;

}

void writePatchData(FILE *f, unsigned int a, unsigned char *d1, unsigned char *d2, int len) {

	int i, n;
	char buf1[64], buf2[64];

	while (len > 0) {

		n = (16 - (a & 0xf)); if (n > len) n = len;

		for (i=0; i<n; i++) {
			sprintf(buf1+i*2, "%02X", d1[i]);
			sprintf(buf2+i*2, "%02X", d2[i]);
		}

		fprintf(f, "%08X: %s %s\n", a-base, buf1, buf2);


		a += n;
		d1 += n;
		d2 += n;
		len -= n;
	}

}


int generate(char *ifname, char *ofname, unsigned int b, unsigned int pa) {

	unsigned char buf1[4096], buf2[4096];
	unsigned char *data, *p;
	FILE *F, *OF;
	long n, len;
	int i, j, cgsn_found, d_found, cimi_found;
	unsigned int a, a2, y;

        base = b;

	F = fopen(ifname, "rb");
	if (F == NULL)
		return WarningMessage("Cannot open %s\n", ifname);

	fseek(F, 0, SEEK_END);
	len = ftell(F);
	fseek(F, 0, SEEK_SET);

	if (len != 0x01000000 && len != 0x02000000 && len != 0x04000000) {
		fclose(F);
		return WarningMessage("%s is not a fullflash file\n", ifname);
	}

	data = (unsigned char *)malloc(len);
	if (data == NULL) {
		fclose(F);
		return WarningMessage("Out of memory\n");
	}

	n = fread(data, 1, len, F);
	fclose(F);

	if (n != len) {
		free(data);
		return WarningMessage("Error reading input file\n");
	}

	/* ----------------------------- */

	cgsn_found = d_found = cimi_found = 0;

	for (i=0; i<len; i+=4) {

                a = base+i;
                a2 = read4(data+i); if (a2 < base || a2 >= base+len) continue;
        	y = read4(data+i+0x04); if (y < base || y >= base+len) continue;
		y = read4(data+i+0x08); if (y < base || y >= base+len) continue;
		y = read4(data+i+0x0c); if (y > 255) continue;
		y = read4(data+i+0x10); if (y < base || y >= base+len) continue;
                j = a2 - base;

		if ((! cgsn_found) && data[j] == 'G' && data[j+1] == 'S' && data[j+2] == 'N' && data[j+3] == '\0') {

			a = read4(data+(a+0x10-base));
                        if (a < base || a >= base+len) continue;
			JumpAddress = a+4;
			a = read4(data+(JumpAddress-base));
                        if (a < base || a >= base+len) continue;
			if ((a & 1) == 0) continue;

			OriginalAtHandle = a;
			a &= ~1;

			if (! check_original_cgsn(data+(a-base), a)) continue;
			cgsn_found = 1;

		}

		if ((! d_found) && data[j] == 'D' && data[j+1] == '\0') {

			a = read4(data+(a+0x10-base));
                        if (a < base || a >= base+len) continue;
			a = read4(data+(a-base));
                        if (a < base || a >= base+len) continue;
			if ((a & 1) == 0) continue;

			a &= ~1;

			if (! check_original_atd(data+(a-base), a)) continue;
			d_found = 1;

		}

		if ((! cimi_found) && data[j] == 'I' && data[j+1] == 'M' && data[j+2] == 'I' && data[j+3] == '\0') {

			a = read4(data+(a+0x10-base));
                        if (a < base || a >= base+len) continue;
                        a += 4;
			a = read4(data+(a-base));
                        if (a < base || a >= base+len) continue;
			if ((a & 1) == 0) continue;

			a &= ~1;

			if (! check_original_cimi(data+(a-base), a)) continue;
			cimi_found = 1;

		}
	}

	if (cgsn_found == 0) {
		free(data);
		return WarningMessage("Cannot find reference to AT+CGSN\n");
	}
	if (d_found == 0) {
		free(data);
		return WarningMessage("Cannot find reference to ATD\n");
	}
	if (cimi_found == 0) {
		free(data);
		return WarningMessage("Cannot find reference to AT+CIMI\n");
	}

        if (pa != 0) {
                i = pa - base;
        	for (j=i; j<i+sizeof(cgsn_patch); j++) {
                	if (data[j] != 0xff) {
                       		free(data);
                		return WarningMessage("Specified memory region is not empty\n");
                        }
        	}
        } else {
        	i = OriginalAtHandle - base;
        	while (i < len) {
        		if (data[i] == 0xff) {
        			for (j=i; j<len; j++) {
        				if (data[j] != 0xff) break;
        			}
        			if (j-i >= sizeof(cgsn_patch) + 0x20) {
                                        p = data + (i & ~0x1ffff);
                                        if (strncmp(p, "EEFULL", 6) != 0 &&
                                            strncmp(p, "EELITE", 6) != 0 &&
                                            strncmp(p, "FFS", 3) != 0
                                        ) {
                                                break;
                                        }
                                }
                                i = j;
        		}
        		i++;
        	}
        	if (i >= len) {
        		free(data);
        		return WarningMessage("No free space for patch data\n");
        	}
        }

	PatchAddress = ((i+3) & ~3) + base;

	write4(cgsn_patch+0x04, OriginalAtHandle);
	write4(cgsn_patch+0x08, GetAtCommandLine);
	write4(cgsn_patch+0x0c, AtEmptyBuffer);
	write4(cgsn_patch+0x10, StoreASCIIZtoSendLater);
	write4(cgsn_patch+0x14, AfterAtCommand);
	write4(cgsn_patch+0x18, StoreDataToSendLater);

	free(data);

	OF = fopen(ofname, "w");
	if (OF == NULL) {
		return WarningMessage("Cannot open %s\n", ofname);
	}

	write4(buf1, OriginalAtHandle);
	write4(buf2, PatchAddress);
	writePatchData(OF, JumpAddress, buf1, buf2, 4);

	memset(buf1, 0xff, sizeof(buf1));
	write4(cgsn_patch+0x04, OriginalAtHandle);
	write4(cgsn_patch+0x08, GetAtCommandLine);
	write4(cgsn_patch+0x0c, AtEmptyBuffer);
	write4(cgsn_patch+0x10, StoreASCIIZtoSendLater);
	write4(cgsn_patch+0x14, AfterAtCommand);
	write4(cgsn_patch+0x18, StoreDataToSendLater);
	writePatchData(OF, PatchAddress, buf1, cgsn_patch, sizeof(cgsn_patch));

	fclose(OF);

	return 1;

}



//---------------------------------------------------------------------------
__fastcall TAutoCGSNform::TAutoCGSNform(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------

void TAutoCGSNform::Invoke() {
        Visible = true;
        BringToFront();
        SetFocus();
}

void __fastcall TAutoCGSNform::bSearchClick(TObject *Sender)
{
        if (FlashOpenDlg->Execute()) {
                eFile->Text = FlashOpenDlg->FileName;
        }
}
//---------------------------------------------------------------------------

void __fastcall TAutoCGSNform::bGenerateClick(TObject *Sender)
{
        AnsiString as, as1, as2;
        char *ifile, *ofile;
        unsigned int b, pa;

        as = eBase->Text;
        b = strtoul(as.c_str(), NULL, 16);
        as = ePA->Text;
        pa = strtoul(as.c_str(), NULL, 16);
        if (b == 0) {
                WarningMessage("Wrong flash base");
                return;
        }
        if (pa != 0 && (pa < FlashAddress || pa >= FlashAddress+FlashSize)) {
                WarningMessage("Wrong patch address");
                return;
        }

        as1 = eFile->Text;
        ifile = as1.c_str();
        if (strlen(ifile) == 0) {
                WarningMessage("Fullflash filename not given");
                return;
        }
        if (SaveVkpDlg->Execute()) {
                bGenerate->Enabled = false;
                Cursor = crHourGlass;
                Repaint();
                as2 = SaveVkpDlg->FileName;
                ofile = as2.c_str();
                if (generate(ifile, ofile, b, pa)) {
                        InfoMessage("Patch generated successfully");
                }
                Cursor = crDefault;
                bGenerate->Enabled = true;
        }
        Visible = false;

}
//---------------------------------------------------------------------------

