//
// Created by rodrijjke on 4/13/21.
//

#ifndef MSH_DYN_STRING_H
#define MSH_DYN_STRING_H
void append_char(char c, int idx, char **buff_p, int *buff_size_p);
char **split(char *src, char delimiter, int *res_len_p);
#endif //MSH_DYN_STRING_H
