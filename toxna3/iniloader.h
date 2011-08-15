#ifndef INILOADER_H_INCLUDED
#define INILOADER_H_INCLUDED

#include <stdio.h>
#include "misc.h"

#define INI_STATUS_OK       (0)
#define INI_STATUS_ERROR    (-1)


/* represents a key-value pair in the INI file */
typedef struct {
	char key[MAX_INPUT_BUFFER];      /* key buffer */
	char value[MAX_INPUT_BUFFER];    /* value buffer */
} keyvalue_t;

/* represents the enitre INI file parameters */
typedef struct {
	int num_of_lines;                /* number of lines (parameters) in INI file */
	keyvalue_t * lines;              /* the lines array */
} inifile_t;

/*
 * initializes an INI-file holder object, loaded from the given file. this function
 * reads the entire file, parses it, and stores it as a key-value array
 */
int ini_load(inifile_t * ini, const char * filename);

/*
 * returns the value associated with the given key. note that a CONST pointer to 
 * the data is returned. returns NULL if the key was not found. keys are NOT 
 * case sensitive.
 */
const char * ini_get(const inifile_t * ini, const char * key);

/*
 * returns an integer value associated with the given key. if the key was not
 * found, returns INI_STATUS_ERROR; otherwise returns INI_STATUS_OK and sets
 * the `value` argument to the integral value of the string.
 */
int ini_get_integer(const inifile_t * ini, const char * key, int * value);

/*
 * releases all resources associated with the INI file holder object
 */
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

/*
 * loads the project's configuration from the INI file specified by `prefix`.
 * the INI file is `prefix`.ini.
 *
 * if some required parameters is missing, or if its value is illegal, a message
 * is printed and the functions returns INI_STATUS_ERROR. otherwise, the function
 * returns INI_STATUS_OK.
 *
 * the config argument will be populated with the values of the parameters
 */
int config_load(config_t * config, const char * inifilename);

/*
 * releases all resources held by the config object
 */
void config_finalize(config_t * config);


#endif /* INILOADER_H_INCLUDED */
