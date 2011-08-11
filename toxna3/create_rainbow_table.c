#include "deht.h"
#include "iniloader.h"
#include "rules.h"
#include "misc.h"
#include "rainbow.h"
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
	int hash_size;
	const char * hash_name;
	int bucket_size;

	INI_GET_STR(ini, hash_name);
	INI_GET_INT(ini, hash_size);
	INI_GET_INT(ini, bucket_size);

	return create_empty_DEHT(prefix, my_hash_func, my_valid_func, hash_size, bucket_size, 8, hash_name);
}

static int generate_all_chains(const rule_info_t * rule, DEHT * deht, uint64_t * seed_table,
							   int chain_length)
{
	const uint64_t iterations = 10 * (rule->num_of_passwords / chain_length);
	uint64_t i;
	uint64_t k = 313;
	char first_password[MAX_INPUT_BUFFER];
	unsigned char last_digest[MAX_DIGEST_LENGTH_IN_BYTES];
	/*char hexdigest[MAX_DIGEST_LENGTH_IN_BYTES * 2 + 1];
	memset(hexdigest, 0, sizeof(hexdigest));*/

	/* fill DEHT with chain heads and tails */
	for (i = 0; i < iterations; i++) {
		k %= rule->num_of_passwords;
		if (rainbow_generate_single_chain(rule, chain_length, k, seed_table,
				first_password, last_digest) != 0) {
			/* error message printed by generate_chain */
			return 1;
		}
		if (add_DEHT(deht, last_digest, rule->digest_size, (unsigned char*)first_password,
				strlen(first_password)) == DEHT_STATUS_FAIL) {
			/* error message printed by add_DEHT */
			return 1;
		}
		/*binary2hexa(last_digest, rule->digest_size, hexdigest, sizeof(hexdigest));
		printf("%05llu: added %s : '%s'\n", k, hexdigest, first_password);*/
		k *= 47;
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
	uint64_t * seed_table = NULL;

	if (ini_get_integer(ini, "num_of_R", &chain_length) != INI_STATUS_OK) {
		fprintf(stderr, "INI file did not specify 'num_of_R'");
		return 1;
	}

	if ((rand_seed = ini_get(ini, "main_rand_seed")) == NULL) {
		fprintf(stderr, "INI file did not specify 'main_rand_seed'");
		return 1;
	}
	seed_table = rainbow_generate_seed_table(prefix, rand_seed, chain_length);
	if (seed_table == NULL) {
		/* error message printed by generate_seed_table */
		return 1;
	}

	res = generate_all_chains(rule, deht, seed_table, chain_length);
	free(seed_table);
	return res;
}


int main(int argc, const char ** argv)
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

	close_DEHT_files(deht);
cleanup2:
	rule_finalize(&rule);
cleanup1:
	ini_finalize(&ini);
cleanup0:
	return res;
}


