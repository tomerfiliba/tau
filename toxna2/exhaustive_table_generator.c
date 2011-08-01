#include <stdio.h>
#include <malloc.h>
#include "rules.h"
#include "deht.h"


static int populate_deht(DEHT * deht, rule_info_t * rule)
{
	int res;
	int pwlength;
	const int max_password = rule_max_password_length(rule);
	char * password = (char*)malloc(max_password + 1);
	unsigned char * digest = (unsigned char*)malloc(rule->digest_size);
	int added_passwords = 0;
	char buf[100];

	if (password == NULL) {
		perror("allocating room for password failed\n");
		return -1;
	}
	if (digest == NULL) {
		perror("allocating room for digest failed\n");
		return -1;
	}

	while (1) {
		res = rule_generate_next_password(rule, password, max_password);
		if (res == RULE_STATUS_EXHAUSTED) {
			/* exhausted all passwords */
			/*printf("Added %d passwords\n", added_passwords);*/
			return 0;
		} else if (res != RULE_STATUS_OK) {
			/* error message is printed by rule_generate_password */
			return -1;
		}
		pwlength = strlen(password);
		if (pwlength == 0) {
			/* ignore the empty password */
			continue;
		}
		if (rule->hashfunc((unsigned char*)password, pwlength, digest) < 0) {
			fprintf(stderr, "%s of generated password failed\n", rule->hashname);
			return -1;
		}
		binary2hexa(digest, rule->digest_size, buf, sizeof(buf));
		printf("%s : %s\n", buf, password);

		/*if (strcmp(password, "S3") == 0) {
			res = 7;
		}*/

		/* When receiving parameter “flag = ‘n’” in .ini file, you must create n random passwords 
		 * (even if n is bigger than password space). In such a case you should use insert_uniquely_DEHT
		 * to avoid inserting same password many times. When “flag = ‘all’” you should use add_DEHT. */
		/*if (rule->limit < 0) {
			res = add_DEHT(deht, digest, rule->digest_size, (unsigned char*)password, pwlength);
		}
		else {*/
		res = insert_uniquely_DEHT(deht, digest, rule->digest_size, (unsigned char*)password, pwlength);
		if (res == DEHT_STATUS_FAIL) {
			/* error message is printed by insert_uniquely_DEHT */
			return -1;
		}
		added_passwords++;
		if (added_passwords % 1000 == 0) {
			printf("%d\n", added_passwords);
		}
	}
}


int main(int argc, const char** argv)
{
	int res = 0;
	rule_info_t rule;
	DEHT * deht = NULL;
	char ini_file[200];

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <prefix>\n", argv[0]);
		return 1;
	}

	system("del complex_md5.data");
	system("del complex_md5.key");

	/* load rule */
	strncpy(ini_file, argv[1], sizeof(ini_file) - 5);
	strcat(ini_file, ".ini");
	if (rule_load_from_file(&rule, ini_file) != 0) {
		/* error message printed by rule_load_from_file */
		return 1;
	}

	/* create deht */
	deht = create_empty_DEHT(argv[1], my_hash_func, my_valid_func,
			65536, /* numEntriesInHashTable */
			10, /* nPairsPerBlock */
			8, /* nBytesPerKey */
			rule.hashname);
	if (deht == NULL) {
		/* error message printed by create_empty_DEHT */
		goto cleanup;
	}

	/* populate the table */
	if (populate_deht(deht, &rule) != 0) {
		/* error message printed by create_empty_DEHT */
		res = 1;
	}

cleanup:
	if (deht != NULL) {
		close_DEHT_files(deht);
	}
	rule_finalize(&rule);
	return res;
}
