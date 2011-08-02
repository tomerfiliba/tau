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


/*
 * look up a password in the DEHT by the given digest, and print it to the screen
 */
void find_password_for_digest(DEHT * deht, const unsigned char * digest, int digestLength)
{
	int res;
	unsigned char password[MAX_INPUT_BUFFER];
	
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

/*
 * read a line from the user and handle the command (either a hash or "quit")
 */
static void user_input_loop(DEHT * deht, unsigned int digest_size)
{
	char line[MAX_INPUT_BUFFER];
	unsigned char digest[MAX_DIGEST_LENGTH_IN_BYTES];
	char * cmd, * rcmd;

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
			if (strlen(cmd) > digest_size) {
				printf("Too long\n");
			}
			else {
				printf("Non hexa\n");
			}
			continue;
		}
		find_password_for_digest(deht, digest, digest_size);
	}
}

int main(int argc, const char** argv)
{
	DEHT * deht = NULL;
	int res = 0;
	unsigned int digest_size;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <prefix>\n", argv[0]);
		return 1;
	}

	/* load deht */
	deht = load_DEHT_from_files(argv[1], my_hash_func, my_valid_func);
	if (deht == NULL) {
		return 1;
	}

	/* instructions: 
	 *   Load the header into memory, but not the table of pointers (i.e. do not call 
	 *   read_DEHT_pointers_table).
	 */

	/* compute digest size (determined by hash name) */
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

	/* main loop */
	user_input_loop(deht, digest_size);

cleanup:
	if (deht != NULL) {
		close_DEHT_files(deht);
	}
	return res;
}

