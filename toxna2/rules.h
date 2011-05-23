#ifndef RULES_H_INCLUDED
#define RULES_H_INCLUDED

#include "misc.h"

typedef enum
{
	TERM_DIGIT, TERM_LATIN, TERM_ASCII, TERM_LEXICON
} term_type_t;

typedef struct
{
	term_type_t type;
	int count;
	int k;
	int limit;
} term_info_t;

typedef struct
{
	int num_of_terms;
	term_info_t * terms;
	int num_of_words;
	char ** words;
	BasicHashFunctionPtr hashfunc;
	int limit; /* the "flag"; -1 means "all" */
	int remaining;
} rule_info_t;

int rule_load_from_file(rule_info_t * info, const char * inifilename);
int rule_load(rule_info_t * info, const char * pattern,
        const char * lexfilename, const char * hashname, const char * flag);
unsigned long rule_limit(rule_info_t * info);
int rule_generate_password(rule_info_t * info, char * output);

#endif /* RULES_H_INCLUDED */
