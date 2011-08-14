#include "misc.h"
#include "deht.h"
#include "iniloader.h"
#include "rainbow.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _MSC_VER
	#define strcasecmp _stricmp
#else
	#include <strings.h>
#endif


static int user_input_loop(const config_t * config, const rule_info_t * rule, 
						   DEHT * deht)
{
	int res;
	char line[MAX_INPUT_BUFFER];
	unsigned char digest[MAX_DIGEST_LENGTH_IN_BYTES];
	char hexdigest[MAX_DIGEST_LENGTH_IN_BYTES * 2 + 1];
	char * cmd, * rcmd;
	char password[MAX_INPUT_BUFFER];

	while (1) {
		printf(">> ");
		if (fgets(line, sizeof(line)-1, stdin) == NULL) {
			break;
		}
		/* trim whitespace */
		for (cmd = line; *cmd == ' ' || *cmd == '\t'; cmd++);
		for (rcmd = cmd + strlen(cmd) - 1; *rcmd == ' ' || *rcmd == '\t' || *rcmd == '\n'; rcmd--) {
			*rcmd = '\0';
		}
		if (*cmd == '\0') {
			continue; /* empty line */
		}
		else if (strcasecmp(cmd, "quit") == 0) {
			break; /* quit cleanly */
		}

		if (cmd[0] == '!') {
			/* plain-text password */
			cmd++;
			config->hash_func((unsigned char*)cmd, strlen(cmd), digest);
			binary2hexa(digest, config->digest_size, hexdigest, sizeof(hexdigest));
			printf("\tIn hexa password is %s\n", hexdigest);
		}
		else if (hexa2binary(cmd, digest, config->digest_size) < 0) {
			if (strlen(cmd) > (unsigned)config->digest_size) {
				fprintf(stderr, "Too long\n");
			}
			else {
				fprintf(stderr, "Non hexa\n");
			}
			continue;
		}

		/* query the rainbow table */
		res = rainbow_query(config, rule, deht, digest, password, sizeof(password) - 1);
		if (res == RAINBOW_STATUS_OK) {
			printf("Try to login with %s\n", password);
		}
		else if (res == RAINBOW_STATUS_NOT_FOUND) {
			printf("Password not found\n");
		}
		else {
			/* error message printed by crack_password */
			return 1;
		}
	}

	return 0;
}


int main2(int argc, const char ** argv)
{
	int res = 1;
	DEHT * deht = NULL;
	rule_info_t rule;
	config_t config;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <prefix>\n", argv[0]);
		return 1;
	}

	/* load ini */
	if (config_load(&config, argv[1]) != INI_STATUS_OK) {
		/* error message printed by ini_load */
		goto cleanup0;
	}
	if (rule_init(&rule, config.rule_pattern, config.lexicon_file) != RULE_STATUS_OK) {
		/* error message printed by rule_init */
		goto cleanup1;
	}

	/* load seed table */
	if (rainbow_load_seed_table(&config) != RAINBOW_STATUS_OK) {
		/* error message printed by load_seed_table */
		goto cleanup2;
	}

	/* load DEHT */
	deht = load_DEHT_from_files(config.prefix, my_hash_func, my_valid_func);
	if (deht == NULL) {
		/* error message printed by load_DEHT */
		goto cleanup2;
	}
	if (read_DEHT_pointers_table(deht) != DEHT_STATUS_SUCCESS) {
		/* error message printed by read_DEHT_pointers_table */
		goto cleanup3;
	}

	/* main loop */
	res = user_input_loop(&config, &rule, deht);

cleanup3:
	close_DEHT_files(deht);
cleanup2:
	rule_finalize(&rule);
cleanup1:
	config_finalize(&config);
cleanup0:
	return res;
}


