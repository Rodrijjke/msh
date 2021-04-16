//
// Created by rodrijjke on 4/14/21.
//

#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <assert.h>
#include <stdio.h>
#include "parse/cmd_parse.h"
#include "prog_exec.h"

static int open_in_redirect_file(struct cmd *cmd)
{
	assert(cmd->redirect_file != NULL);
	return open(cmd->redirect_file, O_RDONLY);
}

static int open_out_redirect_file(struct cmd *cmd)
{
	assert(cmd->redirect_file != NULL);
	assert(cmd->redirect_flags & REDIR_OUT);
	int o_flag = 0, mode_flag = 0;
	o_flag |= O_WRONLY | O_CREAT;
	mode_flag = S_IRUSR | S_IWUSR;
	if (cmd->redirect_flags & REDIR_APPEND)
		o_flag |= O_APPEND;
	else
		o_flag |= O_TRUNC;
	return open(cmd->redirect_file, o_flag, mode_flag);
}

static int exec_in_pipe_last(struct cmd cmd, int read_from) {
	int write_to = STDOUT_FILENO;
	if (cmd.redirect_file != NULL && (cmd.redirect_flags & REDIR_OUT)) {
		write_to = open_out_redirect_file(&cmd);
	}
	int cpid = fork();
	assert(cpid != -1);
	if (cpid == 0) {
		// Child code
		if (read_from != STDIN_FILENO) {
			dup2(read_from, STDIN_FILENO);
			close(read_from);
		}
		if (write_to != STDOUT_FILENO) {
			dup2(write_to, STDOUT_FILENO);
			close(write_to);
		}
		return execv(cmd.name, cmd.args);
	}
	// Parent code
	if (read_from != STDIN_FILENO)
		close(read_from);
	if (write_to != STDOUT_FILENO)
		close(write_to);
	if (waitpid(cpid, NULL, 0) < 0)
		return -1;

	return 0;
}

static int exec_in_pipe(struct cmd cmd, int read_from_prev)
{
	int this_to_next[2];
	if (cmd.redirect_file != NULL && (cmd.redirect_flags & REDIR_OUT)) {
		this_to_next[1] = open_out_redirect_file(&cmd);
		this_to_next[0] = open_in_redirect_file(&cmd);
	}
	else {
		pipe(this_to_next);
	}
	int read_from_this = this_to_next[0];
	int write_to_next = this_to_next[1];

	int cpid = fork();
	assert(cpid != -1);
	if (cpid == 0) {
		// Child code
		close(read_from_this);
		if (read_from_prev != STDIN_FILENO) {
			dup2(read_from_prev, STDIN_FILENO);
			close(read_from_prev);
		}
		dup2(write_to_next, STDOUT_FILENO);
		close(write_to_next);
		return execv(cmd.name, cmd.args);
	}
	// Parent code
	if (read_from_prev != STDIN_FILENO)
		close(read_from_prev);
	close(write_to_next);
	if (waitpid(cpid, NULL, 0) < 0)
		return -1;

	return read_from_this;
}

int exec_pipe(struct cmd *cmd_list, int cmd_count)
{
	int read_from_prev = STDIN_FILENO;
	for (int i = 0; i < cmd_count; i++) {
		if (i == 0 && cmd_list[0].redirect_file != 0 && (cmd_list[0].redirect_flags & REDIR_IN)) { // first cmd
			read_from_prev = open_in_redirect_file(&cmd_list[0]);
		}
		if (i == cmd_count - 1) { // last cmd
			if (exec_in_pipe_last(cmd_list[i], read_from_prev) < 0)
				return -1;
		}
		else {
			if ((read_from_prev = exec_in_pipe(cmd_list[i], read_from_prev)) < 0)
				return -1;
		}
	}
	return 0;
}