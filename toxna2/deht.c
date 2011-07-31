#include <string.h>
#include <stdlib.h>
#include "deht.h"
#include "misc.h"

#define NULL_DISK_PTR              0
#define DEHT_STATUS_DEADEND        2

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
	deht->anLastBlockSize = NULL;
	deht->tmpValidationKey = NULL;
	deht->tmpBlockPairs = NULL;
	deht->pairSize = 0;

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

	deht->pairSize = deht->header.nBytesPerValidationKey + sizeof(DEHT_DISK_PTR);
	deht->blockSize = deht->header.nPairsPerBlock * deht->pairSize + sizeof(DEHT_DISK_PTR);

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

	deht->tmpValidationKey = (unsigned char*)malloc(deht->header.nBytesPerValidationKey);
	if (deht->tmpValidationKey == NULL) {
		fprintf(stderr, "failed allocating tmpValidationKey");
		goto cleanup3;
	}

	deht->tmpBlockPairs = (unsigned char*)malloc(deht->blockSize);
	if (deht->tmpBlockPairs == NULL) {
		fprintf(stderr, "failed allocating tmpBlockPairs");
		goto cleanup4;
	}

	/* success */
	return 0;

cleanup4:
	free(deht->tmpValidationKey);
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
	/*if (strcmp(HashName, "MD5") == 0) {
		deht->header.keySize = MD5_OUTPUT_LENGTH_IN_BYTES;
	}
	else if (strcmp(HashName, "SHA1") == 0) {
		deht->header.keySize = SHA1_OUTPUT_LENGTH_IN_BYTES;
	}
	else {
		fprintf(stderr, "invalid hash name: %s", HashName);
		goto cleanup;
	}*/

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
		if (deht->tmpValidationKey != NULL) {
			free(deht->tmpValidationKey);
		}
		if (deht->tmpBlockPairs != NULL) {
			free(deht->tmpBlockPairs);
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

	/* since we malloc()'ed the deht, it's time for us to free() it */
	free(deht);
}

/*****************************************************************************/

static int fread_from(FILE * file, long offset, void * data, size_t size)
{
	if (fseek(file, offset, SEEK_SET) != 0) {
		perror("fseek");
		return -1;
	}
	if (fread(data, size, 1, file) != 1) {
		perror("fread");
		return -1;
	}
	return 0;
}

static int fwrite_at(FILE * file, long offset, const void * data, size_t size)
{
	if (fseek(file, offset, SEEK_SET) != 0) {
		perror("fseek");
		return -1;
	}
	if (fwrite(data, size, 1, file) != 1) {
		perror("fwrite");
		return -1;
	}
	return 0;
}

/*
 * Parameters:
 *    * deht - the deht object
 *    * bucket - the bucket index in the deht
 * Output Parameters:
 *    * the block PTR
 *    * the pair PTR
 * Returns:
 *    DEHT_STATUS_SUCCESS
 */
