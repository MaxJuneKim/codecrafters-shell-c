#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <fcntl.h>
#include <errno.h> 
#include <poll.h>
#include <pthread.h>

#include "execute_cmd.h"
#include "locate_bin.h"
#include "parse_arg.h"
#include "types.h"
#include "echo.h"
#include "pwd.h"
#include "cd.h"
#include "type.h"

struct Pipeline {
  pid_t child_process_id;
  int output_to_next_cmd_fild_w; // write pipe where input into a child process, from previous command should be written to. 
  int input_from_cur_cmd_fild_r; // read pipe where output that a child process produces are written. If current command is a builtin, this will be ignored.
  int err_from_cur_cmd_fild_r;  // read pipe where error that a child process produces are written. If current command is a builtin, this will be ignored.
  const struct Argument* argument;
};

struct Pipeline init_pipeline(
  pid_t child_process_id, 
  int input_from_cur_cmd_fild_r, 
  int output_to_next_cmd_fild_w, 
  int err_from_cur_cmd_fild_r, 
  const struct Argument* argument
) {
  struct Pipeline result;
  result.child_process_id = child_process_id;
  result.input_from_cur_cmd_fild_r = input_from_cur_cmd_fild_r;
  result.output_to_next_cmd_fild_w = output_to_next_cmd_fild_w;
  result.err_from_cur_cmd_fild_r = err_from_cur_cmd_fild_r;
  result.argument = argument;
  return result;
}

// Helper functions

// function containing logic to execute builtin commands
static void* execute_built_in(void* arg);

// function containing logic to communicate with a child process executing a binary. It will receive error/output from the child process,
// write to a proper redirected files, and streamline to stdin of next child process if pipelines were specified. 
static void* execute_bin(void* arg);

// Helper function. Return true if passed command is a builtin, false otherwise
static bool is_built_in(const char* cmd) {
  return strcmp(cmd, "echo") == 0 || strcmp(cmd, "type") == 0 || strcmp(cmd, "pwd") == 0 || strcmp(cmd, "cd") == 0;
}

void execute_cmd(const struct Argument* commands) {
  const struct Argument* cmd_cursor = commands;

  int output_filds[64][2];
  int input_filds[64][2];
  int err_filds[64][2];
  struct Pipeline* pipelines = (struct Pipeline*)malloc(sizeof(struct Pipeline) * 64);
  pthread_t thread_channel[64];

  size_t ids_index = 0;
  size_t len = 0;

  while (cmd_cursor->arguments && cmd_cursor->output_terminals && cmd_cursor->error_terminals) {
    if (!is_built_in(cmd_cursor->arguments[0])) {
      pipe(output_filds[len]);
      pipe(input_filds[len]);
      pipe(err_filds[len]);
    } else {
      output_filds[len][0] = -1;
      output_filds[len][1] = -1;
      input_filds[len][0] = -1;
      input_filds[len][1] = -1;
      err_filds[len][0] = -1;
      err_filds[len][1] = -1;
    }
    cmd_cursor++;
    len++;
  }

  cmd_cursor = commands;
  while (ids_index < len) { // for each command
    if (ids_index < len - 1) pipelines[ids_index].output_to_next_cmd_fild_w = input_filds[ids_index + 1][1]; // if next cmd is a builtin, it would be -1
    else pipelines[ids_index].output_to_next_cmd_fild_w = -1;
    pipelines[ids_index].input_from_cur_cmd_fild_r = output_filds[ids_index][0]; // -1 if cur_cmd is a builtin
    pipelines[ids_index].err_from_cur_cmd_fild_r = err_filds[ids_index][0]; // -1 if cur_cmd is a builtin
    pipelines[ids_index].argument = cmd_cursor;

    if (is_built_in(cmd_cursor->arguments[0])) { // if built-in thread
      // spawn built-in thread
      pthread_create(&thread_channel[ids_index], NULL, execute_built_in, pipelines + ids_index);
    } else { // if command is binary
      // execute the binary, spawn_binary_thread
      pid_t pid = fork();
      if (pid == 0) { // child process
        for (size_t i = 0; i < len; i++) {
          close(output_filds[i][0]);
          close(input_filds[i][1]);
          close(err_filds[i][0]);

          if (i == ids_index) {
            dup2(output_filds[ids_index][1], STDOUT_FILENO);
            if (ids_index > 0 && !is_built_in(cmd_cursor->arguments[0])) dup2(input_filds[ids_index][0], STDIN_FILENO); // If cur cmd is a builtin, no need to redirect input
            dup2(err_filds[ids_index][1], STDERR_FILENO);
          }

          close(output_filds[i][1]);
          close(input_filds[i][0]);
          close(err_filds[i][1]);
        }

        execvp(cmd_cursor->arguments[0], cmd_cursor->arguments);
        if (errno == ENOENT) {
          fprintf(stderr, "%s: command not found\n", cmd_cursor->arguments[0]);
        } else {
          perror("execvp failed");
        }
        exit(1);
      } else { // spawn thread?
        close(output_filds[ids_index][1]);
        close(input_filds[ids_index][0]);
        close(err_filds[ids_index][1]);

        pipelines[ids_index].child_process_id = pid;
        pthread_create(&thread_channel[ids_index], NULL, execute_bin, pipelines + ids_index);
      }
    }
    ids_index++;
    cmd_cursor++;
  }

  for (size_t i = 0; i < ids_index; i++) {
    pthread_join(thread_channel[i], NULL);
  }
  free(pipelines);
}

