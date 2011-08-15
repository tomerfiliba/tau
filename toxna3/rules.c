#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef _MSC_VER
	#define strcasecmp _stricmp
#else
	#include <strings.h>
#endif
#include "rules.h"
#include "iniloader.h"



static void to_base(uint64_t value, char offset, char radix, char * output)
{
	*output = offset + (value % radix);
	value /= radix;
	output++;
	while (value > 0) {
		*output = offset + (value % radix);
		value /= radix;
		output++;
	}
	*output = '\0';
}

/* ============================= DIGIT TERM ================================ */
static int digit_term_get_rec(int count, uint64_t k, char * output);

static uint64_t digit_term_get_limit(int count)
{
	/* sum of geometric series */
	return (1 - longpow(10, count + 1)) / (-9);
}

static int digit_term_get(int count, uint64_t k, char * output)
{
	if (k == 0) {
		*output = '\0';
		return RULE_STATUS_OK;
	}
	return digit_term_get_rec(count, k - 1, output);
}

static int digit_term_get_rec(int count, uint64_t k, char * output)
{
	uint64_t max = longpow(10, count);
	if (k < max) {
		to_base(k, '0', 10, output);
		return RULE_STATUS_OK;
	}
	*output = '0';
	return digit_term_get_rec(count - 1, k - max, output + 1);
}

/* ======================== LOWER-CASE LATIN TERM ========================== */
static int latin_term_get_rec(int count, uint64_t k, char * output);

static uint64_t latin_term_get_limit(int count)
{
	/* sum of geometric series */
	return (1 - longpow(26, count + 1)) / (-25);
}

static int latin_term_get(int count, uint64_t k, char * output)
{
	if (k == 0) {
		*output = '\0';
		return RULE_STATUS_OK;
	}
	return latin_term_get_rec(count, k - 1, output);
}

static int latin_term_get_rec(int count, uint64_t k, char * output)
{
	uint64_t max = longpow(26, count);
	if (k < max) {
		to_base(k, 'a', 26, output);
		return RULE_STATUS_OK;
	} else {
		*output = 'a';
		return latin_term_get_rec(count - 1, k - max, output + 1);
	}
}

/* ========================= PRINTABLE ASCII TERM ========================== */
static int ascii_term_get_rec(int count, uint64_t k, char * output);

static uint64_t ascii_term_get_limit(int count)
{
	/* sum of geometric series. base: 126-32+1 = 95 */
	return (1 - longpow(95, count + 1)) / (-94);
}

static int ascii_term_get(int count, uint64_t k, char * output)
{
	if (k == 0) {
		*output = '\0';
		return RULE_STATUS_OK;
	}
	return ascii_term_get_rec(count, k - 1, output);
}

static int ascii_term_get_rec(int count, uint64_t k, char * output)
{
	uint64_t max = longpow(95, count);
	if (k < max) {
		to_base(k, ' ', 95, output);
		return RULE_STATUS_OK;
	} else {
		*output = ' ';
		return ascii_term_get_rec(count - 1, k - max, output + 1);
	}
}

/* ============================== LEXICON TERM ============================= */
static int lexicon_term_get_rec(const char ** lexicon, int num_of_words,
        int count, uint64_t k, char * output);

static uint64_t lexicon_term_get_limit(int num_of_words, int count)
{
	/* sum of geometric series */
	return (1 - longpow(num_of_words, count + 1)) / (-num_of_words + 1);
}

static int lexicon_term_get(const char ** lexicon, int num_of_words, int count,
		uint64_t k, char * output)
{
	if (k == 0) {
		*output = '\0';
		return RULE_STATUS_OK;
	}
	return lexicon_term_get_rec(lexicon, num_of_words, count, k - 1, output);
}

static int lexicon_term_get_rec(const char ** lexicon, int num_of_words,
        int count, uint64_t k, char * output)
{
	uint64_t max = longpow(num_of_words, count);
	const char * word;
	if (k < max) {
		word = lexicon[k % num_of_words];
		strcpy(output, word);
		output += strlen(word);
		k /= num_of_words;
		while (k > 0) {
			word = lexicon[k % num_of_words];
			strcpy(output, word);
			output += strlen(word);
			k /= num_of_words;
		}
		return RULE_STATUS_OK;
	} else {
		strcpy(output, lexicon[0]);
		return lexicon_term_get_rec(lexicon, num_of_words, count - 1, k - max,
		        output + strlen(lexicon[0]));
	}
}

