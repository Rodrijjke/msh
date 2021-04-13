//
// Created by rodrijjke on 4/13/21.
//

#include <stdlib.h>
#include <string.h>
#include "dyn_string.h"

void append_char(char c, int idx, char **buff_p, int *buff_size_p)
{
	if (*buff_size_p <= idx) {
		*buff_size_p = *buff_size_p * 2 + 1;
		*buff_p = (char *) realloc(*buff_p, *buff_size_p * sizeof(char));
	}
	(*buff_p)[idx] = c;
}

char **split(char *src, char delimiter, int *res_len_p)
{
	int start = 0, res_len = 0, res_cap = 0, str_len;
	char **res;
	for (int i = 0; ; ++i) {
		if (src[i] != delimiter && src[i] != '\0')
			continue;
		if (res_len >= res_cap) {
			res_cap = res_cap * 2 + 1;
			res = (char **) realloc(res, sizeof(char *) * res_cap);
		}
		str_len = i - start + 1;
		res[res_len] = (char *) malloc(sizeof(char) * str_len);
		strncpy(res[res_len], src + start, str_len - 1);
		res[res_len][str_len - 1] = '\0';
		start = i + 1;
		res_len++;
		if (src[i] == '\0')
			break;
	}
	*res_len_p = res_len;
	return res;
}