static int bucket_find_empty_slot(DEHT * deht, int bucket, DEHT_DISK_PTR * block, 
								  DEHT_DISK_PTR * pair)
{
	int pair_count = 0;
	DEHT_DISK_PTR current_block_disk_ptr = NULL_DISK_PTR;
	DEHT_DISK_PTR next_block_disk_ptr = NULL_DISK_PTR;
	DEHT_DISK_PTR * pair_ptr = NULL_DISK_PTR;
	
	/* try to serve request from cache */
	if (deht->anLastBlockSize[bucket] != -1) {
		*block = deht->hashTableOfPointersImageInMemory[bucket];
		*pair = *block + deht->anLastBlockSize[bucket] * deht->pairSize;
		return DEHT_STATUS_SUCCESS;
	}

	/* find the first block */
	if (deht->hashTableOfPointersImageInMemory != NULL) {
		/* use cached head table if possible */
		current_block_disk_ptr = deht->hashTableOfPointersImageInMemory[bucket];
	} else {
		if (fread_from(deht->keyFP, sizeof(deht->header) + bucket * sizeof(DEHT_DISK_PTR),
				&current_block_disk_ptr, sizeof(DEHT_DISK_PTR)) != 0) {
			return DEHT_STATUS_FAIL;
		}
	}

	if (current_block_disk_ptr == NULL_DISK_PTR) {
		/* bucket is empty */
		*block = NULL_DISK_PTR;
		*pair = NULL_DISK_PTR;
		deht->anLastBlockSize[bucket] = 0;
		return DEHT_STATUS_SUCCESS;
	}

	/* find the last block */
	next_block_disk_ptr = current_block_disk_ptr;
	while (next_block_disk_ptr != NULL_DISK_PTR) {
		current_block_disk_ptr = next_block_disk_ptr;

		if (fread_from(deht->keyFP, current_block_disk_ptr + deht->blockSize - sizeof(DEHT_DISK_PTR),
				&next_block_disk_ptr, sizeof(DEHT_DISK_PTR)) != 0) {
			return DEHT_STATUS_FAIL;
		}
	}
	*block = current_block_disk_ptr;

	/* read last block's pairs */
	if (fread_from(deht->keyFP, current_block_disk_ptr, deht->tmpBlockPairs, 
			deht->pairSize * deht->header.nPairsPerBlock) != 0) {
		return DEHT_STATUS_FAIL;
	}

	/* find last pair */
	for (pair_count = 0; pair_count < deht->header.nPairsPerBlock; pair_count++) {
		/* if data pointer is not initialized, got to end of block, stop scanning*/
		pair_ptr = (DEHT_DISK_PTR*)(deht->tmpBlockPairs + (pair_count * deht->pairSize + deht->header.nBytesPerValidationKey));
		if (*pair_ptr == NULL_DISK_PTR) {
			break;
		}
	}

	if (pair_count == deht->header.nPairsPerBlock) {
		/* block is full */
		*pair = NULL_DISK_PTR;
	} 
	else {
		*pair = current_block_disk_ptr + pair_count * deht->pairSize;
	}

	/* update cache */
	deht->hashPointersForLastBlockImageInMemory[bucket] = current_block_disk_ptr;
	deht->anLastBlockSize[bucket] = pair_count;
	
	return DEHT_STATUS_SUCCESS;
}

/*
 *
 */
static int add_block_to_bucket(DEHT * deht, int bucket, DEHT_DISK_PTR block, 
							DEHT_DISK_PTR * pair)
{
	void * empty_block = NULL;
	DEHT_DISK_PTR next = NULL_DISK_PTR;
	
	memset(deht->tmpBlockPairs, 0, deht->blockSize);

	/* grow file */
	if (fseek(deht->keyFP, 0, SEEK_END) != 0) {
		return DEHT_STATUS_FAIL;
	}
	next = ftell(deht->keyFP);
	if (next < 0) {
		return DEHT_STATUS_FAIL;
	}
	*pair = next;
	if (fwrite(deht->tmpBlockPairs, deht->blockSize, 1, deht->keyFP) != 1) {
		return DEHT_STATUS_FAIL;
	}

	if (block == NULL_DISK_PTR) {
		/*first block in the bucket, need to update head table of pointers*/
		if (deht->hashTableOfPointersImageInMemory == NULL) {
			/* Update head table in disk */
			if (fwrite_at(deht->keyFP, sizeof(deht->header) + sizeof(DEHT_DISK_PTR) * bucket, 
					&next, sizeof(DEHT_DISK_PTR)) != 0) {
				return DEHT_STATUS_FAIL;
			}
		} else {
			/* Update head table in memory */
			deht->hashTableOfPointersImageInMemory[bucket] = next;
		}
	} else {
		/* bucket already exists, update pointer "next" in last block */
		if (fwrite_at(deht->keyFP, block + deht->header.nPairsPerBlock * 
				(deht->header.nBytesPerValidationKey + sizeof(DEHT_DISK_PTR)), 
				&next, sizeof(DEHT_DISK_PTR)) != 0) {
			return DEHT_STATUS_FAIL;
		}
	}
	
	/* update cache */
	deht->hashPointersForLastBlockImageInMemory[bucket] = next;
	deht->anLastBlockSize[bucket] = 0;

	return DEHT_STATUS_SUCCESS;
}

