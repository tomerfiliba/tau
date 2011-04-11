#include <stdio.h>
#include <string.h>
#include "md5.h"


/*
 * constants
 */
#define DIGEST_SIZE   (16)
#define MAX_LINE_SIZE (150)
#define MAX_USERNAME  (MAX_LINE_SIZE)
#define MAX_USERS     (7)
#define WHITESPACE    " \t\n"

/*
 * types
 */
typedef enum {
	FALSE = 0,
	TRUE  = 1
} bool_t;

struct user
{
	char username[MAX_USERNAME + 1];
	unsigned char digested_password[DIGEST_SIZE];
};

/*
 * global variables: users array and number of users
 */
struct user users_array[MAX_USERS];
int num_of_users = 0;

/*
 * shorthand method for calculating the digest of the given buffer
 */
static void calc_md5(unsigned char digest[DIGEST_SIZE], void * buf, size_t size)
{
	MD5_CTX ctx;
	MD5Init(&ctx);
	MD5Update(&ctx, (unsigned char*)buf, size);
	MD5Final(&ctx);
	memcpy(digest, ctx.digest, DIGEST_SIZE);
}

/*
 * prints the given digest in hex format
 */
static void print_md5(unsigned char digest[DIGEST_SIZE])
{
	int i;
	for (i = 0; i < DIGEST_SIZE; i++) {
		printf("%02x", digest[i]);
	}
}

/*
 * print command: displays the currently existing users
 */
static void cmd_print(void)
{
	int i;
	for (i = 0; i < num_of_users; i++) {
		printf("%s\t", users_array[i].username);
		print_md5(users_array[i].digested_password);
		printf("\n");
	}
}

/*
 * checks if the username is valid (returns TRUE or FALSE)
 */
static bool_t is_valid_username(const char * username)
{
	const char *p;
	for(p = username; *p != '\0'; p++) {
		char ch = *p;
		if ((ch < 'a' || ch > 'z') && (ch < 'A' || ch > 'Z')) {
			return FALSE;
		}
	}
	return TRUE;
}

/*
 * checks if the password is valid (returns TRUE or FALSE)
 */
static bool_t is_valid_password(const char * password)
{
	const char *p;
	for(p = password; *p != '\0'; p++) {
		char ch = *p;
		if ((ch < 'a' || ch > 'z') && (ch < 'A' || ch > 'Z') && (ch < '0' || ch > '9')) {
			return FALSE;
		}
	}
	return TRUE;
}

/*
 * return the index of the user in the users_array, or -1 if not found
 */
static int find_user(const char * username)
{
	int i;
	for(i=0; i < num_of_users; i++) {
		if(strcmp(username, users_array[i].username) == 0) {
			return i;
		}
	}
	return -1;
}

/*
 * returns true iff str is made of only whitespace chars
 */
static bool_t is_all_whitespace(const char * str)
{
	const char *p;
	for(p = str; *p != '\0'; p++) {
		if (strchr(WHITESPACE, *p) == NULL) {
			/* we found a non-whitespace char */
			return FALSE;
		}
	}
	return TRUE;
}

/*
 * reads the username and password from the command line and checks for errors.
 * returns TRUE if everything went OK, FALSE otherwise (in this case, an error
 * message is printed)
 */
static bool_t get_username_and_password(char **username, char **password)
{
	char *tail = NULL;
	*username = NULL;
	*password = NULL;

	*username = strtok(NULL, WHITESPACE);
	if (*username == NULL) {
		printf("Error: Missing username\n");
		return FALSE;
	}
	*password = strtok(NULL, WHITESPACE);
	if (*password == NULL) {
		printf("Error: Missing password\n");
		return FALSE;
	}
	/* read up to the terminator (fgets places \0 there) */
	tail = strtok(NULL, "\0");
	if (tail != NULL && !is_all_whitespace(tail)) {
		printf("Error: Expected end of line\n");
		return FALSE;
	}
	if (!is_valid_username(*username)) {
		printf("Error: Illegal username\n");
		return FALSE;
	}
	if (!is_valid_password(*password)) {
		printf("Error: Illegal password\n");
		return FALSE;
	}

	return TRUE;
}

/* 
 * add user command: read username, password, validate prerequisites and 
 * add the user with its hashed password 
 */
void cmd_add(void)
{
	char *username = NULL;
	char *password = NULL;
	int i = num_of_users;

	if (!get_username_and_password(&username, &password)) {
		/* already prints error message */
		return;
	}
	if (num_of_users >= MAX_USERS) {
		printf("Error: Not enough space for new users\n");
		return;
	}
	if (find_user(username) >= 0) {
		printf("Error: Username already exists\n");
		return;
	}

	strcpy(users_array[i].username, username);
	calc_md5(users_array[i].digested_password, (unsigned char*)password, strlen(password));
	num_of_users++;
}

/*
 * login command: read username and password, validate prerequisites, and 
 * check if the digest of the given password matches the known digest 
 * of the password
 */
void cmd_login(void)
{
	char *username = NULL;
	char *password = NULL;
	int i;
	unsigned char digest[DIGEST_SIZE];

	if (!get_username_and_password(&username, &password)) {
		/* already prints error message */
		return;
	}
	i = find_user(username);
	if (i < 0) {
		printf("Error: Nonexistent user\n");
		return;
	}

	calc_md5(digest, password, strlen(password));

	if (memcmp(digest, users_array[i].digested_password, DIGEST_SIZE) == 0) {
		printf("approved.\n");
	}
	else {
		printf("denied.\n");
	}
}


int main (int argc, char *argv[])
{
	char line[MAX_LINE_SIZE + 1];
	char *cmd = NULL;

	/* make sure the entire array is initialized to zeros */
	memset(users_array, 0, sizeof(users_array));

	/* main loop */
	while (1) {
		printf(">> ");
		if (fgets(line, MAX_LINE_SIZE, stdin) == NULL) {
			break; /* EOF */
		}
		cmd = strtok(line, WHITESPACE);
		if (cmd == NULL) {
			continue; /* empty line */
		}

		if (strcmp(cmd, "quit") == 0) {
			break;
		}
		else if (strcmp(cmd, "print") == 0) {
			cmd_print();
		}
		else if (strcmp(cmd, "add") == 0) {
			cmd_add();
		}
		else if (strcmp(cmd, "login") == 0) {
			cmd_login();
		}
		else {
			printf("Error: Illegal command\n");
		}
	}

	return 0;
}
