#ifndef INILOADER_H_INCLUDED
#define INILOADER_H_INCLUDED

#include <stdio.h>
#include "misc.h"

#define INI_STATUS_OK       (0)
#define INI_STATUS_ERROR    (-1)


typedef struct {
	char key[MAX_INPUT_BUFFER];
	char value[MAX_INPUT_BUFFER];
} keyvalue_t;

typedef struct {
	int num_of_lines;
	keyvalue_t * lines;
} inifile_t;

int ini_load(inifile_t * ini, const char * filename);
const char * ini_get(const inifile_t * ini, const char * key);
int ini_get_integer(const inifile_t * ini, const char * key, int * value);
void ini_finalize(inifile_t * ini);



#endif /* INILOADER_H_INCLUDED */
