#include <stdio.h>
#include <string.h>

int q(char *s) {

	fprintf(stderr, "%s\n", s);
	return 1;

}

int main(int argc, char **argv) {

	unsigned char buf[32768];
	char *name, *p;
	int i, len;
	FILE *IF, *OF;

	if (argc < 2) return q("usage: bin2inc BIN_FILE INC_FILE\n");

	IF = fopen(argv[1], "rb");
	if (IF == NULL) return q("cannot open input file\n");
	OF = fopen(argv[2], "w");
	if (OF == NULL) return q("cannot open output file\n");

	name = strdup(argv[2]);
	p = strrchr(name, '.');
	if (p != NULL) *p = '\0';

	len = fread(buf, 1, sizeof(buf), IF);
	fclose(IF);

	fprintf(OF, "static unsigned char %s[] = {\n\n", name);

	for (i=0; i<len; i++) {
		if (i % 8 == 0) fprintf(OF, "\n\t");
		fprintf(OF, "0x%02X, ", (int)buf[i]);
	}

	fprintf(OF, "\n\n};\n");
	fclose(OF);

	return 0;

}


