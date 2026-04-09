#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "parse_arg.h"

// treat every character in single quotes literally, including delimiters.
// If it successfully finished reading characters in the enclosing double quotes, it will return true. otherwise, false
static bool literals_single_quote_open(char* dest, const char** cursor_in_raw_args, int* dest_cursor);

// treat every character in double quotes literally, including delimiters. But it can still allow some special characters to be interpreted
// Backlash in double quotes will escape: ', ", \, $, and new_line. Returns number of characters that were appended
// If it successfully finished reading characters in the enclosing double quotes, it will return true. otherwise, false
static bool literals_double_quote_open(char* dest, const char** cursor_in_raw_args, int* dest_cursor);

// Returns newly malloc'd array of strings. Each string in the array contains each argument of the entire command
// It will treat every character within enclosing single quotes literally. within enclosing double quotes, some special characters will be interpreted
// A string after > will be treated as name of the file that output of this program should be redirected to
struct Argument* parse_args(const char* raw_args) {
  // TODO: add safety measure when result has not enough room for all the arguments, output_terminals, and sources
  struct Argument* result = (struct Argument*)malloc(sizeof(struct Argument));
  result->arguments = (char**)malloc(sizeof(char*) * 1024);
  result->arguments[0] = (char*)malloc(sizeof(char) * 256);
  result->output_terminals = (char**)malloc(sizeof(char*) * 1024);
  // TODO: Decide default value for leftover room for sources
  result->error_redirect = (char**)malloc(sizeof(char*) * 64);

  const char* cursor = raw_args;
  int arg_count = 0;
  int cur_arg_index = 0;
  int terminals_count = 0;
  int cur_terminals_index = 0;
  int err_redirect_count = 0;
  int err_redirect_index = 0;

  while (*cursor != '\0') {
    if (*cursor == ' ') {
      while (*cursor == ' ') cursor++; // skipping multiple whitespaces

      if (*cursor != '>' && (*cursor != '1' || cursor[1] != '>') && (*cursor != '2' || cursor[1] != '>')) { 
        result->arguments[arg_count++][cur_arg_index] = '\0';
        result->arguments[arg_count] = (char*)malloc(sizeof(char) * 256);
        cur_arg_index = 0;
      }
      // cursor++;
      continue;
    } else if (*cursor == '\'') { 
      cursor++;
      if (!literals_single_quote_open(result->arguments[arg_count] + cur_arg_index, &cursor, &cur_arg_index)) {
        free(result->arguments);
        free(result->output_terminals);
        free(result);
        return NULL;
      }
      continue;
    } else if (*cursor == '\"') {
      cursor++;
      if (!literals_double_quote_open(result->arguments[arg_count] + cur_arg_index, &cursor, &cur_arg_index)) {
        free(result->arguments);
        free(result->output_terminals);
        free(result);
        return NULL;
      }
      continue;
    } else if (*cursor == '\\') {
      cursor++;
    } else if (*cursor == '>' || (*cursor == '1' && cursor[1] == '>')) { // output redirector. assume it will be only one >
      cursor += *cursor == '>' ? 1 : 2;
      result->output_terminals[terminals_count] = (char*)malloc(sizeof(char) * 256);
      cur_terminals_index = 0;
      while (*cursor == ' ') cursor++; // skip whitespaces
      while (*cursor != ' ' && *cursor != '\0' && *cursor != '>') {
        // account for single and double quote
        if (*cursor == '\'') {
          cursor++;
          if (!literals_single_quote_open(result->output_terminals[terminals_count] + cur_terminals_index, &cursor, &cur_terminals_index)) {
            free(result->arguments);
            free(result->output_terminals);
            free(result->error_redirect);
            free(result);
            return NULL; // "Invalid single quote usage: Failed to parse"
          } 
          continue;
        } else if (*cursor == '\"') {
          cursor++;
          if (!literals_double_quote_open(result->output_terminals[terminals_count] + cur_terminals_index, &cursor, &cur_terminals_index)) {
            free(result->arguments);
            free(result->output_terminals);
            free(result->error_redirect);
            free(result);
            return NULL; // "Invalid double quote usage: Failed to parse"
          }
          continue;
        } else if (*cursor == '\\') {
          cursor++;
        }
        result->output_terminals[terminals_count][cur_terminals_index++] = *cursor++;
      }
      result->output_terminals[terminals_count++][cur_terminals_index] = '\0';
      continue;
    } else if ((*cursor == '2' && cursor[1] == '>')) {
      cursor += *cursor == '>' ? 1 : 2;
      // result->output_terminals[terminals_count] = (char*)malloc(sizeof(char) * 256);
      result->error_redirect[err_redirect_count] = (char*)malloc(sizeof(char) * 256);
      err_redirect_index = 0;
      while (*cursor == ' ') cursor++; // skip whitespaces
      while (*cursor != ' ' && *cursor != '\0' && *cursor != '>') {
        // account for single and double quote
        if (*cursor == '\'') {
          cursor++;
          if (!literals_single_quote_open(result->error_redirect[err_redirect_count] + err_redirect_index, &cursor, &err_redirect_index)) {
            free(result->arguments);
            free(result->output_terminals);
            free(result->error_redirect);
            free(result);
            return NULL; // "Invalid single quote usage: Failed to parse"
          } 
          continue;
        } else if (*cursor == '\"') {
          cursor++;
          if (!literals_double_quote_open(result->error_redirect[err_redirect_count] + err_redirect_index, &cursor, &err_redirect_index)) {
            free(result->arguments);
            free(result->output_terminals);
            free(result->error_redirect);
            free(result);
            return NULL; // "Invalid double quote usage: Failed to parse"
          }
          continue;
        } else if (*cursor == '\\') {
          cursor++;
        }
        result->error_redirect[err_redirect_count][err_redirect_index++] = *cursor++;
      }
      result->error_redirect[err_redirect_count++][err_redirect_index] = '\0';
      continue;
    } else if (*cursor == '~') { // expanding home directory
      const char* home_dir = getenv("HOME");
      strcpy(result->arguments[arg_count] + cur_arg_index, home_dir);
      cursor++; 
      cur_arg_index += strlen(home_dir);
      continue;
    }
    result->arguments[arg_count][cur_arg_index++] = *cursor++;
  }

