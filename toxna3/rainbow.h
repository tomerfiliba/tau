#ifndef RAINBOW_H_INCLUDED
#define RAINBOW_H_INCLUDED

#include "misc.h"
#include "rules.h"
#include "deht.h"


#define RAINBOW_STATUS_OK         (0)
#define RAINBOW_STATUS_ERROR      (-1)
#define RAINBOW_STATUS_NOT_FOUND  (1)

/*
 * generates the seed table, from the random seed given in the configuration.
 * the table will be stored inside the config struct (config->seed_table), 
 * and it will be freed by config_finalize(). also saves it to the seed file.
 *
 * Parameters:
 *    * config - the configuration struct
 * Returns: RAINBOW_STATUS_OK if the seed table was generated successfully; 
 *          RAINBOW_STATUS_ERROR on error.
 */
int rainbow_generate_seed_table(config_t * config);

/*
 * loads a previously generated seed table. the table will be stored
 * inside the config struct (config->seed_table), and it will be freed
 * by config_finalize().
 *
 * Parameters:
 *    * config - the configuration struct
 * Returns: RAINBOW_STATUS_OK if the seed table was loaded successfully; 
 *          RAINBOW_STATUS_ERROR on error.
 */
int rainbow_load_seed_table(config_t * config);

/*
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
								  unsigned char * last_digest);

/*
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
				  const unsigned char * target_digest, char * output, int max_output);


#endif /* RAINBOW_H_INCLUDED */
