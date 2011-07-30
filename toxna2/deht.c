#include <string.h>
#include <stdlib.h>
#include "deht.h"
#include "misc.h"

#define NULL_DISK_PTR  0


static DEHT * _new_DEHT(const char *prefix, const char * data_filename,
		const char * key_filename, hashKeyIntoTableFunctionPtr hashfun,
        hashKeyforEfficientComparisonFunctionPtr validfun)
{
	DEHT * deht = NULL;

	/* allocate instance */
	deht = (DEHT*)malloc(sizeof(DEHT));
	if (deht == NULL) {
		fprintf(stderr, "failed allocating DEHT");
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
		goto cleanup1;
	}

	deht->dataFP = fopen(data_filename, "w+");
	if (deht->dataFP == NULL) {
		perror("fopen of data file");
		goto cleanup2;
	}

	/* success */
	return deht;

cleanup2:
	fclose(deht->keyFP);
cleanup1:
	free(deht);
	return NULL;
}

static int _init_deht_caches(DEHT * deht)
{
	int i;

	deht->hashPointersForLastBlockImageInMemory = (DEHT_DISK_PTR*)calloc(
		sizeof(DEHT_DISK_PTR), deht->header.numEntriesInHashTable);
	if (deht->hashPointersForLastBlockImageInMemory == NULL) {
		fprintf(stderr, "failed allocating hashPointersForLastBlockImageInMemory");
		goto cleanup1;
	}
	deht->anLastBlockSize = (int*)malloc(sizeof(int) * deht->header.numEntriesInHashTable);
	if (deht->anLastBlockSize == NULL) {
		fprintf(stderr, "failed allocating anLastBlockSize");
		goto cleanup2;
	}
	for (i = 0; i < deht->header.numEntriesInHashTable; i++) {
		deht->anLastBlockSize[i] = -1;
	}

	deht->validationKey = (unsigned char*)malloc(deht->header.nBytesPerValidationKey);
	if (deht->validationKey == NULL) {
		fprintf(stderr, "failed allocating validationKey");
		goto cleanup3;
	}

	/* success */
	return 0;

cleanup3:
	free(deht->anLastBlockSize);
cleanup2:
	free(deht->hashPointersForLastBlockImageInMemory);
cleanup1:
	fclose(deht->dataFP);
	return -1;
}

static void _init_deht_files(const char * prefix, char * key_filename, char * data_filename)
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
	DEHT_DISK_PTR * empty_head_table = NULL;
	size_t entries_written = 0;

	_init_deht_files(prefix, key_filename, data_filename);

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

	deht = _new_DEHT(prefix, data_filename, key_filename, hashfun, validfun);
	if (deht == NULL) {
		return NULL;
	}

	/* init header */
	deht->header.numEntriesInHashTable = numEntriesInHashTable;
	deht->header.nPairsPerBlock = nPairsPerBlock;
	deht->header.nBytesPerValidationKey = nBytesPerKey;
	strncpy(deht->header.sHashName, HashName, sizeof(deht->header.sHashName));
	if (strcmp(HashName, "MD5") == 0) {
		deht->header.keySize = MD5_OUTPUT_LENGTH_IN_BYTES;
	}
	else if (strcmp(HashName, "SHA1") == 0) {
		deht->header.keySize = SHA1_OUTPUT_LENGTH_IN_BYTES;
	}
	else {
		fprintf(stderr, "invalid hash name: %s", HashName);
		goto cleanup;
	}

	if (_init_deht_caches(deht) != 0) {
		goto cleanup;
	}

	/* write header and empty head table */
	if (fwrite(&(deht->header), sizeof(deht->header), 1, deht->keyFP) != 1) {
		perror("could not write header to keys file");
		goto cleanup;
	}
	empty_head_table = (DEHT_DISK_PTR*)calloc(numEntriesInHashTable, sizeof(DEHT_DISK_PTR));
	if (empty_head_table == NULL) {
		fprintf(stderr, "could not allocate empty head table with %d entries", 
			numEntriesInHashTable);
		goto cleanup;
	}
	entries_written = fwrite(empty_head_table, sizeof(DEHT_DISK_PTR), 
		numEntriesInHashTable, deht->keyFP);
	if (entries_written != numEntriesInHashTable) {
		free(empty_head_table);
		perror("could not write empty head table to keys file");
		goto cleanup;
	}
	free(empty_head_table);

	/* Initialize data file */
	if (fputc('\0', deht->dataFP) != '\0') {
		perror("could not initialize data file");
		goto cleanup;
	}

	/* Flush the files */
	fflush(deht->keyFP);
	fflush(deht->dataFP);

	return deht;

