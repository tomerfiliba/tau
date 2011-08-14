#ifndef INILOADER_H_INCLUDED
#define INILOADER_H_INCLUDED

#include <stdio.h>
#include "misc.h"

#define INI_STATUS_OK       (0)
#define INI_STATUS_ERROR    (-1)


typedef struct {
	char key[MAX_INPUT_BUFFER];
	char value[MAX_INPUT_BUFFER];
} keyvalue_t;

typedef struct {
	int num_of_lines;
	keyvalue_t * lines;
} inifile_t;

int ini_load(inifile_t * ini, const char * filename);
const char * ini_get(const inifile_t * ini, const char * key);
int ini_get_integer(const inifile_t * ini, const char * key, int * value);
void ini_finalize(inifile_t * ini);

/****************************************************************************/

/* a struct that holds all the configuration parametes for this project 
 * (loaded from the INI file) */
typedef struct {
	char prefix[MAX_INPUT_BUFFER];
	char random_seed[MAX_INPUT_BUFFER];
	uint64_t * seed_table;

	/* rule parameters */
	char rule_pattern[MAX_INPUT_BUFFER];
	char lexicon_file[MAX_INPUT_BUFFER];
	char hash_name[MAX_INPUT_BUFFER];
	BasicHashFunctionPtr hash_func;
	int digest_size;

	/* rainbow table parameters */
	int chain_length;
	int num_of_buckets;
	int bucket_size;
	int num_of_query_results;
} config_t;

int config_load(config_t * config, const char * inifilename);
void config_finalize(config_t * config);


#endif /* INILOADER_H_INCLUDED */
