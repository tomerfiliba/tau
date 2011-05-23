#include <stdio.h>
#include "rules.h"


int main(int argc, const char** argv)
{
	unsigned long limit;
	rule_info_t rule;
	char password[100];
	int res;


	if (rule_load_from_file(&rule, "SHA_SimpleRun.ini") != 0) {
		return 1;
	}
	limit = rule_limit(&rule);
	printf("limit = %lu\n", limit);

	while (1) {
		res = rule_generate_password(&rule, password);
		if (res == -2) {
			break;
		} else if (res != 0) {
			printf("\nERROR\n");
			break;
		}
		printf("'%s', ", password);
	}
	printf("\n");

	return 0;
}
