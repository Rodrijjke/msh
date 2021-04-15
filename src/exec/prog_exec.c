//
// Created by rodrijjke on 4/14/21.
//

#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <assert.h>
#include <stdio.h>
#include "paths/path_find.h"
#include "parse/cmd_parse.h"
#include "prog_exec.h"

static int exec_in_pipe_last(struct cmd cmd, int read_from, int write_to) {
	char *prog_path = find_prog_path(cmd.name);
	if (prog_path == NULL) {
		fprintf(stderr, "Command not found: %s\n", cmd.name);
		if (read_from != STDIN_FILENO)
			close(read_from);
		if (write_to != STDOUT_FILENO)
			close(write_to);
		return -1;
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
		return execv(prog_path, cmd.args);
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
	pipe(this_to_next);
	int read_from_this = this_to_next[0];
	int write_to_next = this_to_next[1];

	char *prog_path = find_prog_path(cmd.name);
	if (prog_path == NULL) {
		fprintf(stderr, "Command not found: %s\n", cmd.name);
		close(read_from_this);
		close(write_to_next);
		if (read_from_prev != STDIN_FILENO)
			close(read_from_prev);
		return -1;
	}
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
		return execv(prog_path, cmd.args);
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
		if (i == cmd_count - 1) { // last cmd
			if (exec_in_pipe_last(cmd_list[i], read_from_prev, STDOUT_FILENO) < 0)
				return -1;
		}
		else {
			if ((read_from_prev = exec_in_pipe(cmd_list[i], read_from_prev)) < 0)
				return -1;
		}
	}
	return 0;
}

static int exec_pipe_example(struct cmd *cmd_list, int cmd_count)
{
	int cpid;

	int pipe_fd_1[2];
	pipe(pipe_fd_1);
	if ((cpid = fork()) == 0) {
		close(pipe_fd_1[0]); // close read 1
		dup2(pipe_fd_1[1], STDOUT_FILENO); // write 1 <-> stdout
		close(pipe_fd_1[1]); // close write 1
		execl("/usr/bin/echo", "echo", "print('result is ', 123 + 456)", NULL);
	}
	close(pipe_fd_1[1]); // close write 1
	waitpid(cpid, NULL, 0);

	int pipe_fd_2[2];
	pipe(pipe_fd_2);
	if ((cpid = fork()) == 0) {
		close(pipe_fd_2[0]); // close read 2
		dup2(pipe_fd_1[0], STDIN_FILENO); // read 1 <-> stdin
		dup2(pipe_fd_2[1], STDOUT_FILENO); // write 2 <-> stdout
		close(pipe_fd_1[0]); // close read 1
		close(pipe_fd_2[1]); // close write 2
		execl("/usr/bin/python3", "python3", "-i", NULL);
	}
	close(pipe_fd_1[0]); // close read 1
	close(pipe_fd_2[1]); // close write 2
	waitpid(cpid, NULL, 0);

	if ((cpid = fork()) == 0) {
		dup2(pipe_fd_2[0], STDIN_FILENO); // read 2 <-> stdin
		close(pipe_fd_2[0]); // close read 2
		execl("/usr/bin/grep", "grep", "result", NULL);
	}
	close(pipe_fd_2[0]); // close read 2
	waitpid(cpid, NULL, 0);

	return 0;
}

int exec_with_redirect(char *prog_path, char **args, char *redirect_file, int redirect_flags)
{
	if (fork() == 0) {
		// Child process code
		if (redirect_file != NULL) {
			int o_flag = 0, mode_flag = 0;
			int redirect_fd = -1, std_io_fileno = -1;
			if (redirect_flags & REDIR_IN) {
				o_flag |= O_RDONLY;
				std_io_fileno = STDIN_FILENO;
			}
			if (redirect_flags & REDIR_OUT) {
				o_flag |= O_WRONLY | O_CREAT;
				std_io_fileno = STDOUT_FILENO;
				mode_flag = S_IRUSR | S_IWUSR;
				if (redirect_flags & REDIR_APPEND)
					o_flag |= O_APPEND;
				else
					o_flag |= O_TRUNC;
			}
			assert(std_io_fileno != -1);
			redirect_fd = open(redirect_file, o_flag, mode_flag);
			dup2(redirect_fd, std_io_fileno);
		}
		return execv(prog_path, args);
	}
	// Parent process code
	wait(NULL);
	return 0;
}