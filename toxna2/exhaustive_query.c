#include <stdio.h>
#include <stdlib.h>
#include "misc.h"
#include "rules.h"
#include "deht.h"


void find_password_for_digest(DEHT * deht, const unsigned char * digest, int digestLength)
{
	int res;
	unsigned char password[400];
	
	res = query_DEHT(deht, digest, digestLength, password, sizeof(password));
	if (res > 0) {
		/* password is not NUL-terminated */
		password[res] = '\0';
		printf("Try to login with password \"%s\"\n", password);
	}
	else {
		printf("Sorry but this hash doesn't appear in pre-processing\n");
	}
}


int main(int argc, const char** argv)
{
	DEHT * deht = NULL;
	rule_info_t rule;
	char line[257];
	unsigned char digest[MAX_DIGEST_LENGTH_IN_BYTES];
	char ini_file[200];

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <prefix>\n", argv[0]);
		return 1;
	}

	strncpy(ini_file, argv[1], sizeof(ini_file) - 5);
	strcat(ini_file, ".ini");
	if (rule_load_from_file(&rule, ini_file) != 0) {
		/* error message printed by rule_load_from_file */
		return 1;
	}

	deht = load_DEHT_from_files(argv[1], NULL /*hashfun*/, NULL /*validfun*/ );
	if (deht == NULL) {
		return 1;
	}

	while (1) {
		printf(">> ");
		if (fgets(line, sizeof(line)-1, stdin) == NULL) {
			break; /* EOF */
		}
		if (strcmp(line, "quit") == 0) {
			break; /* quit cleanly */
		}
		if (hexa2binary(line, digest, sizeof(digest)) < 0) {
			printf("Non hexa\n");
			continue;
		}
		find_password_for_digest(deht, digest, rule.digest_size);
	}

	close_DEHT_files(deht);
	return 0;
}

