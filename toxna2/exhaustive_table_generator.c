#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "misc.h"


typedef enum
{
	OP_LEXWORD,	/* @ */
	OP_DIGIT,	/* # */
	OP_LOWERENG,	/* . */
	OP_ASCII  	/* $ */
} rule_operator_t;

typedef struct 
{
	rule_operator_t			op;
	int				count;
} rule_term_t;

long longpow(int base, int exp)
{
	long val = 1;
	for (; exp > 0; exp--) {
		val *= base;
	}
	return val;
}

/* ------------------------------------------------------------------------- */

/* #5 */
int digit_term_get_rec(int count, int k, char * output);

int digit_term_get(int count, int k, char * output)
{
	int max = (1-longpow(10, count+1)) / (-9); /* sum of geometric series */
	if (k == 0) {
		*output = '\0';
		return 0;
	}
	if (k < 0 || k >= max) {
		return -1;
	}
	return digit_term_get_rec(count, k - 1, output);
}

int digit_term_get_rec(int count, int k, char * output)
{
	int max = longpow(10, count);
	if (k < max) {
		sprintf(output, "%d", k);
		return 0;
	}
	*output = '0';
	return digit_term_get_rec(count - 1, k - max, output + 1);
}

/* ------------------------------------------------------------------------- */

/* . */
int loweng_term_get_rec(int count, int k, char * output);

int loweng_term_get(int count, int k, char * output)
{
	int max = (1-longpow(26, count+1)) / (-25); /* sum of geometric series */
	if (k == 0) {
		*output = '\0';
		return 0;
	}
	if (k < 1 || k >= max) {
		return -1;
	}
	return loweng_term_get_rec(count, k - 1, output);
}

int loweng_term_get_rec(int count, int k, char * output)
{
	int max = longpow(26, count);
	if (k < max) {
		*output = 'a' + (k % 26);
		k /= 26;
		output++;
		while (k > 0) {
			*output = 'a' + (k % 26);
			k /= 26;
			output++;
		}
		*output = '\0';
		return 0;
	}
	else {
		*output = 'a';
		return loweng_term_get_rec(count - 1, k - max, output + 1);
	}
}

/* ------------------------------------------------------------------------- */

/* $ */
int ascii_term_get_rec(int count, int k, char * output);

int ascii_term_get(int count, int k, char * output)
{
	int max = (1-longpow(95, count+1)) / (-94); /* sum of geometric series */
	if (k == 0) {
		*output = '\0';
		return 0;
	}
	if (k < 1 || k >= max) {
		return -1;
	}
	return ascii_term_get_rec(count, k - 1, output);
}

int ascii_term_get_rec(int count, int k, char * output)
{
	int max = longpow(95, count);
	if (k < max) {
		*output = 32 + (k % 95);
		k /= 95;
		output++;
		while (k > 0) {
			*output = 32 + (k % 95);
			k /= 95;
			output++;
		}
		*output = '\0';
		return 0;
	}
	else {
		*output = 32;
		return ascii_term_get_rec(count - 1, k - max, output + 1);
	}
}

/* ------------------------------------------------------------------------- */

/* $ */
int lexicon_term_get_rec(int count, const char ** lexicon, int num_of_words, int k, char * output);

int lexicon_term_get(int count, const char ** lexicon, int num_of_words, int k, char * output)
{
	int max = (1-longpow(num_of_words, count+1)) / (-num_of_words+1); /* sum of geometric series */
	if (k == 0) {
		*output = '\0';
		return 0;
	}
	if (k < 1 || k >= max) {
		return -1;
	}
	return lexicon_term_get_rec(count, lexicon, num_of_words, k - 1, output);
}

int lexicon_term_get_rec(int count, const char ** lexicon, int num_of_words, int k, char * output)
{
	int max = longpow(num_of_words, count);
	const char * word;
	if (k < max) {
		word = lexicon[k%num_of_words];
		strcpy(output, word);
		output += strlen(word);
		k /= num_of_words;
		while (k > 0) {
			word = lexicon[k%num_of_words];
			strcpy(output, word);
			output += strlen(word);
			k /= num_of_words;
		}
		return 0;
	}
	else {
		strcpy(output, lexicon[0]);
		return lexicon_term_get_rec(count - 1, lexicon, num_of_words, k - max, output + strlen(lexicon[0]));
	}
}

/* ------------------------------------------------------------------------- */

int main(int argc, const char** argv)
{
	char buf[20];
	const char* words[] = {
		"hello.",
		"moshe.",
		"ra.",
		"bla.",
	};


	lexicon_term_get(3, words, 4, 1, buf);
	printf("'%s'\n", buf);
	lexicon_term_get(3, words, 4, 2, buf);
	printf("'%s'\n", buf);
	lexicon_term_get(3, words, 4, 3, buf);
	printf("'%s'\n", buf);
	lexicon_term_get(3, words, 4, 4, buf);
	printf("'%s'\n", buf);
	lexicon_term_get(3, words, 4, 5, buf);
	printf("'%s'\n", buf);
	lexicon_term_get(3, words, 4, 6, buf);
	printf("'%s'\n", buf);
	lexicon_term_get(3, words, 4, 4*4*4, buf);
	printf("'%s'\n", buf);
	lexicon_term_get(3, words, 4, 4*4*4+1, buf);
	printf("'%s'\n", buf);


	return 0;
}
