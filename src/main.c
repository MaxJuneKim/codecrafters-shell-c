#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define total_commands 3

const char* const commands[total_commands] = {"echo", "type", "exit"};

void executeType(char* argCommand) {
  for (int i = 0; i < total_commands; i++) { // For each builtin shell,
    
    // compare argument command to the current builtin shell
    int k = 0;
    while (argCommand[k] != '\0' && argCommand[k] != ' ' && commands[i][k] != '\0') {
      if (argCommand[k] != commands[i][k]) {
        break;
      }
      k++;
    }

    // if match, print "shell builtin" and return
    if ((argCommand[k] == '\0' || argCommand[k] == ' ') && commands[i][k] == '\0') { 
      char temp = argCommand[k];
      argCommand[k] = '\0'; // dangerous?
      printf("%s is a shell builtin\n", argCommand);
      argCommand[k] = temp;
      return;
    } 
  }

  // if not found, print "not found"
  printf("%s: not found\n", argCommand);
}

void executeCommand(char* input) {
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
  } else {
    printf("%s: command not found\n", command);
  }
}

int main(int argc, char *argv[]) {
  // Flush after every printf
  setbuf(stdout, NULL);

  char input[1024];

  // TODO: Uncomment the code below to pass the first stage
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

// char* extractCommand() {

// }
