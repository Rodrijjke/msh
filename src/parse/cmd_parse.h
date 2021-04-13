//
// Created by rodrijjke on 4/9/21.
//

#ifndef MSH_CMD_PARSE_H
#define MSH_CMD_PARSE_H
enum redirect_type {
	REDIRECT_IN = 10,
	REDIRECT_OUT = 20,
	REDIRECT_OUT_APPEND = 30
};

struct cmd {
	char *name;
	int args_count;
	char **args;
	char *redirect_file;
	enum redirect_type redirect_type;
	int is_async;
};

struct input {
	struct cmd *cmd_list;
	int cmd_count;
	const char *error_msg;
};

struct input *parse_input();
#endif //MSH_CMD_PARSE_H
