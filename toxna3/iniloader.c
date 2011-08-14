#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#ifdef _MSC_VER
	#define strcasecmp _stricmp
#else
	#include <strings.h>
#endif
#include "iniloader.h"


/*
 * checks for comment and empty lines in the INI file
 */
static int is_comment_line(const char * text)
{
	const char *p = text;
	for (; *p != '\0'; p++) {
		if (isspace(*p)) {
			continue;
		}
		if (*p == ';') {
			return 1;
		}
		return 0;
	}
	return 1;
}

static int parse_line(char * line, char ** key, char ** value)
{
	char * ch, * rkey;

	/* parse line */
	*key = line;
	rkey = NULL;
	*value = NULL;
	for (ch = line; ch != '\0'; ch++) {
		if (*ch == '=') {
			rkey = ch - 1;
			*ch = '\0';
			*value = ch + 1;
			break;
		}
	}
	if (*value == NULL) {
		fprintf(stderr, "Syntax error in INI file: '%s'\n", line);
		return INI_STATUS_ERROR;
	}

	/* trim whitespace */
	for (; **key == ' ' || **key == '\t'; (*key)++);
	for (; *rkey == ' ' || *rkey == '\t'; *(rkey--) = '\0');
	for (; **value == ' ' || **value == '\t'; (*value)++);
	for (ch = *value + strlen(*value) - 1;
			*ch == ' ' || *ch == '\t' || *ch == '\n' || *ch == '\r'; *(ch--) = '\0');

	return INI_STATUS_OK;
}

int ini_load(inifile_t * ini, const char * filename)
{
	char line[MAX_INPUT_BUFFER];
	char * key, * value;
	int allocated_lines = 50;
	keyvalue_t * tmp = NULL;

	FILE * f = fopen(filename, "r");

	if (f == NULL) {
		perror(filename);
		return INI_STATUS_ERROR;
	}

	ini->num_of_lines = 0;
	ini->lines = (keyvalue_t*)malloc(sizeof(keyvalue_t) * allocated_lines);
	if (ini->lines == NULL) {
		fprintf(stderr, "allocating ini->lines failed\n");
	}

	while (1) {
		if (fgets(line, sizeof(line)-1, f) == NULL) {
			if (feof(f)) {
				break;
			}
			else {
				perror("ini_load: fgets failed");
				goto cleanup;
			}
		}
		if (is_comment_line(line)) {
			continue;
		}
		if (parse_line(line, &key, &value) != INI_STATUS_OK) {
			goto cleanup;
		}

		/* grow lines array if needed */
		if (ini->num_of_lines >= allocated_lines) {
			allocated_lines *= 2;
			tmp = realloc(ini->lines, sizeof(keyvalue_t) * allocated_lines);
			if (tmp == NULL) {
				fprintf(stderr, "reallocating ini->lines failed\n");
				goto cleanup;
			}
			ini->lines = tmp;
		}

		strncpy(ini->lines[ini->num_of_lines].key, key, MAX_INPUT_BUFFER);
		strncpy(ini->lines[ini->num_of_lines].value, value, MAX_INPUT_BUFFER);
		ini->num_of_lines += 1;
	}
	fclose(f);

	return INI_STATUS_OK;

cleanup:
	if (ini->lines != NULL) {
		free(ini->lines);
		ini->lines = NULL;
	}
	return INI_STATUS_ERROR;
}

const char * ini_get(const inifile_t * ini, const char * key)
{
	int i;

	for (i = 0; i < ini->num_of_lines; i++) {
		if (strcasecmp(ini->lines[i].key, key) == 0) {
			return ini->lines[i].value;
		}
	}
	return NULL;
}

int ini_get_integer(const inifile_t * ini, const char * key, int * value)
{
	const char * text = ini_get(ini, key);
	if (text == NULL) {
		return INI_STATUS_ERROR;
	}
	*value = atoi(text);
	return INI_STATUS_OK;
}


void ini_finalize(inifile_t * ini)
{
	ini->num_of_lines = 0;
	if (ini->lines != NULL) {
		free(ini->lines);
		ini->lines = NULL;
	}
}

#define INI_GET_STR(key, dest)	if ((str_value = ini_get(&ini, key)) == NULL) { \
									fprintf(stderr, "INI file does not specify '%s'\n", key); \
									goto cleanup; \
								} \
								strncpy(dest, str_value, sizeof(dest));

#define INI_GET_NUM(key, dest)	if ((ini_get_integer(&ini, key, &dest)) != INI_STATUS_OK) { \
									fprintf(stderr, "INI file does not specify '%s'\n", key); \
									goto cleanup; \
								} \
								if (dest <= 0) { \
									fprintf(stderr, "INI: '%s' must be >= 1'", key); \
									goto cleanup; \
								}

int config_load(config_t * config, const char * prefix)
{
	int res = INI_STATUS_ERROR;
	const char * str_value;
	char inifilename[MAX_INPUT_BUFFER];
	inifile_t ini;

	config->seed_table = NULL;

	strncpy(inifilename, prefix, sizeof(inifilename) - 5);
	strcat(inifilename, ".ini");
	if (ini_load(&ini, inifilename) != INI_STATUS_OK) {
		/* error message printed by ini_load */
		return INI_STATUS_ERROR;
	}

	strncpy(config->prefix, prefix, sizeof(config->prefix));

	INI_GET_STR("rule", config->rule_pattern);
	INI_GET_STR("lexicon_name", config->lexicon_file);
	INI_GET_STR("hash_name", config->hash_name);
	INI_GET_STR("main_rand_seed", config->random_seed);

	INI_GET_NUM("num_of_R", config->chain_length);
	INI_GET_NUM("multi_query", config->num_of_query_results);
	INI_GET_NUM("bucket_size", config->bucket_size);
	INI_GET_NUM("hash_size", config->num_of_buckets);

	if (strcasecmp(config->hash_name, "MD5") == 0) {
		config->hash_func = MD5BasicHash;
		config->digest_size = MD5_OUTPUT_LENGTH_IN_BYTES;
	}
	else if (strcasecmp(config->hash_name, "SHA1") == 0) {
		config->hash_func = SHA1BasicHash;
		config->digest_size = SHA1_OUTPUT_LENGTH_IN_BYTES;
	}
	else {
		fprintf(stderr, "INI: hash_name must be 'MD5' or 'SHA1'");
		goto cleanup;
	}

	res = INI_STATUS_OK;

cleanup:
	ini_finalize(&ini);
	return res;
}

#undef INI_GET_STR
#undef INI_GET_NUM

void config_finalize(config_t * config)
{
	if (config->seed_table != NULL) {
		free(config->seed_table);
		config->seed_table = NULL;
	}
}

