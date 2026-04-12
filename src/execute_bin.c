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
#include "types.h"

// Receiving output and error from child process
static char* receive_child_output(int fild);

struct Output execute_bin(const struct Argument arguments) {
  int output_fild[2];
  int err_fild[2];
  pipe(output_fild);
  pipe(err_fild);

  pid_t process = fork();
  if (process == 0) { // child process
    close(output_fild[0]);
    close(err_fild[0]);
    dup2(output_fild[1], STDOUT_FILENO);
    dup2(err_fild[1], STDERR_FILENO);
    close(output_fild[1]);
    close(err_fild[1]);

    execvp(arguments.arguments[0], (char* const*)arguments.arguments);
    if (errno == ENOENT) {
      fprintf(stderr, "%s: command not found\n", arguments.arguments[0]);
    } else {
      perror("execvp failed");
    }
    exit(1);
  } else { // parent process
    close(output_fild[1]);
    close(err_fild[1]);

    struct Output output;
    output.output = receive_child_output(output_fild[0]);
    output.error = receive_child_output(err_fild[0]);

    int status;
    // wait(NULL);
    waitpid(process, &status, 0);
    if (!WIFEXITED(status)) {
      printf("Parent: Child exited abnormally with status code %d\n", WEXITSTATUS(status));
    }
    return output;
  }
}

static char* receive_child_output(int fild) {
  size_t offset = 0;
  size_t read_bytes;
  size_t capacity = 256;
  char* result = (char*)malloc(sizeof(char) * capacity);

  while ((read_bytes = read(fild, result + offset, capacity - offset)) > 0) {
    offset += read_bytes;
    if (offset >= capacity) { // Reassign if error buffer runs out of capacity
      capacity *= 2;
      char* new_buffer = (char*)malloc(sizeof(char) * capacity);
      strcpy(new_buffer, result);
      free(result);
      result = new_buffer;
    }
  }

  result[offset] = '\0';
  close(fild);
  return result;
}