//
// Created by rodrijjke on 4/9/21.
//

#include <stdio.h>
#include "paths/path_find.h"
#include "parse/cmd_parse.h"

int main(int argc, char **argv)
{
	printf("Welcome to msh!\n");
	while (1)
	{
		printf("> ");
		struct input *input_p = parse_input();
		if (input_p->error_msg != NULL) {
			printf("%s\n", input_p->error_msg);
			continue;
		}
		int cmd_count = input_p->cmd_count;
		if (cmd_count == 0) {
			printf("No commands to execute\n");
			continue;
		}
		for (int i = 0; i < cmd_count; ++i) {
			char *cmd_name = input_p->cmd_list[i].name;
			char *prog_path = find_prog_path(cmd_name);
			if (prog_path == NULL) {
				printf("Not found program %s\n", cmd_name);
			}
			else {
				printf("Found program %s\n", prog_path);
			}
		}
	}
}