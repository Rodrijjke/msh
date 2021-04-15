//
// Created by rodrijjke on 4/14/21.
//

#ifndef MSH_PROG_EXEC_H
#define MSH_PROG_EXEC_H
int exec_pipe(struct cmd *cmd_list, int cmd_count);
int exec(char *prog_path, char **args, char *redirect_file, int redirect_flags);
#endif //MSH_PROG_EXEC_H
