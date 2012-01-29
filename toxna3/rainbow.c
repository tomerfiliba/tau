#include "rainbow.h"
#include <string.h>
#include <stdlib.h>


/*
 * API
 *
 * generates the seed table, from the random seed given in the configuration.
 * the table will be stored inside the config struct (config->seed_table), 
 * and it will be freed by config_finalize(). also saves it to the seed file.
 *
 * Parameters:
 *    * config - the configuration struct
 * Returns: RAINBOW_STATUS_OK if the seed table was generated successfully; 
 *          RAINBOW_STATUS_ERROR on error.
 */
int rainbow_generate_seed_table(config_t * config)
{
	int i;
	unsigned char digest[MD5_OUTPUT_LENGTH_IN_BYTES];
	FILE * f = NULL;
	char seedfilename[MAX_INPUT_BUFFER];

	config->seed_table = (uint64_t*)malloc(config->chain_length * sizeof(uint64_t));
	if (config->seed_table == NULL) {
		fprintf(stderr, "failed to allocate seed_table\n");
		return RAINBOW_STATUS_ERROR;
	}

	MD5BasicHash((const unsigned char*)config->random_seed, strlen(config->random_seed),
		digest);
	for (i = 0; i < config->chain_length; i++) {
		memcpy(&config->seed_table[i], digest, sizeof(uint64_t));
		MD5BasicHash(digest, sizeof(digest), digest);
	}

	strncpy(seedfilename, config->prefix, sizeof(seedfilename) - 6);
	strcat(seedfilename, ".seed");
	f = fopen(seedfilename, "w");
	if (f == NULL) {
		perror(seedfilename);
		goto cleanup;
	}
	if (fwrite(config->seed_table, sizeof(uint64_t), config->chain_length, f) !=
			config->chain_length) {
		perror(seedfilename);
		goto cleanup;
	}
	fclose(f);
	return RAINBOW_STATUS_OK;

cleanup:
	if (f != NULL) {
		fclose(f);
	}
	free(config->seed_table);
	config->seed_table = NULL;
	return RAINBOW_STATUS_ERROR;
}

/*
 * API
 *
 * loads a previously generated seed table. the table will be stored
 * inside the config struct (config->seed_table), and it will be freed
 * by config_finalize().
 *
 * Parameters:
 *    * config - the configuration struct
 * Returns: RAINBOW_STATUS_OK if the seed table was loaded successfully; 
 *          RAINBOW_STATUS_ERROR on error.
 */
int rainbow_load_seed_table(config_t * config)
{
	char seedfilename[MAX_INPUT_BUFFER];
	FILE * f = NULL;

	strncpy(seedfilename, config->prefix, sizeof(seedfilename) - 6);
	strcat(seedfilename, ".seed");
	f = fopen(seedfilename, "r");
	if (f == NULL) {
		perror(seedfilename);
		return RAINBOW_STATUS_ERROR;
	}

	config->seed_table = (uint64_t*)malloc(config->chain_length * sizeof(uint64_t));
	if (config->seed_table == NULL) {
		fprintf(stderr, "failed to allocate seed_table\n");
		goto cleanup1;
	}

	if (fread(config->seed_table, sizeof(uint64_t), config->chain_length, f) !=
			config->chain_length) {
		perror(seedfilename);
		goto cleanup2;
	}
	fclose(f);
	return RAINBOW_STATUS_OK;

cleanup2:
	free(config->seed_table);
	config->seed_table = NULL;
cleanup1:
	fclose(f);
	return RAINBOW_STATUS_ERROR;
}

/*
 * out reduction function: takes a digest and a seed and returns back a number
 * in range [0,num_of_passwords)
 */
static uint64_t reducer(const rule_info_t * rule, uint64_t seed, unsigned char * digest)
{
	/* the input digest has a high-enough entropy, and so does the seed,
       so we might as well just multiply them... no need for more complex
	   reduction methods */
	uint64_t h = *((uint64_t*)digest);
	return (h * seed) % rule->num_of_passwords;
}

/*
 * computes a rainbow chain: iteratively reduces a digest and projects it back 
 * into the password space; goes from `from` up to `upper_bound` (exclusive)
 */
static int rainbow_reduce_chain(const config_t * config, const rule_info_t * rule,
								int from, int upper_bound, unsigned char * digest,
								char * password, int max_password)
{
	int i;
	uint64_t k;

	for (i = from; i < upper_bound; i++) {
		k = reducer(rule, config->seed_table[i], digest);
		if (rule_kth_password(rule, k, password, max_password, 0) != RULE_STATUS_OK) {
			/* error message printed by rule_kth_password */
			return RAINBOW_STATUS_ERROR;
		}
		config->hash_func((unsigned char*)password, strlen(password), digest);
	}
	return RAINBOW_STATUS_OK;
}

/*
 * API
 *
 * generates a single rainbow chain, and returns the head (first password) and
 * tail (last digest)
 *
 * Parameters:
 *    * config - the configuration struct
 *    * rule - the rule object
 *    * k - the first password index into the password space
 *    * max_password - the maximal size of the password buffer
 * Output Parameters:
 *    * first_password - the buffer for the first password (head of chain)
 *    * last_digest - the buffer for the last digest (end of chain)
 * Returns: RAINBOW_STATUS_OK if the target digest was found; 
 *          RAINBOW_STATUS_ERROR on error.
 */
