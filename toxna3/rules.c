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

/* ============================= DIGIT TERM ================================ */
static int digit_term_get_rec(int count, int k, char * output);

static int digit_term_get_limit(int count)
{
	/* sum of geometric series */
	return ((1 - longpow(10, count + 1)) / (-9)) - 1;
}

static int digit_term_get(int count, int k, char * output)
{
	if (k < 0 || k > digit_term_get_limit(count)) {
		return RULE_STATUS_ERROR;
	} else if (k == 0) {
		*output = '\0';
		return RULE_STATUS_OK;
	}
	return digit_term_get_rec(count, k - 1, output);
}

static int digit_term_get_rec(int count, int k, char * output)
{
	int max = longpow(10, count);
	if (k < max) {
		sprintf(output, "%d", k);
		return RULE_STATUS_OK;
	}
	*output = '0';
	return digit_term_get_rec(count - 1, k - max, output + 1);
}

/* ======================== LOWER-CASE LATIN TERM ========================== */
static int latin_term_get_rec(int count, int k, char * output);

static int latin_term_get_limit(int count)
{
	/* sum of geometric series */
	return (1 - longpow(26, count + 1)) / (-25) - 1;
}

static int latin_term_get(int count, int k, char * output)
{
	if (k < 0 || k > latin_term_get_limit(count)) {
		return RULE_STATUS_ERROR;
	} else if (k == 0) {
		*output = '\0';
		return RULE_STATUS_OK;
	}
	return latin_term_get_rec(count, k - 1, output);
}

static void to_base(int value, char offset, char radix, char * output)
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

static int latin_term_get_rec(int count, int k, char * output)
{
	int max = longpow(26, count);
	if (k < max) {
		to_base(k, 'a', 26, output);
		return RULE_STATUS_OK;
	} else {
		*output = 'a';
		return latin_term_get_rec(count - 1, k - max, output + 1);
	}
}

/* ========================= PRINTABLE ASCII TERM ========================== */
static int ascii_term_get_rec(int count, int k, char * output);

static int ascii_term_get_limit(int count)
{
	/* sum of geometric series. base: 126-32+1 = 95 */
	return (1 - longpow(95, count + 1)) / (-94) - 1;
}

static int ascii_term_get(int count, int k, char * output)
{
	if (k == 0) {
		*output = '\0';
		return RULE_STATUS_OK;
	} else if (k < 0 || k > ascii_term_get_limit(count)) {
		return RULE_STATUS_ERROR;
	}
	return ascii_term_get_rec(count, k - 1, output);
}

static int ascii_term_get_rec(int count, int k, char * output)
{
	int max = longpow(95, count);
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
        int count, int k, char * output);

static int lexicon_term_get_limit(int num_of_words, int count)
{
	/* sum of geometric series */
	return (1 - longpow(num_of_words, count + 1)) / (-num_of_words + 1) - 1;
}

static int lexicon_term_get(const char ** lexicon, int num_of_words, int count,
        int k, char * output)
{
	if (k < 0 || k > lexicon_term_get_limit(num_of_words, count)) {
		return RULE_STATUS_ERROR;
	} else if (k == 0) {
		*output = '\0';
		return RULE_STATUS_OK;
	}
	return lexicon_term_get_rec(lexicon, num_of_words, count, k - 1, output);
}

static int lexicon_term_get_rec(const char ** lexicon, int num_of_words,
        int count, int k, char * output)
{
	int max = longpow(num_of_words, count);
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
		for (ch = line; *ch != '\0'; ch++) {
			if (*ch == '\n') {
				*ch = '\0';
				break;
			}
		}

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
				ptrn->terms[j].limit = digit_term_get_limit(
				        ptrn->terms[j].count);
				ptrn->terms[j].max_size = ptrn->terms[j].count;
				break;
			case '.':
				ptrn->terms[j].type = TERM_LATIN;
				ptrn->terms[j].limit = latin_term_get_limit(
				        ptrn->terms[j].count);
				ptrn->terms[j].max_size = ptrn->terms[j].count;
				break;
			case '$':
				ptrn->terms[j].type = TERM_ASCII;
				ptrn->terms[j].limit = ascii_term_get_limit(
				        ptrn->terms[j].count);
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
		/*printf("%d count=%d, limit=%d\n", info->terms[j].type, info->terms[j].count, info->terms[j].limit);*/
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
static unsigned long rule_num_of_passwords_per_pattern(const rule_pattern_t * ptrn)
{
	int i;
	unsigned long limit = 1;

	for (i = 0; i < ptrn->num_of_terms; i++) {
		limit *= ptrn->terms[i].limit + 1;
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
		if (*ch == 1) {
			info->num_of_patterns += 1;
		}
	}

	/* allocate space */
	info->patterns = (rule_pattern_t*) malloc(sizeof(rule_pattern_t) * 
		info->num_of_patterns);
	if (info->patterns == NULL) {
		fprintf(stderr, "failed to allocated rule->patterns\n");
		return RULE_STATUS_ERROR;
	}
	info->pattern_offsets = (unsigned long*) malloc(sizeof(unsigned long) * 
		info->num_of_patterns);
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
			if (rule_load_single_pattern(&info->patterns[i], subpattern, 
					info->num_of_words, info->longest_word) !=  RULE_STATUS_OK) {
				goto cleanup;
			}
			info->pattern_offsets[i] = rule_num_of_passwords_per_pattern(&info->patterns[i]);
			i++;
		}
		else {
			subpattern[j++] = *ch;
		}
	}

	/* don't forget the last one */
	subpattern[j] = '\0';
	if (rule_load_single_pattern(&info->patterns[i], subpattern, 
			info->num_of_words, info->longest_word) !=  RULE_STATUS_OK) {
		goto cleanup;
	}
	info->pattern_offsets[i] = rule_num_of_passwords_per_pattern(&info->patterns[i]);
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
static unsigned long rule_num_of_passwords(const rule_info_t * info)
{
	int i;
	unsigned long total = 0;

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

	return max_length;
}

