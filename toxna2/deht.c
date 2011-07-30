#include <string.h>
#include <stdlib.h>
#include "deht.h"
#include "misc.h"

#define NULL_DISK_PTR  0
#define DEHT_STATUS_FOUND  3
#define DEHT_STATUS_NOT_FOUND  4

/*
 * allocate (malloc) a new DEHT object and initialize all the fields to 
 * default values. also open the key and data files.
 */
static DEHT * alloc_DEHT(const char *prefix, const char * data_filename,
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
	deht->validationKey = NULL;
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

/*
 * initialize the DEHT's caches (malloc the necessary members in the
 * DEHT struct) for future use
 */
static int init_deht_caches(DEHT * deht)
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

/*
 * create the DEHT's key and data file names
 */
static void init_deht_files(const char * prefix, char * key_filename, char * data_filename)
{
	strcpy(key_filename, prefix);
	strcat(key_filename, ".key");

	strcpy(data_filename, prefix);
	strcat(data_filename, ".data");
}

/* 
 * API 
 */
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

	deht = alloc_DEHT(prefix, data_filename, key_filename, hashfun, validfun);
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

	if (init_deht_caches(deht) != 0) {
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

/* 
 * API 
 */
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

	deht = alloc_DEHT(prefix, data_filename, key_filename, hashfun, validfun);
	if (deht == NULL) {
		return NULL;
	}

	/* read header from file */
	if (fread(&deht->header, sizeof(deht->header), 1, deht->keyFP) != 1) {
		fprintf(stderr, "failed to read header from key file\n");
		goto cleanup;
	}

	if (init_deht_caches(deht) != 0) {
		goto cleanup;
	}

	return deht;

cleanup:
	close_DEHT_files(deht);
	return NULL;
}

/* 
 * API 
 */
void close_DEHT_files(DEHT * deht)
{
	if (deht == NULL) {
		return;
	}
	write_DEHT_pointers_table(deht);

	if (deht->hashTableOfPointersImageInMemory != NULL) {
		free(deht->hashTableOfPointersImageInMemory);
		deht->hashTableOfPointersImageInMemory = NULL;
	}
	if (deht->hashPointersForLastBlockImageInMemory != NULL) {
		free(deht->hashPointersForLastBlockImageInMemory);
		deht->hashPointersForLastBlockImageInMemory = NULL;
	}
	if (deht->anLastBlockSize != NULL) {
		free(deht->anLastBlockSize);
		deht->anLastBlockSize = NULL;
	}

	/* no point in checking return value of fclose -- this function is void
	 * anyway and is not meant to report errors back */
	fclose(deht->keyFP);
	fclose(deht->dataFP);

	/* since we malloc()'ed ht, it's time for us to free() it */
	free(deht);
}

/*****************************************************************************/

/*
 *
 */
static int bucket_find_empty_slot(DEHT * deht, int bucket, DEHT_DISK_PTR * block, 
								  DEHT_DISK_PTR * pair)
{
	return -1;
}

/*
 *
 */
static int bucket_insert_pair(DEHT * deht, int bucket, DEHT_DISK_PTR pair, 
							  unsigned char * validation, const unsigned char * data, int dataLength)
{
	return DEHT_STATUS_FAIL;
}

/*
 *
 */
static int bucket_add_block(DEHT * deht, int bucket, DEHT_DISK_PTR block, 
							DEHT_DISK_PTR * pair)
{
	return DEHT_STATUS_FAIL;
}

/* 
 * API 
 */
int add_DEHT(DEHT *deht, const unsigned char *key, int keyLength,
			 const unsigned char *data, int dataLength)
{
	int bucket;
	DEHT_DISK_PTR pair = NULL_DISK_PTR;
	DEHT_DISK_PTR block = NULL_DISK_PTR;

	bucket = deht->hashFunc(key, keyLength, deht->header.numEntriesInHashTable);
	deht->comparisonHashFunc(key, keyLength, deht->validationKey);

	/* Find the last block in the bucket */
	if (bucket_find_empty_slot(deht, bucket, &block, &pair) != DEHT_STATUS_SUCCESS) {
		return DEHT_STATUS_FAIL;
	}

	/* Find the correct place to insert the pair */
	if (pair == NULL_DISK_PTR) {
		if (bucket_add_block(deht, bucket, block, &pair) != DEHT_STATUS_SUCCESS) {
			return DEHT_STATUS_FAIL;
		}
	}

	return bucket_insert_pair(deht, bucket, pair, deht->validationKey, 
		data, dataLength);
}

/*****************************************************************************/

/*
 *
 */
static int bucket_find_key(DEHT * deht, int bucket, unsigned char * validation, 
					   DEHT_DISK_PTR * pair, DEHT_DISK_PTR * block)
{
	return DEHT_STATUS_FAIL;
}

/*
 *
 */
static int read_pair_data(DEHT * deht, DEHT_DISK_PTR pair, const unsigned char * data, 
						  int dataMaxAllowedLength)
{
	return DEHT_STATUS_FAIL;
}

/*
 *
 */
static int find_pair_by_key(DEHT * deht, const unsigned char *key, int keyLength,
					DEHT_DISK_PTR * pair, DEHT_DISK_PTR * block)
{
	int bucket;
	bucket = deht->hashFunc(key, keyLength, deht->header.numEntriesInHashTable);
	deht->comparisonHashFunc(key, keyLength, deht->validationKey);
	return bucket_find_key(deht, bucket, deht->validationKey, pair, block);
}

/* 
 * API 
 */
int query_DEHT(DEHT *deht, const unsigned char *key, int keyLength,
        unsigned char *data, int dataMaxAllowedLength)
{
	int res;
	DEHT_DISK_PTR pair = NULL_DISK_PTR;
	DEHT_DISK_PTR block = NULL_DISK_PTR;

	res = find_pair_by_key(deht, key, keyLength, &pair, &block);
	if (res == DEHT_STATUS_SUCCESS) {
		return read_pair_data(deht, pair, data, dataMaxAllowedLength);
	}

	/* some kind of error */
	return res;
}

/*****************************************************************************/

/* 
 * API 
 */
int insert_uniquely_DEHT(DEHT *deht, const unsigned char *key, int keyLength,
        const unsigned char *data, int dataLength)
{
	int res;
	DEHT_DISK_PTR pair = NULL_DISK_PTR;
	DEHT_DISK_PTR block = NULL_DISK_PTR;

	res = find_pair_by_key(deht, key, keyLength, &pair, &block);
	if (res == DEHT_STATUS_SUCCESS) {
		/* already exists */
		return DEHT_STATUS_NOT_NEEDED;
	}
	else if (res == DEHT_STATUS_NOT_NEEDED) {
		/* if key was not found, add it */
		return add_DEHT(deht, key, keyLength, data, dataLength);
	}

	/* some kind of error */
	return res;
}

/*****************************************************************************/

/* 
 * API 
 */
int read_DEHT_pointers_table(DEHT *deht)
{
	size_t readcount = 0;

	if (deht->hashTableOfPointersImageInMemory != NULL) {
		return DEHT_STATUS_NOT_NEEDED;
	}

	deht->hashTableOfPointersImageInMemory = (DEHT_DISK_PTR*) malloc(
		sizeof(DEHT_DISK_PTR) * deht->header.numEntriesInHashTable);
	if (deht->hashTableOfPointersImageInMemory == NULL) {
		fprintf(stderr, "allocating memory for DEHT pointers table failed\n");
		return DEHT_STATUS_FAIL;
	}
	if (fseek(deht->keyFP, sizeof(deht->header), SEEK_SET) != 0) {
		perror("fseek failed");
		goto cleanup;
	}

	readcount = fread(deht->hashTableOfPointersImageInMemory, sizeof(DEHT_DISK_PTR), 
		deht->header.numEntriesInHashTable, deht->keyFP);
	if (readcount != deht->header.numEntriesInHashTable) {
		perror("failed to read DEHT pointers table");
		goto cleanup;
	}

	return DEHT_STATUS_SUCCESS;

cleanup:
	free(deht->hashTableOfPointersImageInMemory);
	deht->hashTableOfPointersImageInMemory = NULL;
	return DEHT_STATUS_FAIL;
}

/* 
 * API 
 */
int write_DEHT_pointers_table(DEHT *deht)
{
	size_t written = 0;

	if (deht->hashTableOfPointersImageInMemory == NULL) {
		return DEHT_STATUS_NOT_NEEDED;
	}

	if (fseek(deht->keyFP, sizeof(deht->header), SEEK_SET) != 0) {
		perror("fseek failed");
		return DEHT_STATUS_FAIL;
	}

	written = fwrite(deht->hashTableOfPointersImageInMemory, sizeof(DEHT_DISK_PTR), 
		deht->header.numEntriesInHashTable, deht->keyFP);
	if (written != deht->header.numEntriesInHashTable) {
		perror("failed to write everything");
		return DEHT_STATUS_FAIL;
	}
	fflush(deht->keyFP);

	free(deht->hashTableOfPointersImageInMemory);
	deht->hashTableOfPointersImageInMemory = NULL;
	return DEHT_STATUS_SUCCESS;
}
















