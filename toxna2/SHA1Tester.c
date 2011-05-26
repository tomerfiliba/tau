/*
 *** 	based on sha1test.c,							***
 ***	from http://www.packetizer.com/security/sha1/			***
*/

#include <stdio.h>
#include <string.h>
#include "sha1.h"

void Sha1FromString (char *inString)
{
  int sharesult_value = 0, i;

  /* when you want to compute SHA1, first, declere the next struct */
  SHA1Context sha;
  /* then, init it before the first use */
  SHA1Reset(&sha);

  /* compute your string's hash using the next to calls */
  SHA1Input(&sha, (const unsigned char *) inString, strlen(inString));
  sharesult_value = SHA1Result(&sha);

  if (!sharesult_value)
  {
      printf("ERROR-- could not compute message digest for %s\n", inString);
  }
  else
  {
      for(i = 0; i < 5 ; i++)
      {
          printf("%X", sha.Message_Digest[i]); /* again, the result is in sha.Message_Digest[i] which is unsigned Message_Digest[5] */
      }
      printf(" \"%s\"\n", inString);
    }
}

/*
int main (int argc, char *argv[])
{
  int i;
  /-* For each command line argument it prints sha1 hash and contents of string *-/
    for (i = 1; i < argc; i++)
        Sha1FromString (argv[i]);
  return 0;
}
*/
