#ifndef RULES_H_INCLUDED
#define RULES_H_INCLUDED

#include "misc.h"

#define RULE_STATUS_EXHAUSTED  (-2)
#define RULE_STATUS_ERROR      (-1)
#define RULE_STATUS_OK         (0)


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
	int max_size;
} term_info_t;

typedef struct
{
	int num_of_terms;
	term_info_t * terms;
	int num_of_words;
	int longest_word;
	char ** words;
	BasicHashFunctionPtr hashfunc;
	int digest_size;
	char hashname[10];
	int limit; /* the "flag"; -1 means "all" */
	int remaining;
} rule_info_t;

int rule_load_from_file(rule_info_t * info, const char * inifilename);
int rule_load(rule_info_t * info, const char * pattern,
        const char * lexfilename, const char * hashname, const char * flag);
void rule_finalize(rule_info_t * info);

unsigned long rule_num_of_passwords(rule_info_t * info);
int rule_max_password_length(rule_info_t * info);
int rule_generate_next_password(rule_info_t * info, char * output, int output_length);

#endif /* RULES_H_INCLUDED */
