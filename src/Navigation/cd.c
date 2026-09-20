#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "Navigation/cd.h"

struct Output cd(const char* directory) {
  struct Output output = init_output();
  if (directory == NULL) return output; // no parameter

  int result = chdir(directory);
  int len = strlen(directory);
  
  if (result == -1) {
    output.error = (char*)malloc(sizeof(char) * len + 29);
    snprintf(output.error, len + 29, "%s: No such file or directory\n", directory);
  } else {
    output.error = NULL;
  }
  return output;
}