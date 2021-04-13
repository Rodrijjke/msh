//
// Created by rodrijjke on 4/13/21.
//

#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "file_system.h"

char *combine_paths(char *start, char *end)
{
	size_t start_len = strlen(start);
	size_t end_len = strlen(end);
	char *combined = (char *) realloc(start, sizeof(char) * start_len + end_len);
	if (start[start_len-1] == '/') {
		strcpy(combined + start_len, end);
	}
	else {
		combined[start_len] = '/';
		strcpy(combined + start_len + 1, end);
	}
	return combined;
}

int is_dir_has_file(char *dir_path, const char *file_name)
{
	assert(file_name != NULL);
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(dir_path)) != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			if (strcmp(ent->d_name, file_name) == 0) {
				return 1;
			}
		}
		closedir(dir);
	} else {
		fprintf(stderr, "Failed to open dir '%s'\n", dir_path);
		return 0;
	}
	return 0;
}