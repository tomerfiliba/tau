#include <string.h>
#include <stdlib.h>
#include "deht.h"


static DEHT * new_DEHT(const char *prefix, const char * data_filename,
		const char * key_filename,
		hashKeyIntoTableFunctionPtr hashfun,
        hashKeyforEfficientComparisonFunctionPtr validfun)
{
	DEHT * deht = NULL;

	/* allocate instance */
	deht = (DEHT*)malloc(sizeof(DEHT));
	if (deht == NULL) {
		perror("failed allocating DEHT");
		return NULL;
	}

	/* init members */
	deht->dataFP = NULL;
	deht->keyFP = NULL;

	strncpy(deht->sPrefixFileName, prefix, sizeof(deht->sPrefixFileName));
	deht->hashFunc = hashfun;
	deht->comparisonHashFunc = validfun;
	deht->hashTableOfPointersImageInMemory = NULL;
	deht->hashPointersForLastBlockImageInMemory = NULL;
	deht->anLastBlockSize = NULL;

	/* open the files */
	deht->keyFP = fopen(key_filename, "w+");
	if (deht->keyFP == NULL) {
		perror("fopen of key file");
		goto error_cleanup1;
	}

	deht->dataFP = fopen(data_filename, "w+");
	if (deht->dataFP == NULL) {
		perror("fopen of data file");
		goto error_cleanup2;
	}

	/* success */
	return deht;

error_cleanup2:
	fclose(deht->keyFP);
error_cleanup1:
	free(deht);
	return NULL;
}

static void init_deht_files(const char * prefix, char * key_filename, char * data_filename)
{
	strcpy(key_filename, prefix);
	strcat(key_filename, ".key");

	strcpy(data_filename, prefix);
	strcat(data_filename, ".data");
}

DEHT *create_empty_DEHT(const char *prefix,
		hashKeyIntoTableFunctionPtr hashfun,
        hashKeyforEfficientComparisonFunctionPtr validfun,
        int numEntriesInHashTable, int nPairsPerBlock, int nBytesPerKey,
        const char *HashName)
{
	char key_filename[200];
	char data_filename[200];
	FILE * f = NULL;
	DEHT * deht = NULL;

	init_deht_files(prefix, key_filename, data_filename);

	/* make sure the two files do not already exist */
	if ((f = fopen(key_filename, "r")) != NULL) {
		fclose(f);
		fprintf(stderr, "key file already exists\n");
		return NULL;
	}
	if ((f = fopen(data_filename, "r")) != NULL) {
		fclose(f);
		fprintf(stderr, "data file already exists\n");
		return NULL;
	}

	deht = new_DEHT(prefix, data_filename, key_filename, hashfun, validfun);
	if (deht == NULL) {
		return NULL;
	}

	/* init header */
	deht->header.numEntriesInHashTable = numEntriesInHashTable;
	deht->header.nPairsPerBlock = nPairsPerBlock;
	deht->header.nBytesPerValidationKey = nBytesPerKey;
	strncpy(deht->header.sHashName, HashName, sizeof(deht->header.sHashName));

	return deht;
}

DEHT *load_DEHT_from_files(const char *prefix,
        hashKeyIntoTableFunctionPtr hashfun,
        hashKeyforEfficientComparisonFunctionPtr validfun)
{
	char key_filename[200];
	char data_filename[200];
	FILE * f = NULL;
	DEHT * deht = NULL;

	init_deht_files(prefix, key_filename, data_filename);

	/* make sure the two files do exist */
	if ((f = fopen(key_filename, "r")) == NULL) {
		perror("key file does not exist\n");
		return NULL;
	}
	if ((f = fopen(data_filename, "r")) == NULL) {
		perror("data file does not exist\n");
		return NULL;
	}

	deht = new_DEHT(prefix, data_filename, key_filename, hashfun, validfun);
	if (deht == NULL) {
		return NULL;
	}

	/* read header from file */
	if (fread(&deht->header, sizeof(deht->header), 1, deht->keyFP) != 1) {
		fprintf(stderr, "failed to read header from key file\n");
		close_DEHT_files(deht);
		return NULL;
	}

	return deht;
}


void close_DEHT_files(DEHT *ht)
{
	if (ht == NULL) {
		return;
	}
	write_DEHT_pointers_table(ht);

	/* need to free the following members:
	 *
	 * deht->hashTableOfPointersImageInMemory = NULL;
	 * deht->hashPointersForLastBlockImageInMemory = NULL;
	 * anLastBlockSize
	 */

	/* no point in checking return value of fclose -- this function is void
	 * anyway and is not meant to report errors back */
	fclose(ht->keyFP);
	fclose(ht->dataFP);
}

int insert_uniquely_DEHT(DEHT *ht, const unsigned char *key, int keyLength,
        const unsigned char *data, int dataLength)
{
	unsigned char tmp[30];
	int succ = query_DEHT(ht, key, keyLength, tmp, sizeof(tmp));

	if (succ == DEHT_STATUS_NOT_NEEDED) {
		/* already exists */
		return DEHT_STATUS_NOT_NEEDED;
	}
	if (succ <= 0) {
		/* query failed */
		return DEHT_STATUS_FAIL;
	}
	/* if key was not found, add it */
	return add_DEHT(ht, key, keyLength, data, dataLength);
}

int add_DEHT(DEHT *ht, const unsigned char *key, int keyLength,
        const unsigned char *data, int dataLength)
{
	return DEHT_STATUS_FAIL;
}

int query_DEHT(DEHT *ht, const unsigned char *key, int keyLength,
        unsigned char *data, int dataMaxAllowedLength)
{
	return DEHT_STATUS_FAIL;
}

int read_DEHT_pointers_table(DEHT *ht)
{
	if (ht->hashTableOfPointersImageInMemory != NULL) {
		return DEHT_STATUS_NOT_NEEDED;
	}
	return DEHT_STATUS_FAIL;
}

int write_DEHT_pointers_table(DEHT *ht)
{
	if (ht->hashTableOfPointersImageInMemory == NULL) {
		return DEHT_STATUS_NOT_NEEDED;
	}
	free(ht->hashTableOfPointersImageInMemory);
	ht->hashTableOfPointersImageInMemory = NULL;
	return DEHT_STATUS_FAIL;
}
















