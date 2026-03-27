#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "Navigation/cd.h"

void cd(const char* directory) {
  int result = chdir(directory);
  if (result == -1) {
    printf("%s: No such file or directory\n", directory);
  }
}