/*
 * API
 *
 * initializes the given rule object with the given parameters (which are loaded
 * from the INI file).
 * returns RULE_STATUS_OK on success, RULE_STATUS_ERROR on failure.
 */
int rule_init(rule_info_t * info, const char * pattern, const char * lexfilename, 
			  const char * hashname)
{
	info->num_of_patterns = 0;
	info->patterns = NULL;
	info->words = NULL;
	info->num_of_words = 0;
	info->longest_word = -1;

	if (strcasecmp(hashname, "md5") == 0) {
		strcpy(info->hashname, "MD5");
		info->hashfunc = MD5BasicHash;
	} else if (strcasecmp(hashname, "sha1") == 0) {
		strcpy(info->hashname, "SHA1");
		info->hashfunc = SHA1BasicHash;
	} else {
		/* invalid hash */
		fprintf(stderr, "Error: Hash \"%s\" is not supported\n", hashname);
		return RULE_STATUS_ERROR;
	}

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

int rule_get_kth_password_per_pattern(rule_info_t * info, rule_pattern_t * ptrn, 
									  unsigned long k, char * output)
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

int rule_kth_password(rule_info_t * info, unsigned long k, char * output, int output_length)
{
	int i;
	int succ;
	int j;

	if (output_length < rule_max_password_length(info)) {
		fprintf(stderr, "rule_generate_next_password: output buffer too small\n");
		return RULE_STATUS_ERROR;
	}

	return RULE_STATUS_OK;
}


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

/*
 * API
 *
 * initializes the given rule from an INI file. it loads the parameters from the
 * INI file and initializes the rule using rule_init().
 * returns RULE_STATUS_OK on success, RULE_STATUS_ERROR on failure.
 */
int rule_load_from_file(rule_info_t * info, const char * inifilename)
{
	char lexfilename[MAX_INPUT_BUFFER];
	char pattern[MAX_INPUT_BUFFER];
	char hashname[MAX_INPUT_BUFFER];
	char name[MAX_INPUT_BUFFER];
	char value[MAX_INPUT_BUFFER];
	char line[MAX_INPUT_BUFFER];
	FILE *f = fopen(inifilename, "r");

	if (f == NULL) {
		/*perror("rule_load_from_file: fopen failed");*/
		perror(inifilename);
		return RULE_STATUS_ERROR;
	}

	lexfilename[0] = '\0';
	pattern[0] = '\0';
	hashname[0] = '\0';

	while (1) {
		if (fgets(line, sizeof(line)-1, f) == NULL) {
			if (feof(f)) {
				break;
			}
			else {
				perror("rule_load_from_file: fgets failed");
				goto error_cleaup;
			}
		}
		if (is_comment_line(line)) {
			continue;
		}

		if (sscanf(line, "%s = %s", name, value) != 2) {
			fprintf(stderr, "Syntax error in INI file\n");
			goto error_cleaup;
		}
		if (strcasecmp(name, "rule") == 0) {
			strcpy(pattern, value);
		} else if (strcasecmp(name, "lexicon_name") == 0) {
			strcpy(lexfilename, value);
		} else if (strcasecmp(name, "hash_name") == 0) {
			strcpy(hashname, value);
		} else {
			/* instructions: ignore unknown keys in INI file */
		}
	}
	if (fclose(f) != 0) {
		/*perror("rule_load_from_file: fclose failed");*/
		perror(inifilename);
		return RULE_STATUS_ERROR;
	}

	if (lexfilename[0] == '\0') {
		fprintf(stderr, "INI file did not specify lexicon_name\n");
		return RULE_STATUS_ERROR;
	} else if (pattern[0] == '\0') {
		fprintf(stderr, "INI file did not specify rule\n");
		return RULE_STATUS_ERROR;
	} else if (hashname[0] == '\0') {
		fprintf(stderr, "INI file did not specify hash_name\n");
		return RULE_STATUS_ERROR;
	}

	return rule_init(info, pattern, lexfilename, hashname);

error_cleaup:
	fclose(f);
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
	int j;

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