cleanup:
	if (deht != NULL) {
		if (deht->keyFP != NULL) {
			fclose(deht->keyFP);
		}
		if (deht->dataFP != NULL) {
			fclose(deht->dataFP);
		}
		if (deht->validationKey != NULL) {
			free(deht->validationKey);
		}
		free(deht);
	}
	return NULL;
}

DEHT *load_DEHT_from_files(const char *prefix,
        hashKeyIntoTableFunctionPtr hashfun,
        hashKeyforEfficientComparisonFunctionPtr validfun)
{
	char key_filename[200];
	char data_filename[200];
	FILE * f = NULL;
	DEHT * deht = NULL;

	_init_deht_files(prefix, key_filename, data_filename);

	/* make sure the two files do exist */
	f = fopen(key_filename, "r");
	if (f == NULL) {
		perror("key file does not exist\n");
		return NULL;
	}
	fclose(f);
	f = fopen(data_filename, "r");
	if (f == NULL) {
		perror("data file does not exist\n");
		return NULL;
	}
	fclose(f);

	deht = _new_DEHT(prefix, data_filename, key_filename, hashfun, validfun);
	if (deht == NULL) {
		return NULL;
	}

	/* read header from file */
	if (fread(&deht->header, sizeof(deht->header), 1, deht->keyFP) != 1) {
		fprintf(stderr, "failed to read header from key file\n");
		goto cleanup;
	}

	if (_init_deht_caches(deht) != 0) {
		goto cleanup;
	}

	return deht;

cleanup:
	close_DEHT_files(deht);
	return NULL;
}


void close_DEHT_files(DEHT *ht)
{
	if (ht == NULL) {
		return;
	}
	write_DEHT_pointers_table(ht);

	if (ht->hashTableOfPointersImageInMemory != NULL) {
		free(ht->hashTableOfPointersImageInMemory);
		ht->hashTableOfPointersImageInMemory = NULL;
	}
	if (ht->hashPointersForLastBlockImageInMemory != NULL) {
		free(ht->hashPointersForLastBlockImageInMemory);
		ht->hashPointersForLastBlockImageInMemory = NULL;
	}
	if (ht->anLastBlockSize != NULL) {
		free(ht->anLastBlockSize);
		ht->anLastBlockSize = NULL;
	}

	/* no point in checking return value of fclose -- this function is void
	 * anyway and is not meant to report errors back */
	fclose(ht->keyFP);
	fclose(ht->dataFP);

	/* since we malloc()'ed ht, it's time for us to free() it */
	free(ht);
}

static int find_empty_pair(DEHT * deht, int bucket, DEHT_DISK_PTR * block, DEHT_DISK_PTR * pair)
{
	return -1;
}

static int insert_pair(DEHT * deht, int bucket, DEHT_DISK_PTR pair, 
					   unsigned char * validkey, const unsigned char * data, int dataLength)
{
	return DEHT_STATUS_FAIL;
}

static int add_block(DEHT * deht, int bucket, DEHT_DISK_PTR block, DEHT_DISK_PTR * pair)
{
	return DEHT_STATUS_FAIL;
}

