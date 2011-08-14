#include <string.h>
#include <stdlib.h>
#include "deht.h"
#include "misc.h"
#include <assert.h>

#define NULL_DISK_PTR              0
#define DEHT_STATUS_DEADEND        2

/*
 * allocate (malloc) a new DEHT object and initialize all the fields to
 * default values. also open the key and data files.
 */
static DEHT * alloc_DEHT(const char *prefix, const char * data_filename,
		const char * key_filename, hashKeyIntoTableFunctionPtr hashfun,
        hashKeyforEfficientComparisonFunctionPtr validfun, const char * mode)
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
	deht->keyFP = fopen(key_filename, mode);
	if (deht->keyFP == NULL) {
		/*perror("fopen of key file");*/
		perror(prefix);
		goto cleanup1;
	}

	deht->dataFP = fopen(data_filename, mode);
	if (deht->dataFP == NULL) {
		/*perror("fopen of data file");*/
		perror(prefix);
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
	deht->tmpValidationKey = NULL;
cleanup3:
	free(deht->anLastBlockSize);
	deht->anLastBlockSize = NULL;
cleanup2:
	free(deht->hashPointersForLastBlockImageInMemory);
	deht->hashPointersForLastBlockImageInMemory = NULL;
cleanup1:
	fclose(deht->dataFP);
	deht->dataFP = NULL;
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
	char key_filename[MAX_INPUT_BUFFER];
	char data_filename[MAX_INPUT_BUFFER];
	FILE * f = NULL;
	DEHT * deht = NULL;
	DEHT_DISK_PTR * empty_head_table = NULL;
	size_t entries_written = 0;

	init_deht_files(prefix, key_filename, data_filename);

	/* make sure the two files do not already exist */
	if ((f = fopen(key_filename, "r")) != NULL) {
		fclose(f);
		fprintf(stderr, "Error: file \"%s\" already exist.\n", key_filename);
		return NULL;
	}
	if ((f = fopen(data_filename, "r")) != NULL) {
		fclose(f);
		fprintf(stderr, "Error: file \"%s\" already exist.\n", data_filename);
		return NULL;
	}

	deht = alloc_DEHT(prefix, data_filename, key_filename, hashfun, validfun, "w+");
	if (deht == NULL) {
		return NULL;
	}

	/* init header */
	deht->header.numEntriesInHashTable = numEntriesInHashTable;
	deht->header.nPairsPerBlock = nPairsPerBlock;
	deht->header.nBytesPerValidationKey = nBytesPerKey;
	strncpy(deht->header.sHashName, HashName, sizeof(deht->header.sHashName));

	if (init_deht_caches(deht) != 0) {
		goto cleanup;
	}

	/* write header and empty head table */
	if (fwrite(&(deht->header), sizeof(deht->header), 1, deht->keyFP) != 1) {
		/*perror("could not write header to keys file");*/
		perror(deht->sPrefixFileName);
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
		/*perror("could not write empty head table to keys file");*/
		perror(deht->sPrefixFileName);
		goto cleanup;
	}
	free(empty_head_table);

	/* Initialize data file */
	if (fputc('\0', deht->dataFP) != '\0') {
		/*perror("could not initialize data file");*/
		perror(deht->sPrefixFileName);
		goto cleanup;
	}

	/* Flush the files */
	fflush(deht->keyFP);
	fflush(deht->dataFP);

	return deht;

cleanup:
	if (empty_head_table != NULL) {
		free(empty_head_table);
	}
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
	char key_filename[MAX_INPUT_BUFFER];
	char data_filename[MAX_INPUT_BUFFER];
	FILE * f = NULL;
	DEHT * deht = NULL;

	init_deht_files(prefix, key_filename, data_filename);

	/* make sure the two files do exist */
	f = fopen(key_filename, "r");
	if (f == NULL) {
		/*perror("key file does not exist");*/
		perror(prefix);
		return NULL;
	}
	fclose(f);
	f = fopen(data_filename, "r");
	if (f == NULL) {
		/*perror("data file does not exist");*/
		perror(prefix);
		return NULL;
	}
	fclose(f);

	deht = alloc_DEHT(prefix, data_filename, key_filename, hashfun, validfun, "r+");
	if (deht == NULL) {
		return NULL;
	}

	/* read header from file */
	if (fread(&deht->header, sizeof(deht->header), 1, deht->keyFP) != 1) {
		/*perror("failed to read header from key file");*/
		perror(deht->sPrefixFileName);
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
	if (deht->keyFP != NULL) {
		fclose(deht->keyFP);
		deht->keyFP = NULL;
	}
	if (deht->dataFP != NULL) {
		fclose(deht->dataFP);
		deht->dataFP = NULL;
	}

	/* since we malloc()'ed the deht, it's time for us to free() it */
	free(deht);
}

/*****************************************************************************/

/*
 * fseek+fread in a single function
 */
static int fread_from(const char * prefix, FILE * file, DEHT_DISK_PTR offset,
					  void * data, size_t size)
{
	if (fseek(file, (long)offset, SEEK_SET) != 0) {
		/*perror("fseek");*/
		perror(prefix);
		return -1;
	}
	if (fread(data, size, 1, file) != 1) {
		/*perror("fread");*/
		perror(prefix);
		return -1;
	}
	return 0;
}

/*
 * fseek+fwrite in a single function
 */
static int fwrite_at(const char * prefix, FILE * file, DEHT_DISK_PTR offset,
					 const void * data, size_t size)
{
	if (fseek(file, (long)offset, SEEK_SET) != 0) {
		/*perror("fseek");*/
		perror(prefix);
		return -1;
	}
	if (fwrite(data, size, 1, file) != 1) {
		/*perror("fwrite");*/
		perror(prefix);
		return -1;
	}
	return 0;
}

/*
 * finds the first empty slot in the block (the first place where we can insert).
 * if the bucket is all full, this function will succeed but set the output
 * parameters to NULL_DISK_PTR, in which case you need to call add_block_to_bucket.
 *
 * Parameters:
 *    * deht - the DEHT
 *    * bucket - the bucket's index
 * Output Parameters:
 *    * block - disk pointer to the block. will be set to NULL if bucket is full
 *    * pair - disk pointer to the pair. will be set to NULL if bucket is full
 * Returns: DEHT_STATUS_SUCCESS if successfully located a slot or all blocks in
 * the bucket are full (in this case, block and pair will be set to NULL_DISK_PTR).
 */
static int bucket_find_empty_slot(DEHT * deht, int bucket, DEHT_DISK_PTR * block,
								  DEHT_DISK_PTR * pair)
{
	int pair_count = 0;
	DEHT_DISK_PTR current_block_disk_ptr = NULL_DISK_PTR;
	DEHT_DISK_PTR next_block_disk_ptr = NULL_DISK_PTR;
	DEHT_DISK_PTR * pair_ptr = NULL_DISK_PTR;

	/* try to serve request from cache */
	if (deht->anLastBlockSize[bucket] != -1 && deht->hashTableOfPointersImageInMemory != NULL) {
		*block = deht->hashPointersForLastBlockImageInMemory[bucket];
		pair_count = deht->anLastBlockSize[bucket];

		if (pair_count == deht->header.nPairsPerBlock) {
			/* the last block is full */
			*pair = NULL_DISK_PTR;
		} else {
			/* there still is room in the last block */
			*pair = *block + deht->anLastBlockSize[bucket] * deht->pairSize;
		}
		return DEHT_STATUS_SUCCESS;
	}

	/* find the first block */
	if (deht->hashTableOfPointersImageInMemory != NULL) {
		/* use cached head table if possible */
		current_block_disk_ptr = deht->hashTableOfPointersImageInMemory[bucket];
	} else {
		if (fread_from(deht->sPrefixFileName, deht->keyFP, sizeof(deht->header) +
				bucket * sizeof(DEHT_DISK_PTR), &current_block_disk_ptr,
				sizeof(DEHT_DISK_PTR)) != 0) {
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

		if (fread_from(deht->sPrefixFileName, deht->keyFP, current_block_disk_ptr +
				deht->blockSize - sizeof(DEHT_DISK_PTR), &next_block_disk_ptr,
				sizeof(DEHT_DISK_PTR)) != 0) {
			return DEHT_STATUS_FAIL;
		}
	}
	*block = current_block_disk_ptr;

	/* read last block's pairs */
	if (fread_from(deht->sPrefixFileName, deht->keyFP, current_block_disk_ptr, deht->tmpBlockPairs,
			deht->blockSize) != 0) {
		return DEHT_STATUS_FAIL;
	}

	/* find last pair */
	for (pair_count = 0; pair_count < deht->header.nPairsPerBlock; pair_count++) {
		/* if data pointer is not initialized, got to end of block, stop scanning*/
		pair_ptr = (DEHT_DISK_PTR*)(deht->tmpBlockPairs + (pair_count * deht->pairSize +
			deht->header.nBytesPerValidationKey));
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
 * grows a bucket (adds a new block to it)
 *
 * Parameters:
 *    * deht - the DEHT
 *    * bucket - the bucket's index
 *    * block - disk pointer to block
 * Output Parameters:
 *    * pair - disk pointer to pair (will point to the first pair in the
 *      newly-added block)
 * Returns: DEHT_STATUS_SUCCESS if successfully added a new block to bucket,
 * DEHT_STATUS_FAIL otherwise
 */
static int add_block_to_bucket(DEHT * deht, int bucket, DEHT_DISK_PTR block,
							DEHT_DISK_PTR * pair)
{
	DEHT_DISK_PTR next = NULL_DISK_PTR;

	memset(deht->tmpBlockPairs, 0, deht->blockSize);

	/* grow key file */
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
		/* first block in the bucket, need to update head table of pointers */
		if (deht->hashTableOfPointersImageInMemory == NULL) {
			/* update head table in disk */
			if (fwrite_at(deht->sPrefixFileName, deht->keyFP, sizeof(deht->header) +
					sizeof(DEHT_DISK_PTR) * bucket, &next, sizeof(DEHT_DISK_PTR)) != 0) {
				return DEHT_STATUS_FAIL;
			}
		} else {
			/* update head table in memory */
			deht->hashTableOfPointersImageInMemory[bucket] = next;
		}
	} else {
		/* bucket already exists, update pointer "next" in last block */
		if (fwrite_at(deht->sPrefixFileName, deht->keyFP, block + deht->blockSize - sizeof(DEHT_DISK_PTR),
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
 * inserts the given pair (key + data) into the table
 *
 * Parameters:
 *    * deht - the DEHT
 *    * bucket - the bucket index
 *    * pair - disk pointer to the pair
 *    * validation - the validation data of the key
 *    * data - the data associated with the key
 *    * dataLength - the length of the data in bytes
 * Returns: DEHT_STATUS_SUCCESS if successfully inserted key and data,
 * DEHT_STATUS_FAIL otherwise.
 */
static int bucket_insert_pair(DEHT * deht, int bucket, DEHT_DISK_PTR pair,
							  unsigned char * validation, const unsigned char * data,
							  int dataLength)
{
	long eofpos = 0;
	DEHT_DISK_PTR encoded_pos_and_length = NULL_DISK_PTR;

	/* instructions say max password is 256 bytes, and we rely on this to
	 * converse disk space */
	assert(dataLength < 256);

	/* write validation at offset `pair` */
	if (fwrite_at(deht->sPrefixFileName, deht->keyFP, pair, validation,
			deht->header.nBytesPerValidationKey) != 0) {
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
	if (dataLength == 0) {
		/* the empty password -- nothing to write to dataFP */
	}
	else if (fwrite(data, dataLength, 1, deht->dataFP) != 1) {
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

/*
 * locate a pair by the given key. it goes over all of the pairs in all of the
 * blocks of the bucket, and attempts to match the validation data.
 *
 * Parameters:
 *     * deht - the DEHT
 *     * bucket - the bucket index in the table of pointers
 *     * validation - the validation data to match
 * Output Parameters:
 *     * pair - disk pointer to the pair
 *     * block - disk pointer to the block
 * Returns: DEHT_STATUS_SUCCESS if the key was found; DEHT_STATUS_NOT_NEEDED if the
 * key was not found; DEHT_STATUS_FAIL on error.
 */
static int bucket_find_key(DEHT * deht, int bucket, const unsigned char * validation,
					   DEHT_DISK_PTR * pair, DEHT_DISK_PTR * block)
{
	int pair_count = 0;
	DEHT_DISK_PTR current_block_disk_ptr = NULL_DISK_PTR;
	DEHT_DISK_PTR next_block_disk_ptr = NULL_DISK_PTR;
	DEHT_DISK_PTR * pair_ptr = NULL_DISK_PTR;

	/* find the first block */
	if (deht->hashTableOfPointersImageInMemory != NULL) {
		/* use cached head table if possible */
		current_block_disk_ptr = deht->hashTableOfPointersImageInMemory[bucket];
	} else {
		if (fread_from(deht->sPrefixFileName, deht->keyFP, sizeof(deht->header) +
				bucket * sizeof(DEHT_DISK_PTR),  &current_block_disk_ptr,
				sizeof(DEHT_DISK_PTR)) != 0) {
			return DEHT_STATUS_FAIL;
		}
	}

	next_block_disk_ptr = current_block_disk_ptr;
	while (next_block_disk_ptr != NULL_DISK_PTR) {
		current_block_disk_ptr = next_block_disk_ptr;

		/* read block */
		if (fread_from(deht->sPrefixFileName, deht->keyFP, current_block_disk_ptr,
				deht->tmpBlockPairs, deht->blockSize) != 0) {
			return DEHT_STATUS_FAIL;
		}

		/* scan all pairs */
		for (pair_count = 0; pair_count < deht->header.nPairsPerBlock; pair_count++) {
			pair_ptr = (DEHT_DISK_PTR*)(deht->tmpBlockPairs + pair_count * deht->pairSize +
				deht->header.nBytesPerValidationKey);
			if (*pair_ptr == NULL_DISK_PTR) {
				/* end of block */
				break;
			}
			if (memcmp(validation, deht->tmpBlockPairs + pair_count * deht->pairSize,
					deht->header.nBytesPerValidationKey) == 0) {
				/* key found */
				*block = current_block_disk_ptr;
				*pair = current_block_disk_ptr + pair_count * deht->pairSize;
				return DEHT_STATUS_SUCCESS;
			}
		}
		if (pair_count < deht->header.nPairsPerBlock) {
			/* reached an empty slot */
			break;
		}

		next_block_disk_ptr = *(DEHT_DISK_PTR*)(deht->tmpBlockPairs +
			deht->blockSize - sizeof(DEHT_DISK_PTR));
	}

	/* key not found (but let's update the cache anyway) */
	deht->hashPointersForLastBlockImageInMemory[bucket] = current_block_disk_ptr;
	deht->anLastBlockSize[bucket] = pair_count;
	return DEHT_STATUS_NOT_NEEDED;
}

/*
 * reads the pair's data (the value associated with the key).
 * note: if the data buffer is too small, the data will be truncated
 *
 * Parameters:
 *     * deht - the DEHT
 *     * pair - disk pointer to the pair
 *     * dataMaxAllowedLength - the maximal size of the data
 * Output Parameters:
 *     * data - byte buffer that will hold the value
 * Returns: the number of bytes retrieved from the table, or DEHT_STATUS_FAIL
 * on failure
 */
static int read_pair_data(DEHT * deht, DEHT_DISK_PTR pair, unsigned char * data,
						  int dataMaxAllowedLength)
{
	DEHT_DISK_PTR data_ptr = NULL_DISK_PTR;
	int data_len;
	DEHT_DISK_PTR offset;

	/* read from keyFP the pointer to the data in dataFP */
	if (fread_from(deht->sPrefixFileName, deht->keyFP, pair + deht->header.nBytesPerValidationKey,
			&data_ptr, sizeof(DEHT_DISK_PTR)) != 0) {
		return DEHT_STATUS_FAIL;
	}

	/* decode length and offset */
	data_len = data_ptr & 0xFF;
	offset = data_ptr >> 8;

	if (data_len > dataMaxAllowedLength) {
		/* truncate output data */
		data_len = dataMaxAllowedLength;
		/*fprintf(stderr, "query_DEHT: given buffer too short");
		return DEHT_STATUS_FAIL;*/
	}
	if (fread_from(deht->sPrefixFileName, deht->dataFP, offset, data, data_len) != 0) {
		return DEHT_STATUS_FAIL;
	}

	return data_len;
}

/*
 * locates a pair by the given key.
 *
 * Parameters:
 *	   * deht - the DEHT
 *     * key - the key (non null-terminated)
 *     * keyLength - the key's length in bytes
 * Output Parameters:
 *     * pair - disk pointer to the key's pair
 *     * block - disk pointer to the key's block
 * Returns DEHT_STATUS_SUCCESS if the key is found, DEHT_STATUS_NOT_NEEDED is the key
 * is not found, and DEHT_STATUS_FAIL on error
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

	/* some kind of error or key not found */
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
		/*perror("fseek failed");*/
		perror(deht->sPrefixFileName);
		goto cleanup;
	}

	readcount = fread(deht->hashTableOfPointersImageInMemory, sizeof(DEHT_DISK_PTR),
		deht->header.numEntriesInHashTable, deht->keyFP);
	if (readcount != deht->header.numEntriesInHashTable) {
		fprintf(stderr, "failed to read DEHT pointers table");
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
		/*perror("fseek failed");*/
		perror(deht->sPrefixFileName);
		return DEHT_STATUS_FAIL;
	}

	written = fwrite(deht->hashTableOfPointersImageInMemory, sizeof(DEHT_DISK_PTR),
		deht->header.numEntriesInHashTable, deht->keyFP);
	if (written != deht->header.numEntriesInHashTable) {
		/*perror("failed to write everything");*/
		perror(deht->sPrefixFileName);
		return DEHT_STATUS_FAIL;
	}
	fflush(deht->keyFP);

	free(deht->hashTableOfPointersImageInMemory);
	deht->hashTableOfPointersImageInMemory = NULL;
	return DEHT_STATUS_SUCCESS;
}

/*****************************************************************************/
/* Additions for part 3 */
/*****************************************************************************/

/*
 * finds the first pair that matches the validation data, or the last pair in the bucket.
 */
int bucket_multi_find_key(DEHT * deht, int bucket, const unsigned char * validation, 
						  DEHT_DISK_PTR * block, DEHT_DISK_PTR * pair)
{
	int pair_count = 0; 
	DEHT_DISK_PTR current_block_disk_ptr = NULL_DISK_PTR;
	DEHT_DISK_PTR next_block_disk_ptr = NULL_DISK_PTR;
	DEHT_DISK_PTR * pair_ptr = NULL_DISK_PTR;

	/* find the first block, if this is the first call */
	if (*block == NULL_DISK_PTR) {
		if (deht->hashTableOfPointersImageInMemory != NULL) {
			/* use cached head table if possible */
			current_block_disk_ptr = deht->hashTableOfPointersImageInMemory[bucket];
		} 
		else {
			if (fread_from(deht->sPrefixFileName, deht->keyFP, sizeof(deht->header) + 
					bucket * sizeof(DEHT_DISK_PTR), &current_block_disk_ptr, 
					sizeof(DEHT_DISK_PTR)) != 0) {
				return DEHT_STATUS_FAIL;
			}
		}
	} 
	else {
		/* otherwise continue from where we stopped previously */
		current_block_disk_ptr = *block;
		pair_count = (int)(((*pair - *block) / deht->pairSize) + 1);
	}

	next_block_disk_ptr = current_block_disk_ptr;
	while (next_block_disk_ptr != NULL_DISK_PTR) {
		current_block_disk_ptr = next_block_disk_ptr;

		/* read block */
		if (fread_from(deht->sPrefixFileName, deht->keyFP, current_block_disk_ptr,
				deht->tmpBlockPairs, deht->blockSize) != 0) {
			return DEHT_STATUS_FAIL;
		}

		/* scan all pairs */
		for (; pair_count < deht->header.nPairsPerBlock; pair_count++) {
			pair_ptr = (DEHT_DISK_PTR*)(deht->tmpBlockPairs + pair_count * deht->pairSize +
				deht->header.nBytesPerValidationKey);
			if (*pair_ptr == NULL_DISK_PTR) {
				/* end of block */
				break;
			}
			if (memcmp(validation, deht->tmpBlockPairs + pair_count * deht->pairSize,
					deht->header.nBytesPerValidationKey) == 0) {
				/* key found */
				*block = current_block_disk_ptr;
				*pair = current_block_disk_ptr + pair_count * deht->pairSize;
				return DEHT_STATUS_SUCCESS;
			}
		}
		if (pair_count < deht->header.nPairsPerBlock) {
			/* reached an empty slot */
			break;
		}

		next_block_disk_ptr = *(DEHT_DISK_PTR*)(deht->tmpBlockPairs +
			deht->blockSize - sizeof(DEHT_DISK_PTR));
	}

	/* key not found (but let's update the cache anyway) */
	deht->hashPointersForLastBlockImageInMemory[bucket] = current_block_disk_ptr;
	deht->anLastBlockSize[bucket] = pair_count;
	return DEHT_STATUS_NOT_NEEDED;
}


#define MIN(a,b) (((a) < (b)) ? (a) : (b))

int multi_query_DEHT(DEHT *deht, const unsigned char * key, int keyLength, 
					 masrek_t * masrek)
{
	int res;
	int bucket; 
	int item_index = 0;
	DEHT_DISK_PTR pair = NULL_DISK_PTR;
	DEHT_DISK_PTR block = NULL_DISK_PTR;
	DEHT_DISK_PTR data_ptr = NULL_DISK_PTR;
	unsigned char * masrek_ptr = (unsigned char*)masrek->buffer;
	int remaining = masrek->buffer_size;

	bucket = deht->hashFunc(key, keyLength, deht->header.numEntriesInHashTable);
	deht->comparisonHashFunc(key, keyLength, deht->tmpValidationKey);

	/* scan the bucket for the given key */
	while (item_index < masrek->max_items) {
		res = bucket_multi_find_key(deht, bucket, deht->tmpValidationKey, 
			&block, &pair);

		if (res == DEHT_STATUS_SUCCESS) {
			/* found a matching value */
			if (remaining <= 0) {
				/* masrek buffer is full */
				break;
			}

			res = read_pair_data(deht, pair, masrek_ptr, remaining);
			if (res < 0) {
				return -1;
			}
			masrek->items[item_index].buffer = masrek_ptr;
			masrek->items[item_index].length = res;
			item_index++;
			remaining -= res;
			masrek_ptr += res;
		}
		else if (res == DEHT_STATUS_NOT_NEEDED) {
			/* no more matches */
			break;
		}
		else {
			/* some kind of error */
			return -1;
		}
	};

	return item_index;
}


