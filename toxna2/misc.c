#include <stdio.h>
#include "misc.h"
#include "md5.h"
#include "sha1.h"

#define MAX_SEED_USAGE 24

/* You are allowed to change it or keep as is. Up to you*/
LONG_INDEX_PROJ pseudo_random_function(const unsigned char *x,int inputLength,LONG_INDEX_PROJ y)
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

int cryptHash ( BasicHashFunctionPtr cryptHashPtr, const char *passwd, unsigned char *outBuf )
{
	return cryptHashPtr ( passwd, strlen(passwd) , outBuf) ; 
}

int MD5BasicHash ( const unsigned char *in,int len, unsigned char *outBuf)
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


int binary2hexa(const unsigned char *bufIn, int lengthIn,
				char *outStr, int outMaxLen)
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

