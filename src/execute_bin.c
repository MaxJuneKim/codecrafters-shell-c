#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>

#include "execute_bin.h"
#include "locate_bin.h"
#include "parse_arg.h"

void execute_bin(const char* const* args) {
  char* full_path = locate_bin(args[0]);
  if (full_path) {
    pid_t process = fork();
    if (process == 0) { // child process
      execv(full_path, (char* const*)args);
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
    printf("%s: command not found\n", args[0]);
  }
}