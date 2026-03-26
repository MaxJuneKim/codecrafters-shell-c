#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "Navigation/pwd.h"

void pwd() {
  char cwd[1024];
  if (getcwd(cwd, sizeof(cwd)) != NULL) {
    printf("%s\n", cwd);
  }
}