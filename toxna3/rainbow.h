#ifndef RAINBOW_H_INCLUDED
#define RAINBOW_H_INCLUDED

#include "misc.h"
#include "rules.h"
#include "deht.h"


#define RAINBOW_STATUS_OK         (0)
#define RAINBOW_STATUS_ERROR      (-1)
#define RAINBOW_STATUS_NOT_FOUND  (1)


int rainbow_generate_seed_table(config_t * config);
int rainbow_load_seed_table(config_t * config);

int rainbow_generate_single_chain(const config_t * config, const rule_info_t * rule,
								  uint64_t k, char * first_password, int max_password,
								  unsigned char * last_digest);

int rainbow_query(const config_t * config, const rule_info_t * rule, DEHT * deht,
				  const unsigned char * target_digest, char * output, int max_output);


#endif /* RAINBOW_H_INCLUDED */
