#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "execute_bin.h"
#include "locate_bin.h"

#define MAX_ARG_LEN 256;
#define MAX_NUM_ARGS 1024;

// Returns newly malloc'd array of strings. Each string in the array contains each argument of the entire command
static char** parse_args(const char* bin_name, const char* raw_args) {
  char** args = (char**)malloc(sizeof(char**) * 1024);
  args[0] = (char*)malloc(sizeof(char) * 256);
  strcpy(args[0], bin_name);

  int arg_count = 1;
  char* raw_args_dup = strdup(raw_args);
  char* cur_arg = strtok(raw_args_dup, " ");

  while (cur_arg) {
    if (*cur_arg == '\0') { continue; } // skipping empty string
    args[arg_count++] = cur_arg;
    cur_arg = strtok(NULL, " ");
  }
  args[arg_count] = NULL;

  return args;
}

static char** parse_args_opt(const char* raw_args) {
  char** args = (char**)malloc(sizeof(char**) * 1024);
  const char* cursor = raw_args;
  int arg_count = 0;
  int cur_arg_index = 0;

  while (*cursor != '\0') {
    if (*cursor == ' ') {
      args[++arg_count] = (char*)malloc(sizeof(char) * 256);
      cur_arg_index = 0;
      cursor++;
      continue;
    }
    args[arg_count][cur_arg_index++] = *cursor;
    cursor++;
  }

  return args;
}

void execute_bin(const char* bin_name, const char* raw_args) {
  char** args = parse_args(bin_name, raw_args);

  char* full_path = locate_bin(bin_name);
  if (full_path) {
    pid_t process = fork();
    if (process == 0) { // child process
      execv(full_path, args);
      perror("execvp failed");
      exit(1);
    } else { // parent process
      int status;
      free(full_path);
      // wait(NULL);
      waitpid(process, &status, 0);
      if (!WIFEXITED(status)) {
        printf("Parent: Child exited abnormally with status code %d\n", WEXITSTATUS(status));
      }
    }
  } else {
    printf("%s: command not found\n", bin_name);
  }
}