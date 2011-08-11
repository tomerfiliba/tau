#ifndef RAINBOW_H_INCLUDED
#define RAINBOW_H_INCLUDED

#include "misc.h"
#include "rules.h"
#include "deht.h"


#define RAINBOW_STATUS_OK         (0)
#define RAINBOW_STATUS_ERROR      (-1)
#define RAINBOW_STATUS_NOT_FOUND  (1)


uint64_t * rainbow_generate_seed_table(const char * prefix, const char * rand_seed, int chain_length);

uint64_t * rainbow_load_seed_table(const char * prefix, int chain_length);

int rainbow_generate_single_chain(const rule_info_t * rule, int chain_length, uint64_t k,
		const uint64_t * seed_table, char * first_password, unsigned char * last_digest);

int rainbow_query(DEHT * deht, const rule_info_t * rule, const uint64_t * seed_table,
				  int chain_length, int multi_query, const unsigned char * target_digest,
				  char * out_password);



#endif /* RAINBOW_H_INCLUDED */
