#include <stdio.h>
#include <stdlib.h>
#include "misc.h"
#include "rules.h"
#include "deht.h"
#ifdef _MSC_VER
	#define strcasecmp _stricmp
#else
	#include <strings.h>
#endif


#define MAX_LINE_SIZE    256


void find_password_for_digest(DEHT * deht, const unsigned char * digest, int digestLength)
{
	int res;
	unsigned char password[400];
	
	res = query_DEHT(deht, digest, digestLength, password, sizeof(password));
	if (res == DEHT_STATUS_NOT_NEEDED) {
		printf("Sorry but this hash doesn't appear in pre-processing\n");
	}
	else if (res > 0) {
		/* password is not NUL-terminated */
		password[res] = '\0';
		printf("Try to login with password \"%s\"\n", password);
	}
	else {
		printf("An error occured while querying the DEHT\n");
	}
}

int main3(int argc, const char** argv)
{
	DEHT * deht = NULL;
	char line[MAX_LINE_SIZE + 1];
	unsigned char digest[MAX_DIGEST_LENGTH_IN_BYTES];
	int res = 0;
	char * cmd, * rcmd;
	int digest_size;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <prefix>\n", argv[0]);
		return 1;
	}

	deht = load_DEHT_from_files(argv[1], my_hash_func, my_valid_func);
	if (deht == NULL) {
		return 1;
	}

	if (strcmp(deht->header.sHashName, "MD5") == 0) {
		digest_size = MD5_OUTPUT_LENGTH_IN_BYTES;
	}
	else if (strcmp(deht->header.sHashName, "SHA1") == 0) {
		digest_size = SHA1_OUTPUT_LENGTH_IN_BYTES;
	}
	else {
		fprintf(stderr, "invalid DEHT hash '%s'", deht->header.sHashName);
		res = 1;
		goto cleanup;
	}

	while (1) {
		printf(">> ");
		if (fgets(line, sizeof(line)-1, stdin) == NULL) {
			break; /* EOF */
		}
		cmd = line;
		/* remove leading whitespace */
		while (*cmd == ' ' || *cmd == '\t') {
			cmd++;
		}
		/* remove trailing whitespace */
		for (rcmd = cmd + strlen(cmd) - 1; *rcmd == ' ' || *rcmd == '\t' ||*rcmd == '\n'; rcmd--) {
			*rcmd = '\0';
		}
		if (*cmd == '\0') {
			continue; /* empty line */
		}
		if (strcasecmp(cmd, "quit") == 0) {
			break; /* quit cleanly */
		}
		if (hexa2binary(cmd, digest, digest_size) < 0) {
			printf("Non hexa\n");
			continue;
		}
		find_password_for_digest(deht, digest, digest_size);
	}

cleanup:
	if (deht != NULL) {
		close_DEHT_files(deht);
	}
	return res;
}

