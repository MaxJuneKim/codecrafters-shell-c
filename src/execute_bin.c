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

#include "execute_bin.h"
#include "locate_bin.h"
#include "parse_arg.h"
#include "types.h"

struct Pipeline {
  pid_t child_process_id;
  int output_to_next_cmd_fild_w; // channel where input into a child process, from previous command should be written to
  int input_from_cur_cmd_fild_r; // channel where output that a child process produces are written
  int err_from_cur_cmd_fild_r;  // channel where error that a child process produces are written
  
  FILE** output_redir;
  FILE** err_redir;
};

struct Pipeline init_pipeline(
  pid_t child_process_id, 
  int input_from_cur_cmd_fild_r, 
  int output_to_next_cmd_fild_w, 
  int err_from_cur_cmd_fild_r, 
  FILE** output_redir, 
  FILE** err_redir
) {
  struct Pipeline result;
  result.child_process_id = child_process_id;
  result.input_from_cur_cmd_fild_r = input_from_cur_cmd_fild_r;
  result.output_to_next_cmd_fild_w = output_to_next_cmd_fild_w;
  result.err_from_cur_cmd_fild_r = err_from_cur_cmd_fild_r;
  result.output_redir = output_redir;
  result.err_redir = err_redir;
  return result;
}

static void* data_terminal(void* arg);

void execute_bin(const struct Argument* commands) {
  int output_filds[64][2];
  int input_filds[64][2];
  int err_filds[64][2];
  struct Pipeline* pipelines = (struct Pipeline*)malloc(sizeof(struct Pipeline) * 64);
  pthread_t child_processes_channels[64];

  const struct Argument* cursor = commands;
  size_t ids_index = 0;

  while (cursor->arguments && cursor->output_terminals && cursor->error_terminals) {
    pipe(output_filds[ids_index]);
    pipe(input_filds[ids_index]);
    pipe(err_filds[ids_index]);

    pipelines[ids_index].output_to_next_cmd_fild_w = -1;
    if (ids_index > 0) pipelines[ids_index - 1].output_to_next_cmd_fild_w = input_filds[ids_index][1];
    pipelines[ids_index].input_from_cur_cmd_fild_r = output_filds[ids_index][0];
    pipelines[ids_index].err_from_cur_cmd_fild_r = err_filds[ids_index][0];

    // Redirectors
    pipelines[ids_index].output_redir = cursor->output_terminals;
    pipelines[ids_index].err_redir = cursor->error_terminals;

    pid_t pid = fork();
    if (pid == 0) { // child process
      close(output_filds[ids_index][0]);
      close(input_filds[ids_index][1]);
      close(err_filds[ids_index][0]);

      dup2(output_filds[ids_index][1], STDOUT_FILENO);
      if (ids_index > 0) dup2(input_filds[ids_index][0], STDIN_FILENO);
      dup2(err_filds[ids_index][1], STDERR_FILENO);
      
      close(output_filds[ids_index][1]);
      close(input_filds[ids_index][0]);
      close(err_filds[ids_index][1]);

      execvp(cursor->arguments[0], cursor->arguments);
      if (errno == ENOENT) {
        fprintf(stderr, "%s: command not found\n", cursor->arguments[0]);
      } else {
        perror("execvp failed");
      }
      exit(1);
    } else { // spawn thread?
      close(output_filds[ids_index][1]);
      close(input_filds[ids_index][0]);
      close(err_filds[ids_index][1]);

      pipelines[ids_index].child_process_id = pid;
      pthread_create(&child_processes_channels[ids_index], NULL, data_terminal, pipelines + ids_index);
    }
    ids_index++;
    cursor++;
  }

  for (size_t i = 0; i < ids_index; i++) {
    pthread_join(child_processes_channels[i], NULL);
    // close();
  }
  free(pipelines);
}

void* data_terminal(void* const arg) {
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
        ssize_t n = read(fds[i].fd, buffer, 1023);
        buffer[n] = '\0';
        if (n > 0) { // Receive output and error from child and redirect it properly
          // output_to_next_cmd_fild_w being -1 signals the last command, thus, no need to redirect it
          if (fds[i].fd == pipeline->input_from_cur_cmd_fild_r && pipeline->output_to_next_cmd_fild_w != -1) {
            write(pipeline->output_to_next_cmd_fild_w, buffer, n); // writing to next cmd
          }
          FILE** redir_cursor = fds[i].fd == pipeline->input_from_cur_cmd_fild_r ? pipeline->output_redir : pipeline->err_redir;
          while (*redir_cursor) fputs(buffer, *redir_cursor++); // writing to redirected files
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