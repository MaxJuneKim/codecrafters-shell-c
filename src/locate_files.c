#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include <global_vars.h>
#include "locate_files.h"

void load_files_cur_dir() {
  DIR *dir = opendir(".");
  if (!dir) return;

  char** tmp_buf = (char**)malloc(sizeof(char*) * 128);
  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
    tmp_buf[FILES_COUNT++] = strdup(entry->d_name);
  }

  FILES_IN_CUR_DIR = (char**)malloc(sizeof(char*) * (FILES_COUNT + 1));
  for (size_t i = 0; i < FILES_COUNT; i++) 
    FILES_IN_CUR_DIR[i] = tmp_buf[i];

  FILES_IN_CUR_DIR[FILES_COUNT] = NULL;
  closedir(dir);
  free(tmp_buf);
}