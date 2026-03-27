#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "global_vars.h"
#include "type.h"
#include "execute_bin.h"
#include "Navigation/pwd.h"
#include "Navigation/cd.h"

void executeCommand(char* input) {
  if (*input == '\0') { // empty command
    return;
  }

  // extract command
  char command[1024];
  int i = 0;
  while (*(input + i) != '\0' && *(input + i) != ' ') {
    command[i] = *(input + i);
    i++;
  }
  command[i] = '\0';

  if (strcmp(command, "echo") == 0) {
    printf("%s\n", input + i + 1);
  } else if (strcmp(command, "type") == 0) {
    executeType(input + i + 1);
  } else if (strcmp(command, "pwd") == 0) {
    pwd();
  } else if (strcmp(command, "cd") == 0) {
    cd(input + i + 1);
  } else {
    execute_bin(command, input + i + 1);
  }
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
