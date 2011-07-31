/********************************************************************/
/* File DEHT - Disk Embedded Hash Table. An API you must implement  */
/* It supports varied sizes binary key to binary data - very generic*/
/* Has no necessary link to rainbow tables, passwords, etc.         */
/* Read theory of DEHT before this interface                        */
/********************************************************************/
#ifndef _DEHT_H_
#define _DEHT_H_

#include <stdio.h>

/********************************************************************/
/* type DEHT_DISK_PTR stands for "pointers" representation in DEHT  */
/* Data-type of long (argument of "fseek" function) represents an   */
/* offset in a file, which is "disk pointers" in our implementation */
/********************************************************************/
#define DEHT_DISK_PTR    long

/******************************************************************/
/* structure of "first level header" - basic preferences of a DEHT*/
/******************************************************************/
struct DEHTpreferences
{
	char sHashName[10];				/*Name for identification, e.g. "MD5\0" */
	int numEntriesInHashTable; 		/*typically few millions*/
	int nPairsPerBlock;				/*typically few hundreds*/
	int nBytesPerValidationKey;		/*length of key to be compared into,
									 e.g. 8 means 64bit key for validation*/
	/*********************************************************/
	/*It is completely OK to add several members of your own */
	/*Just remember that this struct is saved "as is" to disk*/
	/*So no pointers should be written here                  */
	/*********************************************************/
};

/******************************************************************/
/* Kind of data-structure DEHT_STATUS that can be -1,0,1 as flags */
/******************************************************************/
#define DEHT_STATUS_SUCCESS        1
#define DEHT_STATUS_FAIL          -1
#define DEHT_STATUS_NOT_NEEDED     0

/****************************************************************************/
/* type definition of hashKeyIntoTableFunctionPtr:                          */
/* Definition of what is a data-structre hash-function (not the cryptic one)*/
/* These function take a key and output an index in pointer table           */
/* Note that these function operates on the original key (not condensed one)*/
/* These function shall never fail (i.e. never return -1 or so)             */
/*                                                                          */
/*Arguments are: */
/* const unsigned char *keyBuf, i.e. Binary buffer input*/
/* int keySizeof , i.e. in this project this is crypt output size, */
/*          but in real life this size may vary (e.g. string input)*/
/* int nTableSize, i.e. Output is 0 to (nTableSize-1) to fit table of pointers*/
/*                                                                          */
/****************************************************************************/
typedef int (*hashKeyIntoTableFunctionPtr)(const unsigned char *, int, int);

/****************************************************************************/
/* type definition of hashKeyforEfficientComparisonFunctionPtr:             */
/* I is made to create a key signature (stored in DEHT) that distinguish    */
/* it from any other key in same bucket. Namely to correct false matches    */
/* caused by the hashKeyIntoTableFunctionPtr, thus must be independent of it*/
/* Note that these functions consider nBytesPerValidationKey as hard coded  */
/* E.g. stringTo32bit(very widely used) or cryptHashTo64bit(as in this proj)*/
/*                                                                          */
/*Arguments are: */
/* const unsigned char *keyBuf, i.e. Binary buffer input*/
/* int keySizeof , i.e. in this project this is crypt output size, */
/*          but in real life this size may vary (e.g. string input)*/
/* unsigned char *validationKeyBuf, i.e. Output buffer, assuming allocated with nBytesPerValidationKey bytes*/
/*                                                                          */
/****************************************************************************/
typedef int (*hashKeyforEfficientComparisonFunctionPtr)(const unsigned char *,
        int, unsigned char *);

/****************************************************************************/
/* type definition of DEHT ! a struct containing all required to specify one*/
/****************************************************************************/
/*This struct holds all needed during actual calls*/
typedef struct
{
	/*prefix for filename (should create: prefix.key, prefix.data) */
	char sPrefixFileName[80];
	/*file pointer to the .key file as stdio recognize*/
	FILE *keyFP;
	FILE *dataFP;
	struct DEHTpreferences header;
	/*key to table of pointers*/
	hashKeyIntoTableFunctionPtr hashFunc;
	/*key to validation process (distinguish collision for real match*/
	hashKeyforEfficientComparisonFunctionPtr comparisonHashFunc;
	/*null or some copy of what in file in case we cache it - efficient to cache this and header only*/
	DEHT_DISK_PTR *hashTableOfPointersImageInMemory;
	/*null or some intermidiate to know whenever insert. It has no parallel on disk*/
	DEHT_DISK_PTR *hashPointersForLastBlockImageInMemory;
	/*null or some intermidiate to know whenever insert. It has no parallel on disk. Block size to enable quick insert*/
	int *anLastBlockSize;
	/* size of pair in block */
	int pairSize;
	/* size of block in DEHT */
	int blockSize;
	/* temp space for validation key */
	unsigned char * tmpValidationKey;
	/* temp space for block pairs */
	unsigned char * tmpBlockPairs;
} DEHT;

