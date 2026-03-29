#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "parse_arg.h"

// treat every character in single quotes literally, including delimiters. Returns number of characters that were appended
static const int literals_single_quote_open(char* dest, const char* cursor_in_raw_args);

// treat every character in double quotes literally, including delimiters. But it can still allow some special characters to be interpreted
// Returns number of characters that were appended
static const int literals_double_quote_open(char* dest, const char* cursor_in_raw_args);

// Returns newly malloc'd array of strings. Each string in the array contains each argument of the entire command
char** parse_args(const char* raw_args) {
  char** args = (char**)malloc(sizeof(char**) * 1024);
  args[0] = (char*)malloc(sizeof(char) * 256);

  const char* cursor = raw_args;
  int arg_count = 0;
  int cur_arg_index = 0;

  while (*cursor != '\0') {
    if (*cursor == ' ') {
      if (cur_arg_index > 0) { // skipping empty argument
        args[arg_count++][cur_arg_index] = '\0';
        args[arg_count] = (char*)malloc(sizeof(char) * 256);
        cur_arg_index = 0;
      }
      cursor++;
      continue;
    } else if (*cursor == '\'') { 
      int n = literals_single_quote_open(args[arg_count] + cur_arg_index, cursor + 1);
      if (n == -1) {
        return NULL;
      } 
      cur_arg_index += n;
      cursor += (n + 2);
      
      continue;
    } else if (*cursor == '\"') {
      int n = literals_double_quote_open(args[arg_count] + cur_arg_index, cursor + 1);
      if (n == -1) {
        return NULL;
      } 
      cur_arg_index += n;
      cursor += (n + 2);
      continue;
    } else if (*cursor == '\\') {
      cursor++;
    } else if (*cursor == '~') { // expanding home directory
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

static const int literals_single_quote_open(char* dest, const char* cursor_in_raw_args) {
  char* debug = dest;
  int count = 0;
  while (*cursor_in_raw_args != '\0' && *cursor_in_raw_args != '\'') {
    *dest++ = *cursor_in_raw_args++;
    count++;
  }
  return *cursor_in_raw_args == '\'' ? count : -1;
}

static const int literals_double_quote_open(char* dest, const char* cursor_in_raw_args) {
  char* debug = dest;
  int count = 0;
  while (*cursor_in_raw_args != '\0' && *cursor_in_raw_args != '\"') {
    *dest++ = *cursor_in_raw_args++;
    count++;
  }
  return *cursor_in_raw_args == '\"' ? count : -1;
}