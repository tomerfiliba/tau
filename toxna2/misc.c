#include <stdio.h>
#include <ctype.h>
#include "misc.h"
#include "md5.h"
#include "sha1.h"

#define MAX_SEED_USAGE 24

/* You are allowed to change it or keep as is. Up to you*/
LONG_INDEX_PROJ pseudo_random_function(const unsigned char *x, int inputLength, LONG_INDEX_PROJ y)
{
	LONG_INDEX_PROJ md5res[MD5_OUTPUT_LENGTH_IN_BYTES/sizeof(LONG_INDEX_PROJ)];
	unsigned char buffer4hashing[MAX_SEED_USAGE+sizeof(LONG_INDEX_PROJ)];

	if(inputLength>MAX_SEED_USAGE) inputLength = MAX_SEED_USAGE; 
	/*for efficiency purpose*/
	memcpy(buffer4hashing,x,inputLength);/*copy y itself*/
	memcpy(buffer4hashing+inputLength,&y,sizeof(LONG_INDEX_PROJ)); 
	/*concatenate step to the y*/ 
	MD5BasicHash( buffer4hashing, inputLength+sizeof(LONG_INDEX_PROJ) , (unsigned char *)md5res ); 
	/*main step, hash both y and index as fusion process*/ 
	/*now just harvest 63 bit out of 128*/
	return ((md5res[0])&0x7fffffffffffffff);     
}

int cryptHash (BasicHashFunctionPtr cryptHashPtr, const char *passwd, unsigned char *outBuf)
{
	return cryptHashPtr ( passwd, strlen(passwd) , outBuf) ; 
}

int MD5BasicHash (const unsigned char *in, int len, unsigned char *outBuf)
{
	/* when you want to compute MD5, first, declere the next struct */
	MD5_CTX mdContext;
	/* then, init it before the first use */
	MD5Init (&mdContext);

	/* compute your string's hash using the next to calls */
	MD5Update (&mdContext, (unsigned char *)in, len);
	MD5Final (&mdContext);

	memcpy(outBuf,mdContext.digest,MD5_OUTPUT_LENGTH_IN_BYTES);
	return MD5_OUTPUT_LENGTH_IN_BYTES;
}


int SHA1BasicHash(const unsigned char * data, int size, unsigned char *digest)
{
	int res;
	SHA1Context ctx;
	SHA1Reset(&ctx);
	SHA1Input(&ctx, data, size);
	res = SHA1Result(&ctx);
	if (!res) {
		return -1;
	}
	memcpy(digest, ctx.Message_Digest, SHA1_OUTPUT_LENGTH_IN_BYTES);
	return SHA1_OUTPUT_LENGTH_IN_BYTES;
}


int binary2hexa(const unsigned char *bufIn, int lengthIn, char *outStr, int outMaxLen)
{
	int i;

	if (lengthIn * 2 + 1 > outMaxLen) {
		/* -1 if wanted to exceed */
		/* Note: outStr is null terminated even in case of failour. */
		*outStr = '\0';
		return -1;
	}

	for (i = 0; i < lengthIn; i++) {
		sprintf(outStr + (i*2), "%02x", bufIn[i]);
	}
	*(outStr + (lengthIn*2)) = '\0';

	return lengthIn*2;
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
		return ch - 'A';
	else if (ch < 'a')
		return -1;
	else if (ch <= 'f')
		return ch - 'a';
	else
		return -1;
}

int hexa2binary(const char *strIn, unsigned char *outBuf, int outMaxLen)
{
	const int len = strlen(strIn);
	int i, digit;
	unsigned char prev_digit;
	unsigned char * out_ch = outBuf;

	if (len % 2 != 0) {
		// invalid hex string: strIn of odd length
		return -1;
	}
	if (len / 2 > outMaxLen) {
		// out buffer too small
		return -1;
	}
	memset(outBuf, 0, outMaxLen);
	
	for (i = 0; i < len; i++) {
		digit = hexdigit_to_number(strIn[i]);
		if (digit < 0) {
			// invalid hex digit
			return -1;
		}
		if (i % 2 == 1) {
			*out_ch = (prev_digit << 4) | digit;
			out_ch++;
		}
		else {
			prev_digit = digit;
		}
	}

	return len / 2;
}

