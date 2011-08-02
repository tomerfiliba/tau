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
static int rule_load_pattern(rule_info_t * info, const char* pattern)
{
	const int pattern_len = strlen(pattern);
	int i, j;

	if (pattern_len % 2 != 0) {
		/* invalid syntax */
		fprintf(stderr, "Error: rule \"%s\" does not fit syntax.\n", pattern);
		return RULE_STATUS_ERROR;
	}
	info->num_of_terms = pattern_len / 2;
	info->terms = (term_info_t*) malloc(sizeof(term_info_t)
	        * info->num_of_terms);
	if (info->terms == NULL) {
		fprintf(stderr, "could not allocate space for terms array\n");
		goto error_cleanup;
	}

	for (i = j = 0; i < pattern_len; i += 2, j++) {
		info->terms[j].k = 0;
		info->terms[j].count = pattern[i + 1] - '0';
		switch (pattern[i]) {
			case '#':
				info->terms[j].type = TERM_DIGIT;
				info->terms[j].limit = digit_term_get_limit(
				        info->terms[j].count);
				info->terms[j].max_size = info->terms[j].count;
				break;
			case '.':
				info->terms[j].type = TERM_LATIN;
				info->terms[j].limit = latin_term_get_limit(
				        info->terms[j].count);
				info->terms[j].max_size = info->terms[j].count;
				break;
			case '$':
				info->terms[j].type = TERM_ASCII;
				info->terms[j].limit = ascii_term_get_limit(
				        info->terms[j].count);
				info->terms[j].max_size = info->terms[j].count;
				break;
			case '@':
				info->terms[j].type = TERM_LEXICON;
				info->terms[j].limit = lexicon_term_get_limit(
				        info->num_of_words, info->terms[j].count);
				info->terms[j].max_size = info->terms[j].count * info->longest_word;
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
	if (info->terms != NULL) {
		free(info->terms);
	}
	info->num_of_terms = 0;
	info->terms = NULL;
	return RULE_STATUS_ERROR;
}

/*
 * API
 *
 * initializes the given rule object with the given parameters (which are loaded 
 * from the INI file).
 * returns RULE_STATUS_OK on success, RULE_STATUS_ERROR on failure.
 */
int rule_init(rule_info_t * info, const char * pattern,
        const char * lexfilename, const char * hashname, const char * flag)
{
	if (strcasecmp(flag, "all") == 0) {
		info->limit = -1;
	} else {
		if (sscanf(flag, "%d", &info->limit) != 1) {
			/* invalid flag */
			fprintf(stderr, "Error: flag \"%s\" is not supported\n", flag);
			return RULE_STATUS_ERROR;
		}
	}
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
	info->remaining = info->limit;
	if (rule_load_lexicon(info, lexfilename) != 0) {
		return RULE_STATUS_ERROR;
	}
	if (rule_load_pattern(info, pattern) != 0) {
		return RULE_STATUS_ERROR;
	}
	return RULE_STATUS_OK;
}

/*
 * returns the number of password that this rule may generate. note that there might
 * be some little overlap between passwords, which are counted separately. for example,
 * the rule .1.1 might generate "a" + "\0" = "a" and "\0" + "a" = "a" -- but these
 * two are counted twice. all in all, this happens for a very small percentage of the
 * password space.
 */
unsigned long rule_num_of_passwords(rule_info_t * info)
{
	int i;
	unsigned long limit = 1;

	if (info->limit > 0) {
		return info->limit;
	}
	for (i = 0; i < info->num_of_terms; i++) {
		limit *= (info->terms[i].limit + 1);
	}

	return limit;
}

/*
 * returns the maximal length of a password generated by this rule
 */
int rule_max_password_length(rule_info_t * info)
{
	int i;
	int max_length = 0;

	for (i = 0; i < info->num_of_terms; i++) {
		max_length += info->terms[i].max_size;
	}

	return max_length;
}

/* 
 * implements flag=ALL -- goes over the entire password space, updating
 * the k's in the terms each time.
 */
static int rule_generate_incrementing(rule_info_t * info, char * output)
{
	int i;
	int succ;
	char * password = output;

	password[0] = '\0';

	/* instructions:
	 * Notice, however, that even though each substring in the rule can be an empty sequence, 
	 * the entire rule must not be empty, that is, an empty password which theoretically can be 
	 * invoked by a rule is illegal! 
	 */
	while (password[0] == '\0') {
		/* this loop will break only if the generated password is not empty */

		for (i = 0; i < info->num_of_terms; i++) {
			if (info->terms[i].k > info->terms[i].limit) {
				info->terms[i].k = 0;
				if (i == info->num_of_terms - 1) {
					/* exhausted all terms */
					return RULE_STATUS_EXHAUSTED;
				}
				info->terms[i + 1].k += 1;
			}

			switch (info->terms[i].type) {
				case TERM_DIGIT:
					succ = digit_term_get(info->terms[i].count, info->terms[i].k,
							output);
					break;
				case TERM_LATIN:
					succ = latin_term_get(info->terms[i].count, info->terms[i].k,
							output);
					break;
				case TERM_ASCII:
					succ = ascii_term_get(info->terms[i].count, info->terms[i].k,
							output);
					break;
				case TERM_LEXICON:
					succ = lexicon_term_get((const char**) info->words,
							info->num_of_words, info->terms[i].count,
							info->terms[i].k, output);
					break;
			}
			if (succ != 0) {
				fprintf(stderr, "get next term failed\n");
				return RULE_STATUS_ERROR;
			}
			output += strlen(output);
		}
		info->terms[0].k += 1;
	}

	return RULE_STATUS_OK;
}

/* 
 * implements flag='n' -- chooses pseudo-random numbers as the term indexes,
 * and of course it may not cover the entire password space
 */
static int rule_generate_random(rule_info_t * info, char * output)
{
	int i;
	int succ;
	int k;
	char * password = output;

	password[0] = '\0';
	if (info->remaining <= 0) {
		/* exhausted */
		return RULE_STATUS_EXHAUSTED;
	}

	/* instructions:
	 * Notice, however, that even though each substring in the rule can be an empty sequence, 
	 * the entire rule must not be empty, that is, an empty password which theoretically can be 
	 * invoked by a rule is illegal! 
	 */
	while (password[0] == '\0') {
		/* this loop will break only if the generated password is not empty */

		for (i = 0; i < info->num_of_terms; i++) {
			k = (int) randint(info->terms[i].limit);

			switch (info->terms[i].type) {
				case TERM_DIGIT:
					succ = digit_term_get(info->terms[i].count, k, output);
					break;
				case TERM_LATIN:
					succ = latin_term_get(info->terms[i].count, k, output);
					break;
				case TERM_ASCII:
					succ = ascii_term_get(info->terms[i].count, k, output);
					break;
				case TERM_LEXICON:
					succ = lexicon_term_get((const char**) info->words,
							info->num_of_words, info->terms[i].count, k, output);
					break;
			}
			if (succ != 0) {
				fprintf(stderr, "get next term failed\n");
				return RULE_STATUS_ERROR;
			}
			output += strlen(output);
		}
	}

	info->remaining -= 1;
	return RULE_STATUS_OK;
}

/*
 * API
 *
 * generate the next password for the given rule. if flag=ALL, this will generate
 * an incrementing password, otherwise, a random password. you may call this function
 * for as long as it returns RULE_STATUS_OK. when it returns RULE_STATUS_EXHAUSTED,
 * it means you've exhausted all the passwords this rule may generate.
 * RULE_STATUS_ERROR is returned on error.
 */
int rule_generate_next_password(rule_info_t * info, char * output, int output_length)
{
	if (output_length < rule_max_password_length(info)) {
		fprintf(stderr, "rule_generate_next_password: output buffer too small\n");
		return RULE_STATUS_ERROR;
	}

	if (info->limit < 0) {
		/* flag=ALL */
		return rule_generate_incrementing(info, output);
	} else {
		/* flag='n' */
		return rule_generate_random(info, output);
	}
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
	char flag[MAX_INPUT_BUFFER];
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
	flag[0] = '\0';

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
		} else if (strcasecmp(name, "flag") == 0) {
			strcpy(flag, value);
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
	} else if (flag[0] == '\0') {
		fprintf(stderr, "INI file did not specify flag\n");
		return RULE_STATUS_ERROR;
	} else if (hashname[0] == '\0') {
		fprintf(stderr, "INI file did not specify hash_name\n");
		return RULE_STATUS_ERROR;
	}

	return rule_init(info, pattern, lexfilename, hashname, flag);

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

	if (info->words != NULL) {
		for (i = 0; i < info->num_of_words; i++) {
			free(info->words[i]);
		}
		free(info->words);
		info->words = NULL;
	}

	if (info->terms != NULL) {
		free(info->terms);
		info->terms = NULL;
	}
}