int add_DEHT(DEHT *deht, const unsigned char *key, int keyLength,
        const unsigned char *data, int dataLength)
{
	int bucket;
	DEHT_DISK_PTR pair = NULL_DISK_PTR;
	DEHT_DISK_PTR block = NULL_DISK_PTR;

	bucket = deht->hashFunc(key, keyLength, deht->header.numEntriesInHashTable);
	deht->comparisonHashFunc(key, keyLength, deht->validationKey);

	/* Find the last block in the bucket */
	if (find_empty_pair(deht, bucket, &block, &pair) != 0) {
		perror(deht->sPrefixFileName);
		goto cleanup;
	}

	/* Find the correct place to insert the pair */
	if (pair == NULL_DISK_PTR) {
		/* Need to create a new block */
		if (add_block(deht, bucket, block, &pair)) {
			perror(deht->sPrefixFileName);
			goto cleanup;
		}
	}
	return insert_pair(deht, bucket, pair, deht->validationKey, data, dataLength);

cleanup:
	return DEHT_STATUS_FAIL;
}

int query_DEHT(DEHT *ht, const unsigned char *key, int keyLength,
        unsigned char *data, int dataMaxAllowedLength)
{


	return DEHT_STATUS_FAIL;
}

int insert_uniquely_DEHT(DEHT *ht, const unsigned char *key, int keyLength,
        const unsigned char *data, int dataLength)
{
	/* the size is not important, we just need a few bytes -- we don't use 
	   the data itself */
	unsigned char tmp[30];
	int succ = query_DEHT(ht, key, keyLength, tmp, sizeof(tmp));

	if (succ == DEHT_STATUS_NOT_NEEDED) {
		/* already exists */
		return DEHT_STATUS_NOT_NEEDED;
	}
	if (succ != DEHT_STATUS_SUCCESS) {
		/* query failed */
		return DEHT_STATUS_FAIL;
	}

	/* if key was not found, add it */
	return add_DEHT(ht, key, keyLength, data, dataLength);
}

int read_DEHT_pointers_table(DEHT *ht)
{
	size_t readcount = 0;

	if (ht->hashTableOfPointersImageInMemory != NULL) {
		return DEHT_STATUS_NOT_NEEDED;
	}

	ht->hashTableOfPointersImageInMemory = (DEHT_DISK_PTR*) malloc(
		sizeof(DEHT_DISK_PTR) * ht->header.numEntriesInHashTable);
	if (ht->hashTableOfPointersImageInMemory == NULL) {
		fprintf(stderr, "allocating memory for DEHT pointers table failed\n");
		return DEHT_STATUS_FAIL;
	}
	if (fseek(ht->keyFP, sizeof(ht->header), SEEK_SET) != 0) {
		perror("fseek failed");
		goto cleanup;
	}

	readcount = fread(ht->hashTableOfPointersImageInMemory, sizeof(DEHT_DISK_PTR), 
		ht->header.numEntriesInHashTable, ht->keyFP);
	if (readcount != ht->header.numEntriesInHashTable) {
		perror("failed to read DEHT pointers table");
		goto cleanup;
	}

	return DEHT_STATUS_SUCCESS;

cleanup:
	free(ht->hashTableOfPointersImageInMemory);
	ht->hashTableOfPointersImageInMemory = NULL;
	return DEHT_STATUS_FAIL;
}

int write_DEHT_pointers_table(DEHT *ht)
{
	size_t written = 0;

	if (ht->hashTableOfPointersImageInMemory == NULL) {
		return DEHT_STATUS_NOT_NEEDED;
	}

	if (fseek(ht->keyFP, sizeof(ht->header), SEEK_SET) != 0) {
		perror("fseek failed");
		return DEHT_STATUS_FAIL;
	}

	written = fwrite(ht->hashTableOfPointersImageInMemory, sizeof(DEHT_DISK_PTR), 
		ht->header.numEntriesInHashTable, ht->keyFP);
	if (written != ht->header.numEntriesInHashTable) {
		perror("failed to write everything");
		return DEHT_STATUS_FAIL;
	}
	fflush(ht->keyFP);

	free(ht->hashTableOfPointersImageInMemory);
	ht->hashTableOfPointersImageInMemory = NULL;
	return DEHT_STATUS_SUCCESS;
}
















