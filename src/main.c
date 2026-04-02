#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "echo.h"
#include "global_vars.h"
#include "type.h"
#include "execute_bin.h"
#include "parse_arg.h"
#include "Navigation/pwd.h"
#include "Navigation/cd.h"

void executeCommand(char* input) {
  if (*input == '\0') { // empty command
    return;
  }

  // extract command
  char command[1024];
  struct Argument* args = parse_args(input);
  char* output;
  if (!args) { // parsing failed
    printf("Failed to parse command: %s\n", input);
    return;
  }
  strcpy(command, args->arguments[0]);

  if (strcmp(command, "echo") == 0) {
    output = echo((const char**)args->arguments);
  } else if (strcmp(command, "type") == 0) {
    output = executeType(args->arguments[1]);
  } else if (strcmp(command, "pwd") == 0) {
    output = pwd();
  } else if (strcmp(command, "cd") == 0) {
    output = cd(args->arguments[1]);
  } else {
    output = execute_bin((const char* const*)args->arguments);
  }

  if (args->output_terminals[0] == NULL) {
    printf("%s", output);
  } else {
    char** output_terminals = args->output_terminals;
    while (*output_terminals != NULL) {
      FILE* output_file = fopen(*output_terminals++, "w");
      if (!output_file) {
        printf("Couldn't open %s. Skipping this file...\n", *output_terminals);
        continue;
      }
      fprintf(output_file, "%s", output);
      fclose(output_file);
    }
  }

  // deallocating
  free(args->arguments);
  free(args->output_terminals);
  free(args);
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
