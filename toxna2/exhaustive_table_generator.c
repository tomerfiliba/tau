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

	if (password == NULL) {
		perror("allocating room for password failed\n");
	}
	if (digest == NULL) {
		perror("allocating room for digest failed\n");
	}

	while (1) {
		res = rule_generate_next_password(rule, password, max_password);
		if (res == RULE_STATUS_EXHAUSTED) {
			/* exhausted all passwords */
			return 0;
		} else if (res != RULE_STATUS_OK) {
			/* error message is printed by rule_generate_password */
			return -1;
		}
		pwlength = strlen(password);
		if (rule->hashfunc((unsigned char*)password, pwlength, digest) < 0) {
			fprintf(stderr, "%s of generated password failed\n", rule->hashname);
			return -1;
		}
		res = insert_uniquely_DEHT(deht, digest, rule->digest_size, (unsigned char*)password, pwlength);
		if (res == DEHT_STATUS_FAIL) {
			/* error message is printed by insert_uniquely_DEHT */
			return -1;
		}
	}
}

/* const unsigned char *keyBuf, i.e. Binary buffer input*/
/* int keySizeof , i.e. in this project this is crypt output size, */
/*          but in real life this size may vary (e.g. string input)*/
/* int nTableSize, i.e. Output is 0 to (nTableSize-1) to fit table of pointers*/
/*                                                                          */
/****************************************************************************/
int hashfun(const unsigned char * keyBuf, int keySizeof, int tableSize)
{
	int n;
	unsigned char digest[MD5_OUTPUT_LENGTH_IN_BYTES];
	MD5BasicHash(keyBuf, keySizeof, digest);
	n = *((int*)&digest);
	return n % tableSize;
}

/*Arguments are: */
/* const unsigned char *keyBuf, i.e. Binary buffer input*/
/* int keySizeof , i.e. in this project this is crypt output size, */
/*          but in real life this size may vary (e.g. string input)*/
/* unsigned char *validationKeyBuf, i.e. Output buffer, assuming allocated with nBytesPerValidationKey bytes*/
/*                                                                          */
/****************************************************************************/
int md5_validfunc(const unsigned char *keyBuf, int keySizeof, unsigned char * validationKeyBuf)
{
	int output_size = keySizeof - MD5_OUTPUT_LENGTH_IN_BYTES;
	memcpy(validationKeyBuf, keyBuf + MD5_OUTPUT_LENGTH_IN_BYTES, output_size);
	return output_size;
}

int sha1_validfunc(const unsigned char *keyBuf, int keySizeof, unsigned char * validationKeyBuf)
{
	return 0;
}



int main2(int argc, const char** argv)
{
	int res = 0;
	rule_info_t rule;
	DEHT * deht = NULL;
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

	deht = create_empty_DEHT(argv[1], NULL, NULL,
			65536, /* numEntriesInHashTable */
			10, /* nPairsPerBlock */
			8, /* nBytesPerKey */
			rule.hashname);

	if (deht == NULL) {
		/* error message printed by create_empty_DEHT */
		return 1;
	}

	if (populate_deht(deht, &rule) != 0) {
		/* error message printed by create_empty_DEHT */
		res = 1;
	}

	close_DEHT_files(deht);
	return res;
}
