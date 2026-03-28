#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "parse_arg.h"

// Returns newly malloc'd array of strings. Each string in the array contains each argument of the entire command
char** parse_args(const char* raw_args) {
  char** args = (char**)malloc(sizeof(char**) * 1024);
  args[0] = (char*)malloc(sizeof(char) * 256);
  const char* cursor = raw_args;
  int arg_count = 0;
  int cur_arg_index = 0;
  bool quote_open = false;

  while (*cursor != '\0') {
    if (*cursor == ' ' && !quote_open) {
      if (cur_arg_index > 0) { // skipping empty argument
        args[arg_count++][cur_arg_index] = '\0';
        args[arg_count] = (char*)malloc(sizeof(char) * 256);
        cur_arg_index = 0;
      }
      cursor++;
      continue;
    } else if (*cursor == '\'') {
      quote_open = !quote_open;
      cursor++;
      continue;
    } else if (*cursor == '~' && !quote_open) { // expanding home directory
      const char* home_dir = getenv("HOME");
      while (*home_dir != '\0') {
        args[arg_count][cur_arg_index++] = *home_dir++;
      }
      cursor++;
      continue;
    }
    args[arg_count][cur_arg_index++] = *cursor++;
  }

  args[++arg_count] = NULL;
  return args;
}