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

struct Output execute_bin(const struct Argument* arguments) {
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

    execvp(arguments->arguments[0], (char* const*)arguments->arguments);
    if (errno == ENOENT) {
      fprintf(stderr, "%s: command not found\n", arguments->arguments[0]);
    } else {
      perror("execvp failed");
    }
    exit(1);
  } else { // parent process
    close(output_fild[1]);
    close(err_fild[1]);

    size_t capacity = 1024;
    struct Output output;
    output.output = (char*)malloc(sizeof(char) * capacity);
    output.error = (char*)malloc(sizeof(char) * capacity);

    int status;
    size_t offset = 0;
    size_t read_bytes;

    while ((read_bytes = read(output_fild[0], output.output + offset, capacity - offset)) > 0) {
      offset += read_bytes;
      if (offset >= capacity) { // Reassign if output buffer runs out of capacity
        capacity *= 2;
        char* new_output_buffer = (char*)malloc(sizeof(char) * capacity);
        strcpy(new_output_buffer, output.output);
        free(output.output);
        output.output = new_output_buffer;
      }
    }
    output.output[offset] = '\0';
    close(output_fild[0]);

    offset = 0;
    capacity = 1024;

    while ((read_bytes = read(err_fild[0], output.error + offset, capacity - offset)) > 0) {
      offset += read_bytes;
      if (offset >= capacity) { // Reassign if error buffer runs out of capacity
        capacity *= 2;
        char* new_err_buffer = (char*)malloc(sizeof(char) * capacity);
        strcpy(new_err_buffer, output.error);
        free(output.error);
        output.error = new_err_buffer;
      }
    }
    output.error[offset] = '\0';
    close(err_fild[0]);
    
    // wait(NULL);
    waitpid(process, &status, 0);
    if (!WIFEXITED(status)) {
      printf("Parent: Child exited abnormally with status code %d\n", WEXITSTATUS(status));
    }
    return output;
  }
}