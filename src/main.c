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

// TODO: use this struct and enum to distinguish between stdout and stderr for builtin commands
// enum Output_Type {
//   STDOUT,
//   STDERR
// };

// struct Output {
//   char* output_string;
//   enum Output_Type type;
// };

void executeCommand(char* input) {
  if (*input == '\0') { // empty command
    return;
  }

  // extract command
  char command[1024];
  struct Argument* args = parse_args(input);
  char* output = NULL;
  if (!args) { // parsing failed
    printf("Failed to parse command: %s\n", input);
    return;
  }
  strcpy(command, args->arguments[0]);

  char** err_redirect = args->error_redirect;
  while (*err_redirect) {
    int output_file = open(*err_redirect++, O_CREAT | O_TRUNC, 0644);
    close(output_file);
  }

  if (strcmp(command, "echo") == 0) {
    output = echo((const char**)args->arguments);
  } else if (strcmp(command, "type") == 0) {
    output = executeType(args->arguments[1]);
  } else if (strcmp(command, "pwd") == 0) {
    output = pwd();
  } else if (strcmp(command, "cd") == 0) {
    output = cd(args->arguments[1]);
  } else {
    execute_bin(args);
  }

  if (output && args->output_terminals[0] == NULL) {
    printf("%s", output);
  } else if (output) {
    char** output_terminals = args->output_terminals;
    while (*output_terminals != NULL) {
      FILE* output_file = fopen(*output_terminals, "w");
      if (!output_file) {
        printf("Couldn't open %s. Skipping this file...\n", *output_terminals);
        continue;
      }
      output_terminals++;
      fputs(output, output_file);
      fclose(output_file);
    }
  }

  // deallocating
  free(args->arguments);
  free(args->output_terminals);
  free(args->error_redirect);
  free(args);
  free(output);
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
