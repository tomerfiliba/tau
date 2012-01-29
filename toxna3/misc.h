/************************************************************************/
/* File misc.h, some nice interfaces along with partial implementations */
/************************************************************************/

#ifndef _MISC_H_
#define _MISC_H_

#ifdef _MSC_VER
typedef signed __int32 int32_t;
typedef signed __int64 int64_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
#else
#include <stdint.h>
#endif
#include <string.h>

/* max size of user inputs, passwords, and filename */
#define MAX_USER_INPUT      256
#define MAX_INPUT_BUFFER    (MAX_USER_INPUT+1)

/*************************************************************************/
/* typedef BasicHashFunctionPtr:                                         */
/* A cryptographic hash function performs many bitic operation on a      */
/* variable size binary array and returns a fixed size one:              */
/* Define its type by BasicHashFunctionPtr                               */
/* It returns number of bytes actually used in output buffer (20 in SHA1)*/
/* As sizes are fixed, relatively short and well known, assume output    */
/* buffer is long enough to contain these all.                           */
/*                                                                       */
/* Arguments are: const unsigned char *inbuf, i.e. binary input*/
/* int inputLength , i.e. its length in byte, as it may vary   */
/* unsigned char *outBuf output buffer to fulfill. Assume long enough*/
/*                                                                       */
/*************************************************************************/
typedef int
        (*BasicHashFunctionPtr)(const unsigned char *, int, unsigned char *);

#define MD5_OUTPUT_LENGTH_IN_BYTES    16
int MD5BasicHash(const unsigned char *in, int len, unsigned char *outBuf);

#define SHA1_OUTPUT_LENGTH_IN_BYTES   20
int SHA1BasicHash(const unsigned char *in, int len, unsigned char *outBuf);

#define MAX_DIGEST_LENGTH_IN_BYTES	  ((SHA1_OUTPUT_LENGTH_IN_BYTES > MD5_OUTPUT_LENGTH_IN_BYTES) ? \
											SHA1_OUTPUT_LENGTH_IN_BYTES : MD5_OUTPUT_LENGTH_IN_BYTES)


/*************************************************************************/
/* Function cryptHash do it on an ascii string.                          */
/* Inputs:                                                               */
/* A function pointer to crytographic hash (e.g. MD5BasicHash)           */
/* A null terminated string of ascii password we want to hash            */
/* An output binary buffer (to fullfil without null termination)         */
/* Output: number bytes fulfilled in output (e.g. 16 for MD5)            */
/* usage like cryptHash ( MD5BasicHash , passwd , outbufWith20Bytes );   */
/*************************************************************************/
int cryptHash(BasicHashFunctionPtr cryptHashPtr, const unsigned char *passwd,
        unsigned char *outBuf);

/*************************************************************************/
/* Functions hexa2binary and binary2hexa are parallel to atoi and itoa   */
/* but use little-endian and longer binary size. So strings are read or  */
/* written in a reverse order than what we used to in Math.              */
/* Advantage of little-endian that it is very easy to implement, easier  */
/*   to read, and more compatible with the way things arranged in mem.   */
/*************************************************************************/

/*************************************************************************/
/* Function hexa2binary transforms each pair of hexa characters into     */
/*   a single output byte.                                               */
/* Inputs:                                                               */
/*  A null terminated ascii string (strIn).                              */
/*  A buffer to write output into (outBuf) not null terminated.          */
/*  Length of buffer (outMaxLen) not to exceed - avoid memory violation  */
/* Output: numer of bytes fulfilled in outBuf, or -1 if                  */
/*   either wanted to exceed memory limit, or string was not hexa        */
/*************************************************************************/
int hexa2binary(const char *strIn, unsigned char *outBuf, int outMaxLen);

/*************************************************************************/
/* Function binary2hexa transforms each input byte into two hexa chars   */
/* Inputs:                                                               */
/*  A binary buffer (bufIn) and its length in bytes (lengthIn).          */
/*  An output string to fulfil(outStroutStr) and memory limit (outMaxLen)*/
/* Output: numer of bytes fulfilled in outStr, or -1 if wanted to exceed */
/* Note: outStr is null terminated even in case of failour.              */
/*************************************************************************/
int binary2hexa(const unsigned char *bufIn, int lengthIn, char *outStr,
        int outMaxLen);

/*************************************************************************/
/* In real rainbow-tables number of possible passwords to examine is     */
/* typically larger than 2^32 thus index of such password shall be 64bit */
/* type definition of LONG_INDEX_PROJ is a 64bit index can password space*/
/*************************************************************************/
#define LONG_INDEX_PROJ uint64_t

int64_t longpow(int base, int exp);

int my_hash_func(const unsigned char * keyBuf, int keySize, int tableSize);

int my_valid_func(const unsigned char *keyBuf, int keySize, unsigned char * validationKeyBuf);



#endif


