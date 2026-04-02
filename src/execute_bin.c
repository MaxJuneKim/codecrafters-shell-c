#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>

#include "execute_bin.h"
#include "locate_bin.h"
#include "parse_arg.h"

char* execute_bin(const char* const* args) {
  char* full_path = locate_bin(args[0]);
  int pipe_fd[2];
  char* output;

  if (full_path) {
    pipe(pipe_fd);
    pid_t process = fork();
    if (process == 0) { // child process
      close(pipe_fd[0]);
      // approach 2 idea: use write(int fildes, const void* buf, size_t nbytes)?
      if (dup2(pipe_fd[1], STDOUT_FILENO) == -1) {
        perror("dup2");
        exit(EXIT_FAILURE);
      }
      close(pipe_fd[1]);
      execv(full_path, (char* const*)args);
      perror("execvp failed");
      exit(1);
    } else { // parent process
      close(pipe_fd[1]);
      output = (char*)malloc(sizeof(char) * 2048);

      // receiving data from child
      int read_bytes;
      size_t offset = 0;
      while (offset < 2048 && (read_bytes = read(pipe_fd[0], output + offset, 2048 - offset) > 0)) {
        offset += read_bytes;
      }
      close(pipe_fd[0]);

      int status;
      free(full_path);
      // wait(NULL);
      waitpid(process, &status, 0);
      if (!WIFEXITED(status)) {
        snprintf(output, 1024, "Parent: Child exited abnormally with status code %d\n", WEXITSTATUS(status));
      }
      return output;
    }
  } else {
    int len = strlen(args[0]) + 21;
    output = (char*)malloc(sizeof(char) * len);
    snprintf(output, len, "%s: command not found\n", args[0]);
    return output;
  }
}