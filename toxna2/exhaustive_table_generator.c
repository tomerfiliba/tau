#include <stdio.h>
#include "misc.h"



int main(int argc, const char** argv)
{
	char _tmp[80];
	/*unsigned char digest[SHA1_OUTPUT_LENGTH_IN_BYTES];
	unsigned char hexdigest[SHA1_OUTPUT_LENGTH_IN_BYTES*2 + 1];
	unsigned char text[] = "hello world my name is moshe";

	printf("%d\n", SHA1BasicHash(text, sizeof(text), digest));
	printf("%d\n", binary2hexa(digest, sizeof(digest), hexdigest, sizeof(hexdigest)));
	printf("'%s'\n", hexdigest);*/

	/*unsigned char buf[4];
	unsigned char hexbuf[33];

	printf("%d\n", hexa2binary("11223344", buf, sizeof(buf)));
	printf("%d\n", binary2hexa(buf, sizeof(buf), hexbuf, sizeof(hexbuf)));
	printf("'%s'\n", hexbuf);*/

	gets(_tmp);
	return 0;
}
