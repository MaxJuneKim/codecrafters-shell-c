#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#if defined(__WIN32)
  #include <Windows.h>
#elif defined(__linux__) || defined(__unix__) || defined(TARGET_OS_MAC)
  #include <termios.h>
#endif

#include "echo.h"
#include "global_vars.h"
#include "type.h"
#include "execute_bin.h"
#include "parse_arg.h"
#include "Navigation/pwd.h"
#include "Navigation/cd.h"
#include "types.h"
#include "locate_bin.h"
#include "tab.h"

// TODO: 
// I'm facing plenty of cases where output of my local run and codecrafter testing are different.
// I wonder if this is an OS issue because I am using Mac OS. Build this project in WSL as well and Windows if possible
// and see if I can similar results 

void write_to_files(FILE** file_ptrs, const char* output) {
  while (*file_ptrs) {
    if (output) fputs(output, *file_ptrs);
    if (*file_ptrs == stdout || *file_ptrs == stderr) {
      file_ptrs++;
      continue;
    }
    fclose(*file_ptrs++);
  }
}

void executeCommand(char* input) {
  if (*input == '\0') { // empty command
    return;
  }

  struct Argument args = parse_args(input);
  struct Output output;
  if (!args.arguments[0]) { // signal for parsing failed
    printf("Failed to parse command: %s\n", input);
    return;
  }

  if (strcmp(args.arguments[0], "echo") == 0) {
    output = echo((const char**)args.arguments);
  } else if (strcmp(args.arguments[0], "type") == 0) {
    output = executeType(args.arguments[1]);
  } else if (strcmp(args.arguments[0], "pwd") == 0) {
    output = pwd();
  } else if (strcmp(args.arguments[0], "cd") == 0) {
    output = cd(args.arguments[1]);
  } else {
    output = execute_bin(args);
  }

  write_to_files(args.output_terminals, output.output);
  write_to_files(args.error_terminals, output.error);
  
  // deallocating
  if (output.output) free(output.output);
  if (output.error) free(output.error);
  free_arg(args);
}

int main(int argc, char *argv[]) {
  load_all_executables();

  // Flush after every printf
  setbuf(stdout, NULL);
  struct termios raw;
  struct termios orig;
  tcgetattr(STDIN_FILENO, &orig);
  raw = orig;
  raw.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &raw);
  
  char input[1024];
  bool second_tab = false;
  char** matching_executables = (char**)malloc(sizeof(char*) * PATH_EXECUTABLES_COUNT);
  while (true) {
    printf("$ ");

    size_t cursor = 0;
    while (read(STDIN_FILENO, input + cursor, 1) == 1 && (input[cursor] != '\n' && input[cursor] != '\r')) {
      if (input[cursor] == 127 && cursor > 0) { // backspace
        printf("\b \b");
        cursor--;
        second_tab = false;
      } else if (input[cursor] == '\t') {
        tab(input, matching_executables, &cursor, &second_tab);
      } else if (input[cursor] != 127) {
        printf("%c", input[cursor++]);
        second_tab = false;
      } else {
        second_tab = false;
      }
    }
    input[cursor] = '\0';
    printf("%c", '\n');
    second_tab = false;

    if (strcmp(input, "exit") == 0) {
      break;
    }
    
    executeCommand(input);
  }
  
  tcsetattr(STDIN_FILENO, TCSANOW, &orig);
  free(ALL_EXECUTABLES);
  return 0;
}
