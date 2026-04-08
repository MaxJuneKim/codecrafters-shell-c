#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "global_vars.h"
#include "locate_bin.h"

#if defined(__WIN32) || defined(__WIN64)
  #include <windows.h>
#endif

char* executeType(const char* argCommand) {
  char* output = (char*)malloc(sizeof(char) * 256);
  for (int i = 0; i < total_commands; i++) { // For each builtin shell,
    // compare argument command to the current builtin shell. If match, print "shell builtin" and return
    if (strcmp(argCommand, commands[i]) == 0) {
      snprintf(output, strlen(argCommand) + 21, "%s is a shell builtin\n", argCommand);
      return output;
    }
  }

  // if not found, go through every path in PATH
  char* exec_loc = locate_bin(argCommand);
  if (exec_loc) {
    // TODO: add safety measure to cover the case where there might not be enough room in output string 
    int success = snprintf(output, strlen(argCommand) + strlen(exec_loc) + 6, "%s is %s\n", argCommand, exec_loc);
    free(exec_loc);
  } else { // if not, print "not found"
    snprintf(output, strlen(argCommand) + 13, "%s: not found\n", argCommand);
  }
  return output;
}