/********************************************************************************/
/* Function create_empty_DEHT creates a new DEHT.                               */
/* Inputs: file names on disk (as prefix), hashing functions,                   */
/*    identification name, and parameters regarding memory management           */
/* Output:                                                                      */
/* If fail, Returns NULL and prints informative error to stderr)                */
/* It dump header by itself. Also null table of pointers.                       */
/* Notes:                                                                       */
/* Open them in RW permission (if exist then fail, do not overwrite).           */
/* hashTableOfPointersImageInMemory, hashPointersForLastBlockImageInMemory:=NULL*/
/*add .key and .data to open two files return NULL if fail creation*/
/********************************************************************************/
DEHT *create_empty_DEHT(const char *prefix,
		hashKeyIntoTableFunctionPtr hashfun,
        hashKeyforEfficientComparisonFunctionPtr validfun,
        int numEntriesInHashTable, int nPairsPerBlock, int nBytesPerKey,
        const char *HashName);

/********************************************************************************/
/* Function load_DEHT_from_files imports files created by previously used DEHT */
/* Inputs: file names on disk (as prefix).                                      */
/* Output: an allocated DEHT struct pointer.                                    */
/* Notes:                                                                       */
/* It open files (RW permissions) and create appropriate data-structure on memory */
/* hashTableOfPointersImageInMemory, hashPointersForLastBlockImageInMemory:=NULL*/
/* Returns NULL if fail (e.g. files are not exist) with message to stderr       */
/********************************************************************************/
DEHT *load_DEHT_from_files(const char *prefix,
        hashKeyIntoTableFunctionPtr hashfun,
        hashKeyforEfficientComparisonFunctionPtr validfun);

/********************************************************************************/
/* Function insert_uniquely_DEHT inserts an element.                           */
/* Inputs: DEHT to insert into, key and data (as binary buffer with size)       */
/* Output: just status of action:                                               */
/* If exist returns DEHT_STATUS_NOT_NEEDED(does not insert new key and data)    */
/* If successfully insert returns DEHT_STATUS_SUCCESS.                          */
/* If fail, returns DEHT_STATUS_FAIL                                            */
/* Notes:                                                                       */
/* if hashTableOfPointersImageInMemory use it                                   */
/* if  null, do not load table of pointers into memory just make simple         */
/* insert using several fseek when necessary.                                   */
/********************************************************************************/
int insert_uniquely_DEHT(DEHT *ht, const unsigned char *key, int keyLength,
        const unsigned char *data, int dataLength);

/********************************************************************************/
/* Function add_DEHT inserts an element, whether it exists or not               */
/* Inputs: DEHT to insert into, key and data (as binary buffer with size)       */
/* Output: just status of action:                                               */
/* If successfully insert returns DEHT_STATUS_SUCCESS.                          */
/* If fail, returns DEHT_STATUS_FAIL                                            */
/* Notes:                                                                       */
/* if hashPointersForLastBlockImageInMemory!=NULL use it (save "fseek" commands)*/
/* if anLastBlockSize not null use it either.                                   */
/* if hashTableOfPointersImageInMemory use it (less efficient but stil helps)   */
/* if both null, do not load table of pointers into memory just make simple     */
/* insert using several fseek when necessary.                                   */
/********************************************************************************/
int add_DEHT(DEHT *ht, const unsigned char *key, int keyLength,
        const unsigned char *data, int dataLength);

/********************************************************************************/
/* Function query_DEHT query a key.                                             */
/* Inputs: DEHT to query in, key input and data output buffer.                  */
/* Output:                                                                      */
/* If successfully insert returns number of bytes fullfiled in data buffer      */
/* If not found returns DEHT_STATUS_NOT_NEEDED                                  */
/* If fail returns DEHT_STATUS_FAIL                                             */
/* Notes:                                                                       */
/* If hashTableOfPointersImageInMemory!=NULL use it to save single seek.        */
/* Else access using table of pointers on disk.                                 */
/* "ht" argument is non const as fseek is non const too (will change "keyFP")   */
/********************************************************************************/
int query_DEHT(DEHT *ht, const unsigned char *key, int keyLength,
        unsigned char *data, int dataMaxAllowedLength);

/********************************************************************************/
/* Function read_DEHT_pointers_table loads pointer of tables from disk into RAM     */
/* It will be used for effciency, e.g. when many queries expected soon              */
/* Input: DEHT to act on. (will change member hashTableOfPointersImageInMemory).    */
/* Output:                                                                          */
/* If it is already cached, do nothing and return DEHT_STATUS_NOT_NEEDED.           */
/* If fail, return DEHT_STATUS_FAIL, if success return DEHT_STATUS_NOT_SUCCESS      */
/************************************************************************************/
int read_DEHT_pointers_table(DEHT *ht);

/************************************************************************************/
/* Function write_DEHT_pointers_table writes pointer of tables RAM to Disk & release*/
/* Input: DEHT to act on.                                                           */
/* Output:                                                                          */
/* If not RAM pointer is NULL, return DEHT_STATUS_NOT_NEEDED                        */
/* if fail return DEHT_STATUS_FAIL, if success return DEHT_STATUS_SUCCESS           */
/* Note: do not forget to use "free" and put NULL.                                  */
/************************************************************************************/
int write_DEHT_pointers_table(DEHT *ht);

/************************************************************************************/
/* Function close_DEHT_files closes the DEHT files and release memory.               */
/* Input: DEHT to act on. No Output (never fail).                                   */
/* Notes:                                                                           */
/* calls write_DEHT_hash_table if necessary, call "free" when possible.             */
/* use "fclose" command. do not free "FILE *"                                       */
/************************************************************************************/
void close_DEHT_files(DEHT *ht);


#endif

