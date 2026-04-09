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

void execute_bin(const struct Argument* arguments) {
  char* full_path = locate_bin(arguments->arguments[0]);

  if (full_path) {
    pid_t process = fork();
    if (process == 0) { // child process
      char** cursor = arguments->output_terminals;
      while (*cursor) {
        int fd = open(*cursor, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        dup2(fd, STDOUT_FILENO); // redirecting stdout to output_stream file(s)
        close(fd);
        cursor++;
      }

      cursor = arguments->error_redirect;
      while (*cursor) { // redirecting stderror 
        int fd = open(*cursor, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        dup2(fd, STDERR_FILENO);
        close(fd);
        cursor++;
      }
      execv(full_path, (char* const*)arguments->arguments);
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
    free(full_path);
    char** cursor = arguments->output_terminals;
    if (!(*cursor)) { // If there's no redirected output file, print to stdout
      printf("%s: command not found\n", arguments->arguments[0]);
      return;
    } 

    while (*cursor) { // print to redirected output file(s)
      int fd = open(*cursor, O_WRONLY | O_CREAT | O_TRUNC, 0644);
      FILE* file_ptr = fopen(*cursor, "w");
      if (!file_ptr) {
        printf("Failed to open file: %s\n", *cursor);
        printf("%s: command not found\n", arguments->arguments[0]);
      } else {
        fprintf(file_ptr, "%s: command not found\n", arguments->arguments[0]);
      }
      fclose(file_ptr);
      cursor++;
    }
  }
}