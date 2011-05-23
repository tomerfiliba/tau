#include "deht.h"

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
} DEHT;


DEHT *create_empty_DEHT(const char *prefix,
		hashKeyIntoTableFunctionPtr hashfun,
        hashKeyforEfficientComparisonFunctionPtr validfun,
        int numEntriesInHashTable, int nPairsPerBlock, int nBytesPerKey,
        const char *HashName)
{
	char key_filename[200];
	char data_filename[200];
	DEHT * deht = NULL;

	strcpy(key_filename, prefix);
	strcat(key_filename, ".key");

	strcpy(data_filename, prefix);
	strcat(data_filename, ".data");

	deht = (DEHT*)malloc(sizeof(DEHT));
	if (deht == NULL) {
		/* could not allocate memory */
		return -1;
	}

	strncpy(deht->sPrefixFileName, prefix, sizeof(deht->sPrefixFileName));
	deht->keyFP = fopen(key_filename, "rw");



	return deht;

error_cleanup:
	return NULL;
}























