#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "echo.h"
#include "global_vars.h"
#include "type.h"
#include "execute_bin.h"
#include "parse_arg.h"
#include "Navigation/pwd.h"
#include "Navigation/cd.h"
#include "types.h"

// TODO: 
// I'm facing plenty of cases where output of my local run and codecrafter testing are different.
// I wonder if this is an OS issue because I am using Mac OS. Build this project in WSL as well and Windows if possible
// and see if I can similar results 

void write_to_files(FILE** file_ptrs, const char* output) {
  while (*file_ptrs) {
    if (output) fputs(output, *file_ptrs);
    if (*file_ptrs == stdout || *file_ptrs == stderr) {
      file_ptrs++;
      continue;
    }
    fclose(*file_ptrs++);
  }
}

void executeCommand(char* input) {
  if (*input == '\0') { // empty command
    return;
  }

  // extract command
  char command[1024];
  struct Argument args = parse_args(input);
  // char* output = NULL;
  struct Output output;
  if (!args.arguments[0]) { // signal for parsing failed
    printf("Failed to parse command: %s\n", input);
    return;
  }

  if (strcmp(args.arguments[0], "echo") == 0) {
    output = echo((const char**)args.arguments);
  } else if (strcmp(args.arguments[0], "type") == 0) {
    output = executeType(args.arguments[1]);
  } else if (strcmp(args.arguments[0], "pwd") == 0) {
    output = pwd();
  } else if (strcmp(args.arguments[0], "cd") == 0) {
    output = cd(args.arguments[1]);
  } else {
    output = execute_bin(args);
  }

  // TODO: This code is way too repetitive. Refactor it. One idea is in parse_args, return file_pointers to the redirectors instead of opening them here?
  // outputting to stdout
  write_to_files(args.output_terminals, output.output);
  write_to_files(args.error_terminals, output.error);
  
  // deallocating
  if (output.output) free(output.output);
  if (output.error) free(output.error);
  free_arg(args);
}

int main(int argc, char *argv[]) {
  // Flush after every printf
  setbuf(stdout, NULL);

  char input[1024];

  while (true) {
    printf("$ ");
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = '\0';

    if (strcmp(input, "exit") == 0) {
      break;
    }
    
    executeCommand(input);
  }

  return 0;
}
