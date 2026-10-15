#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include <global_vars.h>
#include "locate_files.h"

size_t get_files_in_dir(char** dest, const char* const dir_path) {
  DIR *dir = opendir(dir_path ? dir_path : ".");
  if (!dir) return -1;

  size_t files_count = 0;
  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
    dest[files_count++] = strdup(entry->d_name);
  }

  dest[files_count] = NULL;
  closedir(dir);
  return files_count;
}