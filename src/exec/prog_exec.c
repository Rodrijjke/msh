//
// Created by rodrijjke on 4/14/21.
//

#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <assert.h>
#include "parse/cmd_parse.h"
#include "prog_exec.h"

int exec(char *prog_path, char **args, char *redirect_file, int redirect_flags)
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