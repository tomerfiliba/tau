/*
*** 	Based on md5driver.c						     ***
***	from http://people.csail.mit.edu/rivest/Md5.c 		     ***

**********************************************************************
** md5driver.c -- sample routines to test                           **
** RSA Data Security, Inc. MD5 message digest algorithm.            **
** Created: 2/16/90 RLR                                             **
** Updated: 1/91 SRD                                                **
**********************************************************************

*/

/*
**********************************************************************
** Copyright (C) 1990, RSA Data Security, Inc. All rights reserved. **
**                                                                  **
** RSA Data Security, Inc. makes no representations concerning      **
** either the merchantability of this software or the suitability   **
** of this software for any particular purpose.  It is provided "as **
** is" without express or implied warranty of any kind.             **
**                                                                  **
** These notices must be retained in any copies of any part of this **
** documentation and/or software.                                   **
**********************************************************************
*/

#include <stdio.h>
#include <string.h>
#include "md5.h"


/* Computes the message digest for string inString.
Prints out message digest and the string (in quotes) and a
carriage return.
*/
static void Md5FromString (char *inString)
{
	int i;

	/* when you want to compute MD5, first, declere the next struct */
	MD5_CTX mdContext;
	/* then, init it before the first use */
	MD5Init (&mdContext);

	/* compute your string's hash using the next to calls */
	MD5Update (&mdContext, (unsigned char *)inString, strlen (inString));
	MD5Final (&mdContext);

	/* the result are stored in binary, in mdContext.digest[16]*/
	/* As we print it in little endian we first put 4 lsb then 4 msb*/
	for (i = 0; i < 16; i++){
		printf ("%x", (mdContext.digest[i])&0xf);
		printf ("%x", ((mdContext.digest[i])>>4)&0xf);
	}
	printf (" \"%s\"\n", inString);
}

/*
int main (int argc, char *argv[])
{
	int i;
	/* For each command line argument it prints message digest and contents of string *-/
	for (i = 1; i < argc; i++)
		Md5FromString (argv[i]);
	return 0;
}
*/

