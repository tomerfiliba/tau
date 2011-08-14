#include "deht.h"
#include "iniloader.h"
#include "rules.h"
#include "misc.h"
#include "rainbow.h"
#include <string.h>
#include <stdlib.h>


static int generate_all_chains(const config_t * config, const rule_info_t * rule,
							   DEHT * deht)
{
	const uint64_t iterations = 10 * (rule->num_of_passwords / config->chain_length);
	uint64_t i;
	uint64_t k = 17; /* some prime number */
	char first_password[MAX_INPUT_BUFFER];
	unsigned char last_digest[MAX_DIGEST_LENGTH_IN_BYTES];
#ifdef SHOW_GENERATED_CHAINS
	char hexdigest[MAX_DIGEST_LENGTH_IN_BYTES * 2 + 1];
	memset(hexdigest, 0, sizeof(hexdigest));
#endif

	/* fill DEHT with chain heads and tails */
	for (i = 0; i < iterations; i++) {
		k = (k * 47 + 1) % rule->num_of_passwords;
		if (rainbow_generate_single_chain(config, rule, k, first_password,
				sizeof(first_password), last_digest) != 0) {
			/* error message printed by generate_chain */
			return 1;
		}
		if (add_DEHT(deht, last_digest, config->digest_size, (unsigned char*)first_password,
				strlen(first_password)) == DEHT_STATUS_FAIL) {
			/* error message printed by add_DEHT */
			return 1;
		}
#if SHOW_GENERATED_CHAINS
		binary2hexa(last_digest, config->digest_size, hexdigest, sizeof(hexdigest));
		printf("%05lu (%05lu): added %s = '%s'\n", i, k, hexdigest, first_password);
#endif
	}

	/* great success */
	return 0;
}

int main(int argc, const char ** argv)
{
	int res = 1;
	config_t config;
	rule_info_t rule;
	DEHT * deht = NULL;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <prefix>\n", argv[0]);
		goto cleanup0;
	}

	if (config_load(&config, argv[1]) != INI_STATUS_OK) {
		/* error message printed by config_load */
		goto cleanup0;
	}
	if (rule_init(&rule, config.rule_pattern, config.lexicon_file) != RULE_STATUS_OK) {
		/* error message printed by rule_init */
		goto cleanup1;
	}

	deht = 	create_empty_DEHT(config.prefix, my_hash_func, my_valid_func,
		config.num_of_buckets, config.bucket_size, 8, config.hash_name);
	if (deht == NULL) {
		/* error message printed by create_empty_DEHT */
		goto cleanup2;
	}

	if (rainbow_generate_seed_table(&config) != RAINBOW_STATUS_OK) {
		/* error message printed by rainbow_generate_seed_table */
		goto cleanup3;
	}

	res = generate_all_chains(&config, &rule, deht);

cleanup3:
	close_DEHT_files(deht);
cleanup2:
	rule_finalize(&rule);
cleanup1:
	config_finalize(&config);
cleanup0:
	return res;
}


