#include <string.h>
#include <stdlib.h>
#include "deht.h"



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
		return NULL;
	}

	strncpy(deht->sPrefixFileName, prefix, sizeof(deht->sPrefixFileName));
	deht->keyFP = fopen(key_filename, "rw");



	return deht;

error_cleanup:
	return NULL;
}























