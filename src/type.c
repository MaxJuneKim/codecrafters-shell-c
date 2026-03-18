#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "global_vars.h"
#include "locate_bin.h"

#if defined(__WIN32) || defined(__WIN64)
  #include <windows.h>
#endif

void executeType(const char* argCommand) {
  for (int i = 0; i < total_commands; i++) { // For each builtin shell,
    // compare argument command to the current builtin shell. If match, print "shell builtin" and return
    if (strcmp(argCommand, commands[i]) == 0) {
      printf("%s is a shell builtin\n", argCommand);
      return;
    }
  }

  // if not found, go through every path in PATH
  char* exec_loc = locate_bin(argCommand);
  if (exec_loc) {
    printf("%s is %s\n", argCommand, exec_loc);
    free(exec_loc);
  } else { // if not, print "not found"
    printf("%s: not found\n", argCommand);
  }
}
