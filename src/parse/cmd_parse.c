//
// Created by rodrijjke on 4/9/21.
//

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "cmd_parse.h"

#define is_space(c) (((c) == ' ' || (c) == '\t'))
#define is_end(c) (((c) == '\n' || (c) == EOF))
#define is_word_char(c) ((!is_space(c) && (c) != '\n' && (c) != '<' && (c) != '>' && (c) != '|' && (c) != '&' && (c) != '"'))
#define is_redirect_op_char(c) (((c) == '>' || (c) == '<'))

static struct input curr_input;
static struct cmd curr_cmd;
static int curr_cmd_idx;

static void skip_spaces()
{
	char c;
	do {
		c = (char) getchar();
		if (is_end(c))
			break;
	}
	while (is_space(c));
	ungetc(c, stdin);
}

static void append_char(char c, int idx, char **buff_p, int *buff_size_p)
{
	if (*buff_size_p <= idx) {
		*buff_size_p = *buff_size_p * 2 + 1;
		*buff_p = (char *) realloc(*buff_p, *buff_size_p * sizeof(char));
	}
	(*buff_p)[idx] = c;
}

static int read_quoted_str(char **buff_p)
{
	char c;
	int len = 0, buff_size = 0;
	*buff_p = NULL;
	assert(getchar() == '"');
	int is_quote_escaped = 0;
	while (1) {
		c = (char) getchar();
		if (is_end(c)) {
			curr_input.error_msg = "No closing quote";
			return len;
		}
		if (c == '"') {
			if (!is_quote_escaped) {
				append_char('\0', len, buff_p, &buff_size);
				return len;
			}
			len--;
		}
		append_char(c, len, buff_p, &buff_size);
		len++;
		is_quote_escaped = (c == '\\') ? 1 : 0;
	}
}

static int read_word(char **buff_p)
{
	char c;
	int len = 0, buff_size = 0;
	*buff_p = NULL;
	while (1) {
		c = (char) getchar();
		if (!is_word_char(c)) {
			append_char('\0', len, buff_p, &buff_size);
			ungetc(c, stdin);
			return len;
		}
		append_char(c, len, buff_p, &buff_size);
		len++;
	}
}

static int read_liter(char **buff_p)
{
	char c = (char) getchar();
	ungetc(c, stdin);
	if (is_end(c) || c == '|')
		return 0;
	int is_quoted = c == '\"';
	return is_quoted
		? read_quoted_str(buff_p)
		: read_word(buff_p);
}

static void read_args()
{
	curr_cmd.args_count = 0;
	curr_cmd.args = NULL;
	int liter_len, args_cap = 0;
	char *buff;
	while (1) {
		liter_len = read_liter(&buff);
		if (liter_len == 0)
			return;
		if (curr_cmd.args_count >= args_cap) {
			args_cap = args_cap * 2 + 1;
			curr_cmd.args = (char **) realloc(curr_cmd.args, args_cap * sizeof(char *));
		}
		curr_cmd.args[curr_cmd.args_count] = (char *) malloc(liter_len * sizeof(char));
		strcpy(curr_cmd.args[curr_cmd.args_count], buff);
		free(buff);
		curr_cmd.args_count++;
		skip_spaces();
	}
}

static enum redirect_type read_redirect_op()
{
	char c = (char) getchar();
	if (c == '>') {
		c = (char) getchar();
		if (c == '>') {
			return REDIRECT_OUT_APPEND;
		}
		else {
			ungetc(c, stdin);
			return REDIRECT_OUT;
		}
	}
	else {
		return REDIRECT_IN;
	}
}

static void read_redirect()
{
	char c = (char) getchar();
	ungetc(c, stdin);
	if (!is_redirect_op_char(c)) {
		return;
	}
	curr_cmd.redirect_type = read_redirect_op();
	skip_spaces();
	char *file;
	int file_len = read_liter(&file);
	if (file_len == 0) {
		curr_input.error_msg = "No file name to redirect I/O";
		return;
	}
	curr_cmd.redirect_file = file;
}

static void read_async_op()
{
	char c = (char) getchar();
	if (c == '&') {
		curr_cmd.is_async = 1;
		return;
	}
	ungetc(c, stdin);
	curr_cmd.is_async = 0;
}

static void save_curr_cmd()
{
	++curr_input.cmd_count;
	size_t new_size = curr_input.cmd_count * sizeof(struct cmd);
	curr_input.cmd_list = (struct cmd *) realloc(curr_input.cmd_list, new_size);
	curr_input.cmd_list[curr_cmd_idx] = curr_cmd;
	++curr_cmd_idx;
}

static void read_cmd()
{
	if (read_liter(&curr_cmd.name) == 0) {
		return;
	}
	skip_spaces();
	read_args();
	skip_spaces();
	read_redirect();
	skip_spaces();
	read_async_op();
	save_curr_cmd();
}

static void init_global()
{
	struct input new_input = {};
	curr_input = new_input;
	curr_cmd_idx = 0;
}

static void init_curr_cmd()
{
	struct cmd new_curr_cmd = {};
	curr_cmd = new_curr_cmd;
}

struct input *parse_input()
{
	init_global();
	char c;
	while (1) {
		skip_spaces();
		init_curr_cmd();
		read_cmd();
		skip_spaces();
		c = (char) getchar();
		if (is_end(c))
			return &curr_input;
		if (c != '|') {
			curr_input.error_msg = "Incorrect commands separator";
			return &curr_input;
		}
	}
}
