#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "Navigation/cd.h"

char* cd(const char* directory) {
  char* output;
  int result = chdir(directory);
  if (result == -1) {
    output = (char*)malloc(sizeof(char) * strlen(directory) + 29);
    snprintf(output, strlen(directory) + 29, "%s: No such file or directory\n", directory);
    return output;
  } 
  output = (char*)malloc(sizeof(char));
  *output = '\0';
  return output;
}