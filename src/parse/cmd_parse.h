//
// Created by rodrijjke on 4/9/21.
//

#ifndef MSH_CMD_PARSE_H
#define MSH_CMD_PARSE_H
enum redirect_flag {
	REDIR_IN = 1,
	REDIR_OUT = 2,
	REDIR_APPEND = 4
};

struct cmd {
	char *name;
	int args_count;
	char **args;
	char *redirect_file;
	int redirect_flags;
	int is_async;
};

struct input {
	struct cmd *cmd_list;
	int cmd_count;
	const char *error_msg;
};

struct input *parse_input();
#endif //MSH_CMD_PARSE_H
