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
#include "execute_cmd.h"
#include "parse_arg.h"
#include "Navigation/pwd.h"
#include "Navigation/cd.h"
#include "types.h"
#include "locate_bin.h"
#include "tab.h"
#include "history.h"

// TODO: 
// I'm facing plenty of cases where output of my local run and codecrafter testing are different.
// I wonder if this is an OS issue because I am using Mac OS. Build this project in WSL as well and Windows if possible
// and see if I can get similar results 

void executeCommand(const char* input) {
  if (*input == '\0') { // empty command
    return;
  }

  struct Argument* args = parse_args(input);
  if (!args) { // signal for parsing failed
    printf("Failed to parse command: %s\n", input);
    return;
  }
  execute_cmd(args);

  for (size_t i = 0; args[i].arguments; i++) free_arg(args[i]);
  free(args);
}

int main(int argc, char *argv[]) {
  load_all_executables();
  // load_history_from_file();

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

    add_to_history(input);

    if (strcmp(input, "exit") == 0) {
      break;
    }
    
    executeCommand(input);
  }
  
  store_history();
  tcsetattr(STDIN_FILENO, TCSANOW, &orig);
  for (size_t i = 0; i < PATH_EXECUTABLES_COUNT; i++) free(ALL_EXECUTABLES[i]);
  free(ALL_EXECUTABLES);
  free(matching_executables);
  return 0;
}
