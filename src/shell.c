//
// Created by rodrijjke on 4/14/21.
//

#include <stdio.h>
#include <string.h>
#include "exec/prog_exec.h"
#include "paths/path_find.h"
#include "parse/cmd_parse.h"
#include "shell.h"

void start_shell_cycle()
{
	while (1)
	{
		printf("> ");
		struct input *input_p = parse_input();
		if (input_p->error_msg != NULL) {
			printf("%s\n", input_p->error_msg);
			continue;
		}
		if (input_p->cmd_count == 0) {
			continue;
		}
		struct cmd curr_cmd = input_p->cmd_list[0];
		if (strcmp(curr_cmd.name, "exit") == 0) {
			break;
		}
		int errno = exec_pipe(input_p->cmd_list, input_p->cmd_count);
		if (errno) {
			printf("Error %d\n", errno);
		}
	}
}