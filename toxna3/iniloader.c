#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#ifdef _MSC_VER
	#define strcasecmp _stricmp
#else
	#include <strings.h>
#endif
#include "iniloader.h"


/*
 * checks for comment and empty lines in the INI file
 */
static int is_comment_line(const char * text)
{
	const char *p = text;
	for (; *p != '\0'; p++) {
		if (isspace(*p)) {
			continue;
		}
		if (*p == ';') {
			return 1;
		}
		return 0;
	}
	return 1;
}

static int parse_line(char * line, char ** key, char ** value)
{
	char * ch, * rkey;

	/* parse line */
	*key = line;
	rkey = NULL;
	*value = NULL;
	for (ch = line; ch != '\0'; ch++) {
		if (*ch == '=') {
			rkey = ch - 1;
			*ch = '\0';
			*value = ch + 1;
			break;
		}
	}
	if (*value == NULL) {
		fprintf(stderr, "Syntax error in INI file: '%s'\n", line);
		return INI_STATUS_ERROR;
	}

	/* trim whitespace */
	for (; **key == ' ' || **key == '\t'; (*key)++);
	for (; *rkey == ' ' || *rkey == '\t'; *(rkey--) = '\0');
	for (; **value == ' ' || **value == '\t'; (*value)++);
	for (ch = *value + strlen(*value) - 1;
			*ch == ' ' || *ch == '\t' || *ch == '\n' || *ch == '\r'; *(ch--) = '\0');

	return INI_STATUS_OK;
}

int ini_load(inifile_t * ini, const char * filename)
{
	char line[MAX_INPUT_BUFFER];
	char * key, * value;
	int allocated_lines = 50;
	keyvalue_t * tmp = NULL;

	FILE * f = fopen(filename, "r");

	if (f == NULL) {
		perror(filename);
		return INI_STATUS_ERROR;
	}

	ini->num_of_lines = 0;
	ini->lines = (keyvalue_t*)malloc(sizeof(keyvalue_t) * allocated_lines);
	if (ini->lines == NULL) {
		fprintf(stderr, "allocating ini->lines failed\n");
	}

	while (1) {
		if (fgets(line, sizeof(line)-1, f) == NULL) {
			if (feof(f)) {
				break;
			}
			else {
				perror("ini_load: fgets failed");
				goto cleanup;
			}
		}
		if (is_comment_line(line)) {
			continue;
		}
		if (parse_line(line, &key, &value) != INI_STATUS_OK) {
			goto cleanup;
		}

		/* grow lines array if needed */
		if (ini->num_of_lines >= allocated_lines) {
			allocated_lines *= 2;
			tmp = realloc(ini->lines, sizeof(keyvalue_t) * allocated_lines);
			if (tmp == NULL) {
				fprintf(stderr, "reallocating ini->lines failed\n");
				goto cleanup;
			}
			ini->lines = tmp;
		}

		strncpy(ini->lines[ini->num_of_lines].key, key, MAX_INPUT_BUFFER);
		strncpy(ini->lines[ini->num_of_lines].value, value, MAX_INPUT_BUFFER);
		ini->num_of_lines += 1;
	}
	fclose(f);
	
	return INI_STATUS_OK;

cleanup:
	if (ini->lines != NULL) {
		free(ini->lines);
		ini->lines = NULL;
	}
	return INI_STATUS_ERROR;
}

const char * ini_get(const inifile_t * ini, const char * key)
{
	int i;

	for (i = 0; i < ini->num_of_lines; i++) {
		if (strcasecmp(ini->lines[i].key, key) == 0) {
			return ini->lines[i].value;
		}
	}
	return NULL;
}

int ini_get_integer(const inifile_t * ini, const char * key, int * value)
{
	const char * text = ini_get(ini, key);
	if (text == NULL) {
		return INI_STATUS_ERROR;
	}
	*value = atoi(text);
	return INI_STATUS_OK;
}


void ini_finalize(inifile_t * ini)
{
	ini->num_of_lines = 0;
	if (ini->lines != NULL) {
		free(ini->lines);
		ini->lines = NULL;
	}
}

/*
int main()
{
	int i;
	inifile_t ini;

	if (ini_load(&ini, "test.ini") != INI_STATUS_OK) {
		return 1;
	}
	for (i = 0; i < ini.num_of_lines; i++) {
		printf("'%s' = '%s'\n", ini.lines[i].key, ini.lines[i].value);
	}
	printf("'%s'\n", ini_get(&ini, "num_of_r"));

	return 0;
}
*/