/* ========================================================================= */

/*
 * load the rule's lexicon into the rule info
 */
static int rule_load_lexicon(rule_info_t * info, const char* filename)
{
	char line[MAX_INPUT_BUFFER];
	int i;
	int allocated_indexes = 0;
	char ** newbuf = NULL;
	char * ch;
	int length;
	FILE * f = fopen(filename, "r");

	info->num_of_words = 0;
	info->words = NULL;
	info->longest_word = 0;

	if (f == NULL) {
		/*perror("rule_load_lexicon: fopen of lexicon file failed");*/
		perror(filename);
		return RULE_STATUS_ERROR;
	}

	while (1) {
		if (fgets(line, sizeof(line), f) == NULL) {
			break; /* EOF */
		}

		for (ch = line + strlen(line) - 1; *ch == '\n' || *ch == '\r'; *(ch--) = '\0');

		if (info->num_of_words >= allocated_indexes) {
			allocated_indexes += 1000;
			newbuf = realloc(info->words, sizeof(char**) * allocated_indexes);
			if (newbuf == NULL) {
				fprintf(stderr, "could not grow words array\n");
				goto error_cleanup;
			}
			info->words = (char**) newbuf;
		}

		length = strlen(line);
		info->words[info->num_of_words] = (char*) malloc(length + 1);
		if (info->words[info->num_of_words] == NULL) {
			fprintf(stderr, "could not allocate space for word\n");
			goto error_cleanup;
		}
		strcpy(info->words[info->num_of_words], line);
		if (length > info->longest_word) {
			info->longest_word = length;
		}
		info->num_of_words++;
	}

	if (fclose(f) != 0) {
		/* fclose failed */
		/*perror("rule_load_lexicon: fclose failed");*/
		perror(filename);
		return RULE_STATUS_ERROR;
	}

	return RULE_STATUS_OK;

error_cleanup:
	for (i = 0; i < info->num_of_words; i++) {
		free(info->words[i]);
	}
	if (info->words != NULL) {
		free(info->words);
	}
	info->num_of_words = 0;
	info->words = NULL;
	fclose(f);
	return RULE_STATUS_ERROR;
}

/*
 * loads the rule's pattern -- does some preprocessing and stores it
 * efficiently as terms
 */
static int rule_load_single_pattern(rule_pattern_t * ptrn, const char* pattern,
									int num_of_words, int longest_word)
{
	const int pattern_len = strlen(pattern);
	int i, j;

	if (pattern_len % 2 != 0) {
		/* invalid syntax */
		fprintf(stderr, "Error: rule \"%s\" does not fit syntax.\n", pattern);
		return RULE_STATUS_ERROR;
	}
	ptrn->num_of_terms = pattern_len / 2;
	ptrn->terms = (term_info_t*) malloc(sizeof(term_info_t)
	        * ptrn->num_of_terms);
	if (ptrn->terms == NULL) {
		fprintf(stderr, "could not allocate space for terms array\n");
		goto error_cleanup;
	}

	for (i = j = 0; i < pattern_len; i += 2, j++) {
		ptrn->terms[j].count = pattern[i + 1] - '0';
		switch (pattern[i]) {
			case '#':
				ptrn->terms[j].type = TERM_DIGIT;
				ptrn->terms[j].limit = digit_term_get_limit(ptrn->terms[j].count);
				ptrn->terms[j].max_size = ptrn->terms[j].count;
				break;
			case '.':
				ptrn->terms[j].type = TERM_LATIN;
				ptrn->terms[j].limit = latin_term_get_limit(ptrn->terms[j].count);
				ptrn->terms[j].max_size = ptrn->terms[j].count;
				break;
			case '$':
				ptrn->terms[j].type = TERM_ASCII;
				ptrn->terms[j].limit = ascii_term_get_limit(ptrn->terms[j].count);
				ptrn->terms[j].max_size = ptrn->terms[j].count;
				break;
			case '@':
				ptrn->terms[j].type = TERM_LEXICON;
				ptrn->terms[j].limit = lexicon_term_get_limit(num_of_words,
				        ptrn->terms[j].count);
				ptrn->terms[j].max_size = ptrn->terms[j].count * longest_word;
				break;
			default:
				/* invalid syntax */
				fprintf(stderr, "Error: rule \"%s\" does not fit syntax.\n", pattern);
				goto error_cleanup;
		}
		/*printf("    * type=%c (%d) count=%d, limit=%llu max_size=%d\n", pattern[i],
				ptrn->terms[j].type, ptrn->terms[j].count, ptrn->terms[j].limit,
				ptrn->terms[j].max_size);*/
	}

	return RULE_STATUS_OK;

error_cleanup:
	if (ptrn->terms != NULL) {
		free(ptrn->terms);
	}
	ptrn->num_of_terms = 0;
	ptrn->terms = NULL;
	return RULE_STATUS_ERROR;
}

