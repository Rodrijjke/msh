//
// Created by rodrijjke on 4/13/21.
//

#include <stdlib.h>
#include "path_find.h"
#include "utils/dyn_string.h"
#include "utils/file_system.h"

char *find_prog_path(char *prog_name)
{
	if (prog_name == NULL)
		return NULL;
	char *path_env = getenv("PATH");
	int dir_count = 0;
	char **dir_paths = split(path_env, ':', &dir_count);
	for (int i = 0; i < dir_count; ++i) {
		if (is_dir_has_file(dir_paths[i], prog_name)) {
			return combine_paths(dir_paths[i], prog_name);
		}
	}
	return NULL;
}