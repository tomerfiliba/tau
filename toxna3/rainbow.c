#include "rainbow.h"
#include <string.h>
#include <stdlib.h>


uint64_t * rainbow_generate_seed_table(const char * prefix, const char * rand_seed, int chain_length)
{
	int i;
	uint64_t * seed_table = NULL;
	unsigned char digest[MD5_OUTPUT_LENGTH_IN_BYTES];
	FILE * f = NULL;
	char seedfilename[MAX_INPUT_BUFFER];

	seed_table = (uint64_t*)malloc(chain_length * sizeof(uint64_t));
	if (seed_table == NULL) {
		fprintf(stderr, "failed to allocate seed_table\n");
		return NULL;
	}

	MD5BasicHash((const unsigned char*)rand_seed, strlen(rand_seed), digest);
	for (i = 0; i < chain_length; i++) {
		memcpy(&seed_table[i], digest, sizeof(uint64_t));
		MD5BasicHash(digest, sizeof(digest), digest);
	}

	strncpy(seedfilename, prefix, sizeof(seedfilename) - 6);
	strcat(seedfilename, ".seed");
	f = fopen(seedfilename, "w");
	if (f == NULL) {
		perror(seedfilename);
		goto cleanup;
	}
	if (fwrite(seed_table, sizeof(uint64_t), chain_length, f) != chain_length) {
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


uint64_t * rainbow_load_seed_table(const char * prefix, int chain_length)
{
	char seedfilename[MAX_INPUT_BUFFER];
	FILE * f = NULL;
	uint64_t * seed_table = NULL;

	strncpy(seedfilename, prefix, sizeof(seedfilename) - 6);
	strcat(seedfilename, ".seed");
	f = fopen(seedfilename, "r");
	if (f == NULL) {
		perror(seedfilename);
		return NULL;
	}

	seed_table = (uint64_t*)malloc(chain_length * sizeof(uint64_t));
	if (seed_table == NULL) {
		fprintf(stderr, "failed to allocate seed_table\n");
		goto cleanup1;
	}

	if (fread(seed_table, sizeof(uint64_t), chain_length, f) != chain_length) {
		perror(seedfilename);
		goto cleanup2;
	}
	fclose(f);
	return seed_table;

cleanup2:
	free(seed_table);
cleanup1:
	fclose(f);
	return NULL;
}


static uint64_t rainbow_reduce(const rule_info_t * rule, uint64_t seed, unsigned char * digest)
{
	/* the input digest has high-enough entropy, and so does the seed, 
       so we might as well just multiply them... no need for more complex
	   reduction methods */
	uint64_t h = *((uint64_t*)digest);
	return (h * seed) % rule->num_of_passwords;
}

static int rainbow_compute_chain(const rule_info_t * rule, const uint64_t * seed_table,
								 int from, int to, unsigned char * digest)
{
	int i;
	uint64_t k;
	char password[MAX_INPUT_BUFFER];

	for (i = from; i <= to; i++) {
		k = rainbow_reduce(rule, seed_table[i], digest);
		if (rule_kth_password(rule, k, password, sizeof(password), 0) != RULE_STATUS_OK) {
			/* error message printed by rule_kth_password */
			return RAINBOW_STATUS_ERROR;
		}
		rule->hashfunc((unsigned char*)password, strlen(password), digest);
	}
	return RAINBOW_STATUS_OK;
}

int rainbow_generate_single_chain(const rule_info_t * rule, int chain_length, uint64_t k,
		const uint64_t * seed_table, char * first_password, unsigned char * last_digest)
{
	if (rule_kth_password(rule, k, first_password, MAX_DIGEST_LENGTH_IN_BYTES, 0) != 
			RULE_STATUS_OK) {
		/* error message printed by rule_kth_password */
		return 1;
	}
	rule->hashfunc((unsigned char*)first_password, strlen(first_password), last_digest);
	return rainbow_compute_chain(rule, seed_table, 0, chain_length - 1, last_digest);
}

int rainbow_query(DEHT * deht, const rule_info_t * rule, const uint64_t * seed_table,
				  int chain_length, int multi_query, const unsigned char * target_digest,
				  char * out_password)
{
	unsigned char curr_digest[MAX_DIGEST_LENGTH_IN_BYTES];


	return RAINBOW_STATUS_NOT_FOUND;
}


