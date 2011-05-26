#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
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
		return -1;
	} else if (k == 0) {
		*output = '\0';
		return 0;
	}
	return digit_term_get_rec(count, k - 1, output);
}

static int digit_term_get_rec(int count, int k, char * output)
{
	int max = longpow(10, count);
	if (k < max) {
		sprintf(output, "%d", k);
		return 0;
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
		return -1;
	} else if (k == 0) {
		*output = '\0';
		return 0;
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
		return 0;
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
		return 0;
	} else if (k < 0 || k > ascii_term_get_limit(count)) {
		return -1;
	}
	return ascii_term_get_rec(count, k - 1, output);
}

static int ascii_term_get_rec(int count, int k, char * output)
{
	int max = longpow(95, count);
	if (k < max) {
		to_base(k, ' ', 95, output);
		return 0;
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
		return -1;
	} else if (k == 0) {
		*output = '\0';
		return 0;
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
		return 0;
	} else {
		strcpy(output, lexicon[0]);
		return lexicon_term_get_rec(lexicon, num_of_words, count - 1, k - max,
		        output + strlen(lexicon[0]));
	}
}

/* ========================================================================= */

static int rule_load_lexicon(rule_info_t * info, const char* filename)
{
	char line[200];
	int i;
	int allocated_indexes = 0;
	char ** newbuf = NULL;
	char * ch;
	FILE * f = fopen(filename, "r");

	info->num_of_words = 0;
	info->words = NULL;

	if (f == NULL) {
		/* fopen failed; */
		perror("rule_load_lexicon: fopen of lexicon file failed");
		return -1;
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
				printf("memory allocation (1) failed\n");
				goto error_cleanup;
			}
			info->words = (char**) newbuf;
		}

		info->words[info->num_of_words] = (char*) malloc(strlen(line) + 1);
		if (info->words[info->num_of_words] == NULL) {
			printf("memory allocation (2) failed\n");
			goto error_cleanup;
		}
		strcpy(info->words[info->num_of_words], line);
		info->num_of_words++;
	}

	if (fclose(f) != 0) {
		/* fclose failed */
		perror("rule_load_lexicon: fclose failed");
		return -1;
	}

	return 0;

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
	return -1;
}

static int rule_load_pattern(rule_info_t * info, const char* pattern)
{
	const int pattern_len = strlen(pattern);
	int i, j;

	if (pattern_len % 2 != 0) {
		/* invalid syntax */
		fprintf(stderr, "invalid rule pattern\n");
		return -1;
	}
	info->num_of_terms = pattern_len / 2;
	info->terms = (term_info_t*) malloc(sizeof(term_info_t)
	        * info->num_of_terms);
	if (info->terms == NULL) {
		fprintf(stderr, "rule_load_pattern: memory allocation (3) failed\n");
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
				break;
			case '.':
				info->terms[j].type = TERM_LATIN;
				info->terms[j].limit = latin_term_get_limit(
				        info->terms[j].count);
				break;
			case '$':
				info->terms[j].type = TERM_ASCII;
				info->terms[j].limit = ascii_term_get_limit(
				        info->terms[j].count);
				break;
			case '@':
				info->terms[j].type = TERM_LEXICON;
				info->terms[j].limit = lexicon_term_get_limit(
				        info->num_of_words, info->terms[j].count);
				break;
			default:
				/* invalid syntax */
				fprintf(stderr, "rule_load_pattern: invalid pattern '%c'\n", pattern[i]);
				goto error_cleanup;
		}
		/*printf("%d count=%d, limit=%d\n", info->terms[j].type, info->terms[j].count, info->terms[j].limit);*/
	}

	return 0;

error_cleanup:
	if (info->terms != NULL) {
		free(info->terms);
	}
	info->num_of_terms = 0;
	info->terms = NULL;
	return -1;
}