/*
 *
 */
static int bucket_insert_pair(DEHT * deht, int bucket, DEHT_DISK_PTR pair, 
							  unsigned char * validation, const unsigned char * data, 
							  int dataLength)
{
	long eofpos = 0;
	DEHT_DISK_PTR encoded_pos_and_length = NULL_DISK_PTR;
	
	/* write validation at offset `pair` */
	if (fwrite_at(deht->keyFP, pair, validation, deht->header.nBytesPerValidationKey) != 0) {
		return DEHT_STATUS_FAIL;
	}

	/* seek dataFP to end of file */
	if (fseek(deht->dataFP, 0, SEEK_END) != 0) {
		return DEHT_STATUS_FAIL;
	}
	/* encode position and length into a DEHT_DISK_PTR bytes */
	eofpos = ftell(deht->dataFP);
	encoded_pos_and_length = (eofpos << 8) | (dataLength & 0xFF);

	/* write data pointer to offset `pair` */
	if (fwrite(&encoded_pos_and_length, sizeof(encoded_pos_and_length), 1, deht->keyFP) != 1) {
		return DEHT_STATUS_FAIL;
	}
	
	/* write data to dataFP (at end of file) */
	if (fwrite(data, dataLength, 1, deht->dataFP) != 1) {
		return DEHT_STATUS_FAIL;
	}

	/* update cache */
	if (deht->anLastBlockSize[bucket] != -1) {
		deht->anLastBlockSize[bucket] += 1;
	}

	return DEHT_STATUS_SUCCESS;
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
	deht->comparisonHashFunc(key, keyLength, deht->tmpValidationKey);

	/* find the last block in the bucket */
	if (bucket_find_empty_slot(deht, bucket, &block, &pair) != DEHT_STATUS_SUCCESS) {
		return DEHT_STATUS_FAIL;
	}

	/* find the correct place to insert the pair */
	if (pair == NULL_DISK_PTR) {
		/* need to create a new block */
		if (add_block_to_bucket(deht, bucket, block, &pair) != DEHT_STATUS_SUCCESS) {
			return DEHT_STATUS_FAIL;
		}
	}

	return bucket_insert_pair(deht, bucket, pair, deht->tmpValidationKey, 
		data, dataLength);
}

/*****************************************************************************/

static int scan_pairs_for_validation(DEHT * deht, const unsigned char * validation, 
									 DEHT_DISK_PTR * current_block_disk_ptr)
{
	int pair_count = 0;
	DEHT_DISK_PTR * pair_ptr = NULL_DISK_PTR;

	/* read block */
	if (fread_from(deht->keyFP, *current_block_disk_ptr, deht->tmpBlockPairs, deht->blockSize) != 0) {
		return DEHT_STATUS_FAIL;
	}

	/* scan all pairs */
	for (pair_count = 0; pair_count < deht->header.nPairsPerBlock; pair_count++) {
		pair_ptr = (DEHT_DISK_PTR*)(deht->tmpBlockPairs + (pair_count * deht->pairSize + 
			deht->header.nBytesPerValidationKey));
		if (*pair_ptr == NULL_DISK_PTR) {
			/* end of block */
			break;
		}
		if (memcmp(validation, deht->tmpBlockPairs + pair_count * deht->pairSize, 
				deht->header.nBytesPerValidationKey) == 0) {
			return DEHT_STATUS_SUCCESS;
		}
	}

	if (pair_count < deht->header.nPairsPerBlock) {
		/* we reached the end of blocks linked-list without finding the key */
		return DEHT_STATUS_DEADEND;
	}

	*current_block_disk_ptr = *(DEHT_DISK_PTR*)(deht->tmpBlockPairs + deht->blockSize - sizeof(DEHT_DISK_PTR));
	return DEHT_STATUS_NOT_NEEDED;
}

