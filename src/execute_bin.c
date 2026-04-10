#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <fcntl.h>
#include <errno.h> 

#include "execute_bin.h"
#include "locate_bin.h"
#include "parse_arg.h"

void execute_bin(const struct Argument* arguments) {
  pid_t process = fork();
  if (process == 0) { // child process
    char** cursor = arguments->output_terminals;
    int stdout_fd = dup(STDOUT_FILENO);

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
    execvp(arguments->arguments[0], (char* const*)arguments->arguments);

    if (errno == ENOENT) {
      dup2(stdout_fd, STDOUT_FILENO);
      printf("%s: command not found\n", arguments->arguments[0]);
    }
    // perror("execvp failed");
    exit(1);
  } else { // parent process
    int status;
    // wait(NULL);
    waitpid(process, &status, 0);
    if (!WIFEXITED(status)) {
      printf("Parent: Child exited abnormally with status code %d\n", WEXITSTATUS(status));
    }
  }
}