int rule_load(rule_info_t * info, const char * pattern,
        const char * lexfilename, const char * hashname, const char * flag)
{
	if (strcasecmp(flag, "all") == 0) {
		info->limit = -1;
	} else {
		if (sscanf(flag, "%d", &info->limit) != 1) {
			/* invalid flag */
			fprintf(stderr, "rule_load: invalid flag '%s'", flag);
			return -1;
		}
	}
	if (strcasecmp(hashname, "md5") == 0) {
		strcpy(info->hashname, "MD5");
		info->hashfunc = MD5BasicHash;
		info->digest_size = MD5_OUTPUT_LENGTH_IN_BYTES;
	} else if (strcasecmp(hashname, "sha1") == 0) {
		strcpy(info->hashname, "SHA1");
		info->hashfunc = SHA1BasicHash;
		info->digest_size = SHA1_OUTPUT_LENGTH_IN_BYTES;
	} else {
		/* invalid hash */
		fprintf(stderr, "rule_load: invalid hash name '%s'", hashname);
		return -1;
	}
	info->remaining = info->limit;
	if (rule_load_lexicon(info, lexfilename) != 0) {
		return -1;
	}
	if (rule_load_pattern(info, pattern) != 0) {
		return -1;
	}
	return 0;
}

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

int rule_max_password_length(rule_info_t * info)
{
	int i;
	int max_length = 0;

	for (i = 0; i < info->num_of_terms; i++) {
		max_length += info->terms[i].count;
	}

	return max_length;
}

static int rule_generate_incrementing(rule_info_t * info, char * output)
{
	int i;
	int succ;

	for (i = 0; i < info->num_of_terms; i++) {
		if (info->terms[i].k > info->terms[i].limit) {
			info->terms[i].k = 0;
			if (i == info->num_of_terms - 1) {
				/* exhausted all terms */
				return -2;
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
			printf("get next term failed\n");
			return -1;
		}
		output += strlen(output);
	}
	info->terms[0].k += 1;
	return 0;
}

static int rule_generate_random(rule_info_t * info, char * output)
{
	int i;
	int succ;
	int k;

	if (info->remaining <= 0) {
		/* exhausted */
		return -2;
	}

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
			printf("get next term failed\n");
			return -1;
		}
		output += strlen(output);
	}
	info->remaining -= 1;
	return 0;
}

int rule_generate_next_password(rule_info_t * info, char * output, int output_length)
{
	if (output_length < rule_max_password_length(info)) {
		fprintf(stderr, "rule_generate_next_password: output buffer too small\n");
		return -1;
	}

	if (info->limit < 0) {
		return rule_generate_incrementing(info, output);
	} else {
		return rule_generate_random(info, output);
	}
}

int rule_load_from_file(rule_info_t * info, const char * inifilename)
{
	FILE *f = fopen(inifilename, "r");
	char lexfilename[200];
	char pattern[200];
	char hashname[200];
	char flag[200];
	char name[100];
	char value[200];
	int res;

	if (f == NULL) {
		perror("rule_load_from_file: fopen failed");
		return -1;
	}

	lexfilename[0] = '\0';
	pattern[0] = '\0';
	hashname[0] = '\0';
	flag[0] = '\0';

	while (1) {
		res = fscanf(f, "%s = %s", name, value);
		if (res == EOF) {
			break;
		} else if (res != 2) {
			printf("Syntax error in INI file\n");
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
			printf("Invalid key '%s' in INI file\n", name);
			goto error_cleaup;
		}
	}
	if (fclose(f) != 0) {
		perror("rule_load_from_file: fclose failed");
		return -1;
	}

	if (lexfilename[0] == '\0') {
		fprintf(stderr, "INI file did not specify lexicon_name\n");
		return -1;
	} else if (pattern[0] == '\0') {
		fprintf(stderr, "INI file did not specify rule\n");
		return -1;
	} else if (flag[0] == '\0') {
		fprintf(stderr, "INI file did not specify flag\n");
		return -1;
	} else if (hashname[0] == '\0') {
		fprintf(stderr, "INI file did not specify hash_name\n");
		return -1;
	}

	return rule_load(info, pattern, lexfilename, hashname, flag);

error_cleaup:
	fclose(f);
	return -1;
}

