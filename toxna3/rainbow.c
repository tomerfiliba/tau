#include "rainbow.h"
#include <string.h>
#include <stdlib.h>


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


static uint64_t rainbow_reduce(const rule_info_t * rule, uint64_t seed, unsigned char * digest)
{
	/* the input digest has a high-enough entropy, and so does the seed,
       so we might as well just multiply them... no need for more complex
	   reduction methods */
	uint64_t h = *((uint64_t*)digest);
	return (h * seed) % rule->num_of_passwords;
}

static int rainbow_compute_chain(const config_t * config, const rule_info_t * rule,
								 int start, int upper_bound, unsigned char * digest)
{
	int i;
	uint64_t k;
	char password[MAX_INPUT_BUFFER];

	for (i = start; i < upper_bound; i++) {
		k = rainbow_reduce(rule, config->seed_table[i], digest);
		if (rule_kth_password(rule, k, password, sizeof(password), 0) != RULE_STATUS_OK) {
			/* error message printed by rule_kth_password */
			return RAINBOW_STATUS_ERROR;
		}
		config->hash_func((unsigned char*)password, strlen(password), digest);
	}
	return RAINBOW_STATUS_OK;
}

int rainbow_generate_single_chain(const config_t * config, const rule_info_t * rule,
								  uint64_t k, char * first_password, int max_password,
								  unsigned char * last_digest)
{
	if (rule_kth_password(rule, k, first_password, max_password, 0) !=
			RULE_STATUS_OK) {
		/* error message printed by rule_kth_password */
		return 1;
	}
	config->hash_func((unsigned char*)first_password, strlen(first_password), last_digest);
	return rainbow_compute_chain(config, rule, 0, config->chain_length, last_digest);
}

/****************************************************************************/

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

int rainbow_query(const config_t * config, const rule_info_t * rule, DEHT * deht,
				  const unsigned char * target_digest, char * output, int max_output)
{
	int j, k, max_size;
	int res;
	int num_of_matches;
	masrek_t masrek;
	unsigned char digest[MAX_DIGEST_LENGTH_IN_BYTES];

	if (masrek_init(&masrek, config->num_of_query_results) != 0) {
		/* error message printed by masrek_init */
		return 1;
	}

	/* iterate over all starting points in the chain */
	for (j = config->chain_length; j >= 0; j--) {
		memcpy(digest, target_digest, config->digest_size);
		if (rainbow_compute_chain(config, rule, j, config->chain_length, digest) != RAINBOW_STATUS_OK) {
			/* error message printed by rainbow_compute_chain */
			goto cleanup_error;
		}

		/* find all possible starting points */
		num_of_matches = multi_query_DEHT(deht, digest, config->digest_size, &masrek);
		if (num_of_matches < 0) {
			/* error message printed by multi_query_DEHT */
			goto cleanup_error;
		}

		for (k = 0; k < num_of_matches; k++) {
			config->hash_func((unsigned char*)masrek.items[k].buffer,
				masrek.items[k].length, digest);

			if (rainbow_compute_chain(config, rule, 0, j, digest) != RAINBOW_STATUS_OK) {
				/* error message printed by rainbow_compute_chain */
				goto cleanup_error;
			}

			if (memcmp(digest, target_digest, config->digest_size) == 0) {
				/* found a matching password */
				max_size = (masrek.items[k].length < max_output) ? masrek.items[k].length : max_output;
				memcpy(output, masrek.items[k].buffer, max_size);
				output[max_size] = '\0';
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