/*
 *
 */
static int bucket_find_key(DEHT * deht, int bucket, unsigned char * validation, 
					   DEHT_DISK_PTR * pair, DEHT_DISK_PTR * block)
{
	int pair_count = 0;
	int res = 0;
	DEHT_DISK_PTR current_block_disk_ptr = NULL_DISK_PTR;
	DEHT_DISK_PTR next_block_disk_ptr = NULL_DISK_PTR;

	/* find the first block */
	if (deht->hashTableOfPointersImageInMemory != NULL) {
		/* use cached head table if possible */
		current_block_disk_ptr = deht->hashTableOfPointersImageInMemory[bucket];
	} else {
		if (fread_from(deht->keyFP, sizeof(deht->header) + bucket * sizeof(DEHT_DISK_PTR), 
				&current_block_disk_ptr, sizeof(DEHT_DISK_PTR)) != 0) {
			return DEHT_STATUS_FAIL;
		}
	}
	
	next_block_disk_ptr = current_block_disk_ptr;
	while (next_block_disk_ptr != NULL_DISK_PTR) {
		/* scan all the pairs of the current block and update next_block_disk_ptr
		 * to the next block in the linked list */
		res = scan_pairs_for_validation(deht, validation, &next_block_disk_ptr);
		
		switch (res) {
			case DEHT_STATUS_SUCCESS:
				/* key found */
				*block = current_block_disk_ptr;
				*pair = current_block_disk_ptr + pair_count * deht->pairSize;
				return DEHT_STATUS_SUCCESS;

			case DEHT_STATUS_DEADEND:
				/* we've reached the end of the linked list without finding the key */
				break;

			case DEHT_STATUS_NOT_NEEDED:
				/* didn't find key yet, but we can keep on going */
				continue;

			default:
				/* some kind of error */
				return res;
		}
	}

	/* key not found (but let's update the cache anyway) */
	deht->hashPointersForLastBlockImageInMemory[bucket] = current_block_disk_ptr;
	deht->anLastBlockSize[bucket] = pair_count;
	return DEHT_STATUS_NOT_NEEDED;
}

/*
 *
 */
static int read_pair_data(DEHT * deht, DEHT_DISK_PTR pair, unsigned char * data, 
						  int dataMaxAllowedLength)
{
	DEHT_DISK_PTR data_ptr = NULL_DISK_PTR;
	int data_len;
	DEHT_DISK_PTR offset;

	if (fread_from(deht->keyFP, pair + deht->header.nBytesPerValidationKey, &data_ptr, 
			sizeof(DEHT_DISK_PTR)) != 0) {
		return DEHT_STATUS_FAIL;
	}

	data_len = data_ptr & 0xFF;
	offset = data_ptr >> 8;

	if (data_len > dataMaxAllowedLength) {
		fprintf(stderr, "query_DEHT: given buffer too short");
		return DEHT_STATUS_FAIL;
	}
	if (fread_from(deht->dataFP, offset, data, data_len) != 0) {
		return DEHT_STATUS_FAIL;
	}
	
	return data_len;
}

/*
 *
 */
static int find_pair_by_key(DEHT * deht, const unsigned char *key, int keyLength,
					DEHT_DISK_PTR * pair, DEHT_DISK_PTR * block)
{
	int bucket;
	bucket = deht->hashFunc(key, keyLength, deht->header.numEntriesInHashTable);
	deht->comparisonHashFunc(key, keyLength, deht->tmpValidationKey);
	return bucket_find_key(deht, bucket, deht->tmpValidationKey, pair, block);
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
