/*
 * returns the number of password that this rule may generate. note that there might
 * be some little overlap between passwords, which are counted separately. for example,
 * the rule .1.1 might generate "a" + "\0" = "a" and "\0" + "a" = "a" -- but these
 * two are counted twice. all in all, this happens for a very small percentage of the
 * password space.
 */
static uint64_t rule_num_of_passwords_per_pattern(const rule_pattern_t * ptrn)
{
	int i;
	uint64_t limit = 1;

	for (i = 0; i < ptrn->num_of_terms; i++) {
		limit *= ptrn->terms[i].limit;
	}
	return limit;
}

static int rule_load_multiple_patterns(rule_info_t * info, const char * pattern)
{
	int i, j;
	const char * ch;
	char subpattern[MAX_INPUT_BUFFER];

	/* count subpatterns */
	info->num_of_patterns = 1;
	for (ch = pattern; *ch != '\0'; ch++) {
		if (*ch == '&') {
			info->num_of_patterns += 1;
		}
	}

	/*printf("info->num_of_patterns = %d\n", info->num_of_patterns);*/

	/* allocate space */
	info->patterns = (rule_pattern_t*) malloc(sizeof(rule_pattern_t) *
		info->num_of_patterns);
	if (info->patterns == NULL) {
		fprintf(stderr, "failed to allocated rule->patterns\n");
		return RULE_STATUS_ERROR;
	}
	info->pattern_offsets = (uint64_t*) malloc(sizeof(uint64_t) * info->num_of_patterns);
	if (info->pattern_offsets == NULL) {
		fprintf(stderr, "failed to allocated rule->pattern_offsets\n");
		free(info->patterns);
		return RULE_STATUS_ERROR;
	}

	/* load subpatterns, one at a time */
	i = 0;
	j = 0;
	for (ch = pattern; *ch != '\0'; ch++) {
		if (*ch == '&') {
			subpattern[j] = '\0';
			j = 0;
			/*printf("subpattern[%d] = %s\n", i, subpattern);*/
			if (rule_load_single_pattern(&info->patterns[i], subpattern,
					info->num_of_words, info->longest_word) !=  RULE_STATUS_OK) {
				goto cleanup;
			}
			info->pattern_offsets[i] = rule_num_of_passwords_per_pattern(&info->patterns[i]);
			/*printf("    space = %llu\n", info->pattern_offsets[i]);*/
			i++;
		}
		else {
			subpattern[j++] = *ch;
		}
	}

	/* don't forget the last one */
	subpattern[j] = '\0';
	/*printf("subpattern[%d] = %s\n", i, subpattern);*/
	if (rule_load_single_pattern(&info->patterns[i], subpattern,
			info->num_of_words, info->longest_word) !=  RULE_STATUS_OK) {
		goto cleanup;
	}
	info->pattern_offsets[i] = rule_num_of_passwords_per_pattern(&info->patterns[i]);
	/*printf("    space = %llu\n", info->pattern_offsets[i]);*/
	return RULE_STATUS_OK;

cleanup:
	free(info->patterns);
	info->patterns = NULL;
	free(info->pattern_offsets);
	info->pattern_offsets = NULL;
	return RULE_STATUS_ERROR;
}

/*
 *
 */
static uint64_t rule_num_of_passwords(const rule_info_t * info)
{
	int i;
	uint64_t total = 0;

	for (i = 0; i < info->num_of_patterns; i++) {
		total += info->pattern_offsets[i];
	}
	return total;
}


/*
 * returns the maximal length of a password generated by this rule
 */