  result->arguments[arg_count][cur_arg_index] = '\0';
  result->arguments[++arg_count] = NULL;
  result->output_terminals[terminals_count] = NULL;
  return result;
}

// TODO: try to implement parse_args so it uses less lines of codes
// struct Argument* parse_args_v2(const char* raw_args) {
//   struct Argument* result = (struct Argument*)malloc(sizeof(struct Argument));
//   result->arguments = (char**)malloc(sizeof(char*) * 1024);
//   result->output_terminals = (char**)malloc(sizeof(char*) * 1024);

//   while (*cursor != '\0') {
//     // when reading a character,
//     /*
//     skipping whitespaces?
//     if single quote is open -> treat this character literally
//     if double quote is open -> treat this character literally, if not one of the exceptions: ',",$,\n,\
//     if single quote is opening/closing -> invert single_quote_open
//     if double quote is opening/closing

//     if current character goes to arguments
//     if current character goes to output_terminals

//     process by character? process by delimiter?
//     */
//   }
// }

static bool literals_single_quote_open(char* dest, const char** cursor_in_raw_args, int* dest_cursor) {
  char* debug = dest;
  // int count = 0;
  while (**cursor_in_raw_args != '\0' && **cursor_in_raw_args != '\'') {
    *dest++ = *(*cursor_in_raw_args)++;
    (*dest_cursor)++;
  }
  return *(*cursor_in_raw_args)++ == '\'';
}

static bool literals_double_quote_open(char* dest, const char** cursor_in_raw_args, int* dest_cursor) {
  char* debug = dest;
  while (**cursor_in_raw_args != '\0' && **cursor_in_raw_args != '\"') {
    if (**cursor_in_raw_args == '\\') {
      switch ((*cursor_in_raw_args)[1]) {
        case '\"': case '\'': case '$': case '\\': case '\n':
          (*cursor_in_raw_args)++;
          break;
        default:
          break;
      }
    }
    *dest++ = *(*cursor_in_raw_args)++;
    (*dest_cursor)++;
  }

  return *(*cursor_in_raw_args)++ == '\"';
}