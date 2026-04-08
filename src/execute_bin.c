#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <fcntl.h>

#include "execute_bin.h"
#include "locate_bin.h"
#include "parse_arg.h"

void execute_bin(const char** args, const char** output_stream) {
  char* full_path = locate_bin(args[0]);

  if (full_path) {
    pid_t process = fork();
    if (process == 0) { // child process
      const char** cursor = output_stream;
      while (*cursor) {
        int fd = open(*cursor, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (dup2(fd, STDOUT_FILENO) == -1) { // redirecting stdout to output_stream file(s)
          printf("Failed to open file: %s\n", *cursor);
        };
        close(fd);
        cursor++;
      }
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
    const char** cursor = output_stream;
    if (!(*cursor)) { // empty redirection
      printf("%s: command not found\n", args[0]);
      return;
    }

    while (*cursor) {
      int fd = open(*cursor, O_WRONLY | O_CREAT | O_TRUNC, 0644);
      FILE* file_ptr = fopen(*cursor, "w");
      if (!file_ptr) {
        printf("Failed to open file parent: %s\n", *cursor);
      } else {
        fprintf(file_ptr, "%s: command not found\n", args[0]);
      }
      fclose(file_ptr);
      cursor++;
    }
  }
}