static int rule_max_password_length(const rule_info_t * info)
{
	int i, j;
	int max_length = 0;
	int curr_length = 0;

	for (j = 0; j < info->num_of_patterns; j++) {
		curr_length = 0;
		for (i = 0; i < info->patterns[j].num_of_terms; i++) {
			curr_length += info->patterns[j].terms[i].max_size;
		}
		if (curr_length > max_length) {
			max_length = curr_length;
		}
	}

	/* plus 1 for terminating NUL */
	return max_length + 1;
}

/*
 * API
 *
 * initializes the given rule object with the given parameters (which are loaded
 * from the INI file).
 * returns RULE_STATUS_OK on success, RULE_STATUS_ERROR on failure.
 */
int rule_init(rule_info_t * info, const char * pattern, const char * lexfilename)
{
	info->num_of_patterns = 0;
	info->patterns = NULL;
	info->words = NULL;
	info->num_of_words = 0;
	info->longest_word = -1;

	if (rule_load_lexicon(info, lexfilename) != RULE_STATUS_OK) {
		return RULE_STATUS_ERROR;
	}
	if (rule_load_multiple_patterns(info, pattern) != RULE_STATUS_OK) {
		return RULE_STATUS_ERROR;
	}
	info->num_of_passwords = rule_num_of_passwords(info);
	info->longest_password = rule_max_password_length(info);
	return RULE_STATUS_OK;
}

static int rule_get_kth_password_per_pattern(const rule_info_t * info, 
		rule_pattern_t * ptrn, uint64_t k, char * output)
{
	int i;
	int j;
	int succ;

	for (i = 0; i < ptrn->num_of_terms; i++) {
		j = (int)(k % ptrn->terms[i].limit);
		k /= ptrn->terms[i].limit;

		switch (ptrn->terms[i].type) {
			case TERM_DIGIT:
				succ = digit_term_get(ptrn->terms[i].count, j, output);
				break;
			case TERM_LATIN:
				succ = latin_term_get(ptrn->terms[i].count, j, output);
				break;
			case TERM_ASCII:
				succ = ascii_term_get(ptrn->terms[i].count, j, output);
				break;
			case TERM_LEXICON:
				succ = lexicon_term_get((const char**) info->words,
						info->num_of_words, ptrn->terms[i].count, j, output);
				break;
		}
		if (succ != 0) {
			fprintf(stderr, "get next term failed\n");
			return RULE_STATUS_ERROR;
		}
		output += strlen(output);
	}
	return RULE_STATUS_OK;
}

/*
 * API
 *
 * returns the k'th password in the password space defined by this rule
 * if allow_empty is 0, this function will not return empty passwords
 * (it will choose a different k until the password is non-empty). if you
 * set this argument to 1, you may get empty passwords.
 */
int rule_kth_password(const rule_info_t * info, uint64_t k, char * output,
					  int output_length, int allow_empty)
{
	int i;

	if (output_length < info->longest_password) {
		fprintf(stderr, "rule_kth_password: output buffer too small\n");
		return RULE_STATUS_ERROR;
	}

	output[0] = '\0';
	while (output[0] == '\0') {
		for (i = 0; i < info->num_of_patterns; i++) {
			if (k < info->pattern_offsets[i]) {
				return rule_get_kth_password_per_pattern(info, &info->patterns[i], k, output);
			}
			else {
				k -= info->pattern_offsets[i];
			}
		}
		/* if we allow the empty password, just break, otherwise loop until we get a
		   non-empty password */
		if (allow_empty) {
			break;
		}
		else {
			k = (k * 593 + 1) % info->num_of_passwords;
		}
	}

	/* k too large */
	fprintf(stderr, "rule_kth_password: k out of bounds of the password space\n");
	return RULE_STATUS_ERROR;
}

/*
 * API
 *
 * releases all resources held by this rule object
 */
void rule_finalize(rule_info_t * info)
{
	int i;

	if (info->patterns != NULL) {
		for (i = 0; i < info->num_of_patterns; i++) {
			free(info->patterns[i].terms);
		}
		free(info->patterns);
		info->patterns = NULL;
	}

	if (info->words != NULL) {
		for (i = 0; i < info->num_of_words; i++) {
			free(info->words[i]);
		}
		free(info->words);
		info->words = NULL;
	}
}




