#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "Navigation/pwd.h"

struct Output pwd() {
  struct Output output;
  output.output = (char*)malloc(sizeof(char) * 256);
  
  if (getcwd(output.output, 256) != NULL) {
    int len = strlen(output.output);
    output.output[len] = '\n';
    output.output[len + 1] = '\0';
    output.error = NULL;
  } else {
    free(output.output);
    output.output = NULL;
    output.error = (char*)malloc(sizeof(char) * 19);
    snprintf(output.error, 19, "Failed to get pwd\n");
  }
  
  return output;
}