void* execute_built_in(void* arg) { 
  // This thread-executed function does not have ownership of the passed instance pipeline argument, thus should not free it
  struct Pipeline* pipeline = (struct Pipeline*)arg;
  struct Output output;

  if (strcmp(pipeline->argument->arguments[0], "echo") == 0) { // if built-in thread
    output = echo((const char**)pipeline->argument->arguments);
  } else if (strcmp(pipeline->argument->arguments[0], "type") == 0) {
    output = executeType(pipeline->argument->arguments[1]);
  } else if (strcmp(pipeline->argument->arguments[0], "pwd") == 0) {
    output = pwd();
  } else if (strcmp(pipeline->argument->arguments[0], "cd") == 0) {
    output = cd(pipeline->argument->arguments[1]);
  } 

  // Write to next cmd. Check if output to next command fd is -1, signaling another builtin
  if (pipeline->output_to_next_cmd_fild_w != -1) {
    write(pipeline->output_to_next_cmd_fild_w, output.output, strlen(output.output));
    close(pipeline->output_to_next_cmd_fild_w);
  }

  FILE** output_cursor = pipeline->argument->output_terminals;
  while (*output_cursor && output.output) fputs(output.output, *output_cursor++);
  FILE** err_cursor = pipeline->argument->error_terminals;
  while (*err_cursor && output.error) fputs(output.error, *err_cursor++);

  free(output.output);
  free(output.error);

  return NULL;
}

void* execute_bin(void* const arg) {
  // This thread-executed function does not have ownership of the passed instance pipeline argument, thus should not free it
  // TODO: maybe add typedef that differentiates between pointers that should have ownership of block of memory with head of malloc with correct metadata and those that don't?
  // Note about Malloc: malloc returns head of the allocated memory along with small metadata, usually 8 or 16 bytes, which stores information such as the total size of the block 
  // and whether it is free or allocated. Thus, trying to free offset memory like this: free(memory + 4() can of often lead to severe result and crash
  struct Pipeline* pipeline = (struct Pipeline*)arg;
  struct pollfd fds[2];
  fds[0].fd = pipeline->input_from_cur_cmd_fild_r;
  fds[0].events = POLLIN;
  fds[1].fd = pipeline->err_from_cur_cmd_fild_r;
  fds[1].events = POLLIN;

  char buffer[1024];
  int open_pipes = 2;

  while (open_pipes > 0) {
    int ret = poll(fds, 2, -1); // waiting for event mask to check if the pipe is ready for read
    if (ret < 0) {
      fputs("Error occured in IO. Please try again\n", stdout);
    }

    for (int i = 0; i < 2; i++) {
      if (fds[i].revents & POLLIN) {
        // Safety measure to check if a file descriptor is -1, signaling not to read it
        ssize_t n = fds[i].fd >= 0 ? read(fds[i].fd, buffer, 1023) : 0;
        buffer[n] = '\0';
        if (n > 0) { // Receive output and error from child and redirect it properly
          // output_to_next_cmd_fild_w being -1 signals the last command, thus, no need to redirect it. It also serves as a safety measure in case next cmd is a builtin
          if (fds[i].fd == pipeline->input_from_cur_cmd_fild_r && pipeline->output_to_next_cmd_fild_w >= 0) {
            write(pipeline->output_to_next_cmd_fild_w, buffer, n); // writing to next cmd
          }
          FILE** redir_cursor = fds[i].fd == pipeline->input_from_cur_cmd_fild_r ? pipeline->argument->output_terminals : pipeline->argument->error_terminals;
          while (*redir_cursor) fputs(buffer, *redir_cursor++);
          // writing to redirected files
        }
      }
      if (fds[i].revents & POLLHUP) { 
        // Although the EOF reached logic can be in the previous if loop, this seems to fail on codecrafters server because 
        // linux may return POLLIN when POLLHUP is returned. thus, to account for both mac and linux, plain if is used instead of else if
        // EOF reached on this pipe
        close(fds[i].fd);
        fds[i].fd = -1;
        open_pipes--;
      }
    }
  }

  int status;
  // wait(NULL);
  waitpid(pipeline->child_process_id, &status, 0);
  if (!WIFEXITED(status)) {
    printf("Parent: Child exited abnormally with status code %d\n", WEXITSTATUS(status));
  }

  // clean up
  close(pipeline->output_to_next_cmd_fild_w);
  return NULL;
}