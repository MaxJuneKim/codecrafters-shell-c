#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "Navigation/pwd.h"

char* pwd() {
  char* cwd = (char*)malloc(sizeof(char) * 256);
  int len = 0;
  if (getcwd(cwd, 256) != NULL) {
    len = strlen(cwd);
  }
  cwd[len] = '\n';
  cwd[len + 1] = '\0';
  return cwd;
}