int rainbow_generate_single_chain(const config_t * config, const rule_info_t * rule,
								  uint64_t k, char * first_password, int max_password,
								  unsigned char * last_digest)
{
	char tmp_password[MAX_INPUT_BUFFER];

	if (rule_kth_password(rule, k, first_password, max_password, 0) !=
			RULE_STATUS_OK) {
		/* error message printed by rule_kth_password */
		return RAINBOW_STATUS_ERROR;
	}
	config->hash_func((unsigned char*)first_password, strlen(first_password), last_digest);
	return rainbow_reduce_chain(config, rule, 0, config->chain_length, last_digest,
		tmp_password, sizeof(tmp_password) - 1);
}

/*
 * initialize a masrek object (allocates the memory buffers)
 */
static int masrek_init(masrek_t * masrek, int max_items)
{
	int i;

	masrek->max_items = max_items;
	masrek->buffer_size = MAX_INPUT_BUFFER * max_items;
	masrek->items = NULL;
	masrek->buffer = (char*)malloc(masrek->buffer_size);
	if (masrek->buffer == NULL) {
		fprintf(stderr, "failed to allocate masrek buffer\n");
		return -1;
	}
	masrek->items = (masrek_item_t*)malloc(max_items * sizeof(masrek_item_t));
	if (masrek->items == NULL) {
		free(masrek->buffer);
		masrek->buffer = NULL;
		fprintf(stderr, "failed to allocate masrek buffer\n");
		return -1;
	}
	for (i = 0; i < max_items; i++) {
		masrek->items[i].length = -1;
		masrek->items[i].buffer = NULL;
	}
	return 0;
}

/*
 * release all resources held by the masrek
 */
static void masrek_finalize(masrek_t * masrek)
{
	if (masrek->buffer != NULL) {
		free(masrek->buffer);
		masrek->buffer = NULL;
	}
	if (masrek->items != NULL) {
		free(masrek->items);
		masrek->items = NULL;
	}
}

/*
 * API
 *
 * performs a query on the rainbow table, looking for a password that generates
 * the target hash.
 *
 * Parameters:
 *    * config - the configuration struct
 *    * rule - the rule object
 *    * deht - the DEHT
 *    * target_digest - the digest to try to match
 *    * max_password - the maximal size of the password buffer
 * Output Parameters:
 *    * password - the password buffer (NUL-terminated string)
 * Returns: RAINBOW_STATUS_OK if the target digest was found; RAINBOW_STATUS_NOT_FOUND
 *          if the target digest was not found; RAINBOW_STATUS_ERROR on error.
 */
int rainbow_query(const config_t * config, const rule_info_t * rule, DEHT * deht,
				  const unsigned char * target_digest, char * password, int max_password)
{
	int j, k, max_size;
	int res;
	int num_of_matches;
	masrek_t masrek;
	unsigned char digest[MAX_DIGEST_LENGTH_IN_BYTES];
	char tmp_password[MAX_INPUT_BUFFER];

	if (masrek_init(&masrek, config->num_of_query_results) != 0) {
		/* error message printed by masrek_init */
		return 1;
	}

	/* iterate over all starting points in the chain */
	for (j = config->chain_length; j >= 0; j--) {
		memcpy(digest, target_digest, config->digest_size);
		
		/* go down the chain */
		if (rainbow_reduce_chain(config, rule, j, config->chain_length, digest, 
				tmp_password, sizeof(tmp_password) - 1) != RAINBOW_STATUS_OK) {
			goto cleanup_error;
		}

		/* find all possible starting points */
		num_of_matches = multi_query_DEHT(deht, digest, config->digest_size, &masrek);
		if (num_of_matches < 0) {
			/* error message printed by multi_query_DEHT */
			goto cleanup_error;
		}

		/* go over all possible starting points */
		for (k = 0; k < num_of_matches; k++) {
			config->hash_func((unsigned char*)masrek.items[k].buffer, 
				masrek.items[k].length, digest);

			if (rainbow_reduce_chain(config, rule, 0, j, digest, 
					tmp_password, sizeof(tmp_password) - 1) != RAINBOW_STATUS_OK) {
				goto cleanup_error;
			}

			if (memcmp(digest, target_digest, config->digest_size) == 0) {
				/* found a matching password - copy it out */
				max_size = strlen(tmp_password);
				if (max_size > max_password) {
					max_size = max_password;
				}
				memcpy(password, tmp_password, max_size);
				password[max_size] = '\0';
				goto cleanup_success;
			}
		}
	}

	/* finished searching the table without any match */
	res = RAINBOW_STATUS_NOT_FOUND;
	goto cleanup;
cleanup_success:
	res = RAINBOW_STATUS_OK;
	goto cleanup;
cleanup_error:
	res = RAINBOW_STATUS_ERROR;
cleanup:
	masrek_finalize(&masrek);
	return res;
}
