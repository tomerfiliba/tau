#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include "misc.h"
#include "md5.h"
#include "sha1.h"

#define MAX_SEED_USAGE 24

/* You are allowed to change it or keep as is. Up to you*/
LONG_INDEX_PROJ pseudo_random_function(const unsigned char *x, int inputLength,
        LONG_INDEX_PROJ y)
{
	LONG_INDEX_PROJ
	        md5res[MD5_OUTPUT_LENGTH_IN_BYTES / sizeof(LONG_INDEX_PROJ)];
	unsigned char buffer4hashing[MAX_SEED_USAGE + sizeof(LONG_INDEX_PROJ)];

	if (inputLength > MAX_SEED_USAGE)
		inputLength = MAX_SEED_USAGE;
	/*for efficiency purpose*/
	memcpy(buffer4hashing, x, inputLength);/*copy y itself*/
	memcpy(buffer4hashing + inputLength, &y, sizeof(LONG_INDEX_PROJ));
	/*concatenate step to the y*/
	MD5BasicHash(buffer4hashing, inputLength + sizeof(LONG_INDEX_PROJ),
	        (unsigned char *) md5res);
	/*main step, hash both y and index as fusion process*/
	/*now just harvest 63 bit out of 128*/
	/*return ((md5res[0])&0x7fffffffffffffffull); */
	return md5res[0] >> 1;
}

int cryptHash(BasicHashFunctionPtr cryptHashPtr, const unsigned char *passwd,
        unsigned char *outBuf)
{
	return cryptHashPtr(passwd, strlen((const char*) passwd), outBuf);
}

int MD5BasicHash(const unsigned char *in, int len, unsigned char *outBuf)
{
	/* when you want to compute MD5, first, declere the next struct */
	MD5_CTX mdContext;
	/* then, init it before the first use */
	MD5Init(&mdContext);

	/* compute your string's hash using the next to calls */
	MD5Update(&mdContext, (unsigned char *) in, len);
	MD5Final(&mdContext);

	memcpy(outBuf, mdContext.digest, MD5_OUTPUT_LENGTH_IN_BYTES);
	return MD5_OUTPUT_LENGTH_IN_BYTES;
}

int SHA1BasicHash(const unsigned char * data, int size, unsigned char *digest)
{
	int res;
	int i, j;
	unsigned tmp;
	SHA1Context ctx;
	SHA1Reset(&ctx);
	SHA1Input(&ctx, data, size);
	res = SHA1Result(&ctx);
	if (!res) {
		return -1;
	}
	/* convert low-endian unsigned ints to bytes */
	for (i = j = 0; i < 5; i++) {
		tmp = ctx.Message_Digest[i];
		digest[j++] = (tmp >> 24) & 0xFF;
		digest[j++] = (tmp >> 16) & 0xFF;
		digest[j++] = (tmp >> 8) & 0xFF;
		digest[j++] = tmp & 0xFF;
	}
	return SHA1_OUTPUT_LENGTH_IN_BYTES;
}

int binary2hexa(const unsigned char *bufIn, int lengthIn, char *outStr,
        int outMaxLen)
{
	int i;

	if (lengthIn * 2 + 1 > outMaxLen) {
		/* -1 if wanted to exceed */
		/* Note: outStr is null terminated even in case of failour. */
		*outStr = '\0';
		return -1;
	}

	for (i = 0; i < lengthIn; i++) {
		sprintf(outStr + (i * 2), "%02x", bufIn[i]);
	}
	*(outStr + (lengthIn * 2)) = '\0';

	return lengthIn * 2;
}

static int hexdigit_to_number(char ch)
{
	if (ch < '0')
		return -1;
	else if (ch <= '9')
		return ch - '0';
	else if (ch < 'A')
		return -1;
	else if (ch <= 'F')
		return ch - 'A' + 10;
	else if (ch < 'a')
		return -1;
	else if (ch <= 'f')
		return ch - 'a' + 10;
	else
		return -1;
}


int hexa2binary(const char *strIn, unsigned char *outBuf, int outMaxLen)
{
	int i, digit;
	int len = strlen(strIn);
	unsigned char prev_digit;
	const int bin_len = (len / 2) + (len % 2);
	int outindex = outMaxLen - bin_len;

	if (bin_len > outMaxLen) {
		/* out buffer too small */
		return -1;
	}
	memset(outBuf, 0, outMaxLen);
	if (len % 2 != 0) {
		/* consume first uneven hexdigit */
		digit = hexdigit_to_number(strIn[0]);
		if (digit < 0) {
			/* invalid hex digit */
			return -1;
		}
		outBuf[outindex] = digit;
		outindex++;
		strIn++;
		len--;
	}
	
	for (i = 0; i < len; i++) {
		digit = hexdigit_to_number(strIn[i]);
		if (digit < 0) {
			/* invalid hex digit */
			return -1;
		}
		if (i % 2 == 1) {
			outBuf[outindex] = (prev_digit << 4) | digit;
			outindex++;
		} else {
			prev_digit = digit;
		}
	}

	return bin_len;
}

long longpow(int base, int exp)
{
	long val = 1;
	for (; exp > 0; exp--) {
		val *= base;
	}
	return val;
}

int randint(int bound)
{
	time_t t = time(NULL);
	LONG_INDEX_PROJ r = pseudo_random_function((unsigned char*) &t, sizeof(t),
	        rand());
	return r % bound;
}

/****************************************************************************/
/* const unsigned char *keyBuf, i.e. Binary buffer input                    */
/* int keySize , i.e. in this project this is crypt output size,            */
/*          but in real life this size may vary (e.g. string input)         */
/* int nTableSize, i.e. Output is 0 to (nTableSize-1) to fit table of       */
/*          pointers                                                        */
/*                                                                          */
/****************************************************************************/
int my_hash_func(const unsigned char * keyBuf, int keySize, int tableSize)
{
	int i;
	unsigned int xorsum = 0x8BADF00D;
	assert(keySize > sizeof(unsigned int));

	/* xor the key, DWORD at a time, to generate a "unique value" */
	for (i = 0; i < (int)(keySize - sizeof(unsigned int)); i ++) {
		xorsum ^= *((unsigned int*)(keyBuf+i));
	}
	return xorsum % tableSize;
}

/****************************************************************************/
/* const unsigned char *keyBuf, i.e. Binary buffer input                    */
/* int keySize , i.e. in this project this is crypt output size,            */
/*          but in real life this size may vary (e.g. string input)         */
/* unsigned char *validationKeyBuf, i.e. Output buffer, assuming allocated  */
/*            with nBytesPerValidationKey bytes                             */
/*                                                                          */
/****************************************************************************/
int my_valid_func(const unsigned char *keyBuf, int keySize, unsigned char * validationKeyBuf)
{
	/* make sure the key is long enough */
	assert(keySize >= 8);
	/* use the 8 bytes lower bytes as the validation data for the key */
	memcpy(validationKeyBuf, keyBuf, 8);
	/* this is hardcoded because nBytesPerValidationKey == 8*/
	return 8;
}

