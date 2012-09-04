/* TODO
 * 	could extend to other archive formats...
 *
 * 	check not-toplevel directory (....)
 *
 */
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

void skip(unsigned long int bytes)
{
	while (bytes--) {
		(void)getchar();
	}
}

void matched(const char *type, const char *z)
{
	printf("Found .%s inside %s\n", z, type);
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
	int i, j;

	if (argc <= 1) {
		fprintf(stderr, "Usage: %s extensions... < archivefile\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	/* lowercase args */
	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '.') argv[i]++;
		for (j = 0; argv[i][j]; j++) {
			argv[i][j] = tolower( ((unsigned int)argv[i][j]) );
		}
	}

/* pkzip/infozip scanner */
ZIP_NEXT:
	if (getchar() == 80 /* P */
	&& getchar() == 75 /* K */
	&& getchar() == 3 /* ^C */
	&& getchar() == 4) { /* ^D */
		int cm, fnlen, exlen, ccsize, ucsize, gbp;
		char ext[5], *ptr;

		getchar(); /* version to extract */
		getchar();
		gbp = getchar(); /* general purpose bit flag */
		gbp |= (getchar() << 8);
		cm = getchar(); /* compression method */
		cm |= (getchar() << 8);
		getchar(); /* lastmod file time */
		getchar();
		getchar(); /* lastmod file date */
		getchar();
		getchar(); /* crc32 */
		getchar();
		getchar();
		getchar();
		ccsize = getchar(); /* compressed size */
		ccsize |= (getchar() << 8);
		ccsize |= (getchar() << 16);
		ccsize |= (getchar() << 24);
		ucsize = getchar(); /* uncompressed size */
		ucsize |= getchar();
		ucsize |= getchar();
		ucsize |= getchar();
		fnlen = getchar(); /* filename length */
		fnlen |= (getchar() << 8);
		exlen = getchar(); /* filename length */
		exlen |= (getchar() << 8);
		if (!ferror(stdin) && !feof(stdin)) {
			/* find last 4 bytes of zip filename */
			if (fnlen < 4) {
				skip(fnlen+exlen);
			} else {
				skip(fnlen-4);
				ext[0] = tolower(((unsigned int)getchar()));
				ext[1] = tolower(((unsigned int)getchar()));
				ext[2] = tolower(((unsigned int)getchar()));
				ext[3] = tolower(((unsigned int)getchar()));
				skip(exlen);
				if (ext[3] == '/') {
					/* directory entry */
				}
				ext[4] = 0;
				ptr = strchr(ext, '.');
				if (!ptr || !*ptr) ptr = ext;
				else ptr = ptr+1;
				for (i = 1; i < argc; i++) {
					if (strcmp(ptr, argv[i])) continue;

					matched("zipfile", ptr);
				}
			}
			/* skip compressed data */
			if (gbp & 8) {
				/* fuck, okay this is complicated */
				for (i = j = 0; i != EOF;) {
					while ((i = getchar()) != 80 && i != EOF) j++;
					if (i == 80 /* P */
					&& getchar() == 75 /* K */
					&& getchar() == 7 /* ^G */
					&& getchar() == 8) { /* ^H */
						getchar(); /* crc32 */
						getchar();
						getchar();
						getchar();
						ccsize = getchar(); /* compressed size */
						ccsize |= (getchar() << 8);
						ccsize |= (getchar() << 16);
						ccsize |= (getchar() << 24);
						if (ccsize != j + 4) {
							/* want to abort? */
						}
						ucsize = getchar(); /* uncompressed size */
						ucsize |= getchar();
						ucsize |= getchar();
						ucsize |= getchar();
						/* hit! */
						break;
					}
				}
			} else {
				skip(ccsize);
			}
			goto ZIP_NEXT;
		}
	}
	clearerr(stdin);
	fseek(stdin, 0, SEEK_SET);
	exit(EXIT_SUCCESS);
}
