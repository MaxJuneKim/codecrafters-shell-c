#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#if defined(__WIN32) || defined(__WIN64)
  #include <windows.h>
#endif

#define total_commands 3

const char* const commands[total_commands] = {"echo", "type", "exit"};

void executeType(char* argCommand) {
  for (int i = 0; i < total_commands; i++) { // For each builtin shell,
    // compare argument command to the current builtin shell
    int k = 0;
    while (argCommand[k] != '\0' && commands[i][k] != '\0') {
      if (argCommand[k] != commands[i][k]) {
        break;
      }
      k++;
    }

    // if match, print "shell builtin" and return
    if (argCommand[k] == '\0' && commands[i][k] == '\0') { 
      printf("%s is a shell builtin\n", argCommand);
      return;
    } 
  }

  // if not found, go through every path in PATH
  // getting PATH variable
  #if defined(__WIN32) || defined(__WINT64)
    char* path_value = GetEnvironmentVariable("PATH");
  #elif defined(__linux__) || defined(__unix__) || defined(TARGET_OS_MAC)
    char* path_value = getenv("PATH");
  #endif

  char all_paths[strlen(path_value) + 1];
  strcpy(all_paths, path_value);

  // testing each path
  #if defined(__WIN32)
    char delimiter[2] = ";"
  #elif defined(__linux__) || defined(__unix__) || defined(TARGET_OS_MAC)
    char delimiter[2] = ":";
  #endif

  char* cur_path = strtok(all_paths, delimiter);
  char cur_full_path[1024];
  while (cur_path != NULL) {
    // check if the file exists and permission is executable
    strcpy(cur_full_path, cur_path);
    strcat(cur_full_path, "/");
    strcat(cur_full_path, argCommand);
    if (access(cur_full_path, X_OK) == 0) { 
      printf("%s is %s\n", argCommand, cur_full_path);
      // printf("full path is: %s\n", cur_full_path);
      return;
    } 
    // printf("full path is: %s\n", cur_full_path);
    cur_path = strtok(NULL, delimiter);
  }

  // if not, print "not found"
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
