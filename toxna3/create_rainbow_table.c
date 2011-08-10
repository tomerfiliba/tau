#include "deht.h"
#include "iniloader.h"
#include "rules.h"
#include "misc.h"
#include <string.h>
#include <stdlib.h>


#define INI_GET_STR(ini, var) if ((var = ini_get(ini, #var)) == NULL) {\
			fprintf(stderr, "INI file did not specify '%s'", #var);\
			return NULL;}

#define INI_GET_INT(ini, var) if (ini_get_integer(ini, #var, &var) != INI_STATUS_OK) {\
			fprintf(stderr, "INI file did not specify '%s'", #var);\
			return NULL;}

static DEHT * create_deht_from_ini(const char * prefix, const inifile_t * ini)
{
	DEHT * deht = NULL;
	int hash_size;
	const char * hash_name;
	int bucket_size;

	INI_GET_STR(ini, hash_name);
	INI_GET_INT(ini, hash_size);
	INI_GET_INT(ini, bucket_size);

	return create_empty_DEHT(prefix, my_hash_func, my_valid_func, hash_size, bucket_size, 8, hash_name);
}

static unsigned long * generate_seed_table(const char * prefix, const char * rand_seed, int chain_length)
{
	int i;
	unsigned long * seed_table = NULL;
	char digest[MD5_OUTPUT_LENGTH_IN_BYTES];
	FILE * f = NULL;
	char seedfilename[MAX_INPUT_BUFFER];

	seed_table = (unsigned long *)malloc(chain_length * sizeof(unsigned long));
	if (seed_table == NULL) {
		fprintf(stderr, "failed to allocate seed_table\n");
		return NULL;
	}

	MD5BasicHash(rand_seed, strlen(rand_seed), digest);
	for (i = 0; i < chain_length; i++) {
		memcpy(&seed_table[i], digest, sizeof(unsigned long));
		MD5BasicHash(digest, sizeof(digest), digest);
	}

	strncpy(seedfilename, prefix, sizeof(seedfilename) - 6);
	strcat(seedfilename, ".seed");
	f = fopen(seedfilename, "w");
	if (f == NULL) {
		perror(seedfilename);
		goto cleanup;
	}
	if (fwrite(seed_table, sizeof(unsigned long), chain_length, f) != chain_length) {
		perror(seedfilename);
		goto cleanup;
	}
	fclose(f);
	return seed_table;

cleanup:
	if (f != NULL) {
		fclose(f);
	}
	free(seed_table);
	return NULL;
}

static unsigned long R_function(unsigned long seed, unsigned char * digest, int digest_size)
{
	unsigned long h = *((unsigned long *)digest);
	return h * seed;
}

static int generate_single_chain(const rule_info_t * rule, int chain_length, int k, unsigned long * seed_table, 
						  char * first_password, char * last_digest)
{
	int i;
	char password[MAX_INPUT_BUFFER];
	char digest[MAX_DIGEST_LENGTH_IN_BYTES];

	if (rule_kth_password(rule, k, password, sizeof(password)) != RULE_STATUS_OK) {
		/* error message printed by rule_kth_password */
		return 1;
	}
	rule->hashfunc(password, strlen(password), digest);
	strncpy(first_password, password, sizeof(password));

	for (i = 0; i < chain_length; i++) {
		k = R_function(seed_table[i], digest, rule->digest_size) % rule->num_of_passwords;
		if (rule_kth_password(rule, k, password, sizeof(password)) != RULE_STATUS_OK) {
			/* error message printed by rule_kth_password */
			return 1;
		}
		rule->hashfunc(password, strlen(password), digest);
	}

	memcpy(last_digest, digest, rule->digest_size);
	return 0;
}

static int generate_all_chains(const rule_info_t * rule, DEHT * deht, unsigned long * seed_table, 
							   int chain_length)
{
	unsigned long i;
	char first_password[MAX_INPUT_BUFFER];
	char last_digest[MAX_DIGEST_LENGTH_IN_BYTES];

	/* fill DEHT with chain heads and tails */
	for (i = 0; i < 10 * (rule->num_of_passwords / chain_length); i++) {
		if (generate_single_chain(rule, chain_length, i, seed_table, first_password, last_digest) != 0) {
			/* error message printed by generate_chain */
			return 1;
		}
		if (add_DEHT(deht, last_digest, rule->digest_size, first_password, 
				strlen(first_password)) == DEHT_STATUS_FAIL) {
			/* error message printed by add_DEHT */
			return 1;
		}
	}

	/* great success */
	return 0;
}


static int generate_passwords(const char * prefix, const inifile_t * ini, 
							  const rule_info_t * rule, DEHT * deht)
{
	int chain_length;
	const char * rand_seed;
	int res;
	unsigned long * seed_table = NULL;

	if (ini_get_integer(ini, "num_of_R", &chain_length) != INI_STATUS_OK) {
		fprintf(stderr, "INI file did not specify 'num_of_R'");
		return 1;
	}

	if ((rand_seed = ini_get(ini, "main_rand_seed")) == NULL) {
		fprintf(stderr, "INI file did not specify 'main_rand_seed'");
		return 1;
	}
	seed_table = generate_seed_table(prefix, rand_seed, chain_length);
	if (seed_table == NULL) {
		/* error message printed by generate_seed_table */
		return 1;
	}

	res = generate_all_chains(rule, deht, seed_table, chain_length);
	free(seed_table);
	return res;
}


int main2(int argc, const char ** argv)
{
	int res = 1;
	inifile_t ini;
	rule_info_t rule;
	DEHT * deht = NULL;
	char inifilename[MAX_INPUT_BUFFER];

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <prefix>\n", argv[0]);
		goto cleanup0;
	}
	strncpy(inifilename, argv[1], sizeof(inifilename) - 5);
	strcat(inifilename, ".ini");

	if (ini_load(&ini, inifilename) != INI_STATUS_OK) {
		/* error message printed by ini_load */
		goto cleanup0;
	}
	if (rule_load(&rule, &ini) != RULE_STATUS_OK) {
		/* error message printed by rule_load */
		goto cleanup1;
	}
	deht = create_deht_from_ini(argv[1], &ini);
	if (deht == NULL) {
		/* error message printed by create_deht_from_ini */
		goto cleanup2;
	}

	res = generate_passwords(argv[1], &ini, &rule, deht);

	/* success */
	res = 0;


//cleanup3:
	close_DEHT_files(deht);
cleanup2:
	rule_finalize(&rule);
cleanup1:
	ini_finalize(&ini);
cleanup0:
	return res;
}


