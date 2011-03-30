#include <stdio.h>
#include <string.h>
#include "md5.h"


/*
 * constants
 */
#define DIGEST_SIZE  (16)
#define MAX_USERS    (7)

/*
 * types
 */
typedef enum {
	FALSE = 0,
	TRUE  = 1
} bool_t;

struct user
{
	char username[80];
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
void calc_md5(unsigned char digest[DIGEST_SIZE], void * buf, size_t size)
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
void print_md5(unsigned char digest[DIGEST_SIZE])
{
	int i;
	for (i = 0; i < DIGEST_SIZE; i++) {
		printf("%02x", digest[i]);
	}
}

/*
 * print command: displays the currently existing users
 */
void cmd_print(void)
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
bool_t is_valid_username(const char * username)
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
bool_t is_valid_password(const char * password)
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
int find_user(const char * username)
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
 * add user command: read username, password, validate prerequisites and 
 * add the user with its hashed password 
 */
void cmd_add(void)
{
	char username[81];
	char password[81];
	int i = num_of_users;

	scanf("%80s\t%80s", username, password);
	if (num_of_users >= MAX_USERS) {
		printf("Error: Not enough space for new users\n");
		return;
	}
	if (!is_valid_username(username)) {
		printf("Error: Illegal username\n");
		return;
	}
	if (!is_valid_password(password)) {
		printf("Error: Illegal password\n");
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
	char username[81];
	char password[81];
	int i;
	unsigned char digest[DIGEST_SIZE];

	scanf("%80s\t%80s", username, password);
	if (!is_valid_username(username)) {
		printf("Error: Illegal username\n");
		return;
	}
	if (!is_valid_password(password)) {
		printf("Error: Illegal password\n");
		return;
	}
	i = find_user(username);
	if (i < 0) {
		printf("Error: Nonexistent username\n");
		return;
	}

	calc_md5(digest, password, strlen(password));

	if (memcmp(digest, users_array[i].digested_password, 16) == 0) {
		printf("approved.\n");
	}
	else {
		printf("denied.\n");
	}
}


int main (int argc, char *argv[])
{
	char cmd[81];

	/* make sure the entire array is initlaized to zeros */
	memset(users_array, 0, sizeof(users_array));

	/* main loop */
	while (1) {
		printf(">> ");
		scanf("%80s", cmd);

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
