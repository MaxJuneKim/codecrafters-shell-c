#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "Navigation/cd.h"

void cd(const char* directory) {
  if (strcmp(directory, "~") == 0) {
    char* home_directory = getenv("HOME");
    chdir(home_directory);
    return;
  }

  int result = chdir(directory);
  if (result == -1) {
    printf("%s: No such file or directory\n", directory);
  }
}