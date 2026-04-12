#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "parse_arg.h"

enum TOKEN_TYPE {
  ARG,
  STDOUT_REDIR,
  STDERR_REDIR,
  STDOUT_REDIR_APPEND,
  STDERR_REDIR_APPEND
};

struct TOKEN {
  char* token;
  enum TOKEN_TYPE token_type;
};

// helper functions

// treat every character in single quotes literally, including delimiters.
// If it successfully finished reading characters in the enclosing double quotes, it will return true. otherwise, false
static bool literals_single_quote_open(char* dest, const char** cursor_in_raw_args, int* dest_cursor);

// treat every character in double quotes literally, including delimiters. But it can still allow some special characters to be interpreted
// Backlash in double quotes will escape: ', ", \, $, and new_line. Returns number of characters that were appended
// If it successfully finished reading characters in the enclosing double quotes, it will return true. otherwise, false
static bool literals_double_quote_open(char* dest, const char** cursor_in_raw_args, int* dest_cursor);

// Get next token, which could be either normal argument, output redirect destination, or error redirect destination
// Return newly malloc'd string that contains the expanded, full version of each argument. If parsing has failed, return null
static struct TOKEN* get_next_token(const char** cursor);

// helper function to get_next_token. Consume next character(s) and return number of characters were appended as result. 
// Operate under assumption that quote is not open. There may be cases where more than one character is consumed
// returning pointer to the next character to be consumed might be an idea as well
static int consume_next_character(char* dest, const char** cursor);

// check if the current character is a delimeter and current token should end
// '\0': terminating null character
// ' ': empty space
// > or 1>: stdout redirect operator
// 2> : stderr redirect operator
static bool is_delimeter(const char* cursor);

static bool is_output_redir(const char* cursor);

static bool is_err_redir(const char* cursor);

static bool is_output_appd_redir(const char* cursor);

static bool is_err_appd_redir(const char* cursor);

// header defined functions
void free_arg(struct Argument args) {
  char** cursor = args.arguments;
  while (*cursor) free(*cursor++);
  free(args.arguments);
}

// Returns newly malloc'd array of strings. Each string in the array contains each argument of the entire command
// It will treat every character within enclosing single quotes literally. within enclosing double quotes, some special characters will be interpreted
// A string after > will be treated as name of the file that output of this program should be redirected to
struct Argument parse_args(const char* raw_args) {
  const char* cursor = raw_args;
  struct Argument result; // = (struct Argument*)malloc(sizeof(struct Argument));
  // TODO: Decide default value for leftover room for sources and implement auto resizing for safety measure
  result.arguments = (char**)malloc(sizeof(char*) * 256);
  result.output_terminals = (FILE**)malloc(sizeof(FILE*) * 64);
  result.error_terminals = (FILE**)malloc(sizeof(FILE*) * 64);
  result.output_terminals[0] = stdout;
  result.error_terminals[0] = stderr;

  int arg_index = 0;
  int output_index = 0;
  int error_index = 0;

  while (*cursor != '\0') {
    struct TOKEN* next_token = get_next_token(&cursor);
    if (!next_token || *next_token->token == '\0') { // parsing has failed
      result.arguments[0] = NULL;
      result.output_terminals[0] = NULL;
      result.error_terminals[0] = NULL;
      return result;
    }

    char mode[2];
    switch (next_token->token_type) {
      case ARG:
        result.arguments[arg_index++] = next_token->token;
        break;
      case STDOUT_REDIR: case STDOUT_REDIR_APPEND: 
        strcpy(mode, next_token->token_type == STDOUT_REDIR ? "w" : "a");
        result.output_terminals[output_index++] = fopen(next_token->token, mode);
        break;
      case STDERR_REDIR: case STDERR_REDIR_APPEND: 
        strcpy(mode, next_token->token_type == STDERR_REDIR ? "w" : "a");
        result.error_terminals[error_index++] = fopen(next_token->token, mode);
        break;
    }
  }

  result.arguments[arg_index] = NULL;
  result.output_terminals[output_index > 1 ? output_index : 1] = NULL;
  result.error_terminals[error_index > 1 ? error_index : 1] = NULL;
  return result;
}

static struct TOKEN* get_next_token(const char** cursor) {
  struct TOKEN* result = (struct TOKEN*)malloc(sizeof(struct TOKEN));
  result->token = (char*)malloc(sizeof(char) * 256);
  int index = 0;

  // skip whitespaces
  while (**cursor == ' ') (*cursor)++;

  // Decide if current token should be normal arg, stdout, stderr, stdout append, or stderr append redirector
  if (is_output_appd_redir(*cursor)) {
    result->token_type = STDOUT_REDIR_APPEND;
    (*cursor) += **cursor == '>' ? 2 : 3;
    while (**cursor == ' ') (*cursor)++;
  } else if (is_output_redir(*cursor)) { // append operator
    result->token_type = STDOUT_REDIR;
    (*cursor) += **cursor == '>' ? 1 : 2;
    while (**cursor == ' ') (*cursor)++;
  } else if (is_err_appd_redir(*cursor)) {
    result->token_type = STDERR_REDIR_APPEND;
    (*cursor) += 3;
    while (**cursor == ' ') (*cursor)++;
  } else if (is_err_redir(*cursor)) {
    result->token_type = STDERR_REDIR;
    (*cursor) += 2;
    while (**cursor == ' ') (*cursor)++;
  }  else {
    result->token_type = ARG;
  }

  // Decision: When I encounter single or double quote, do I deal with the characters enclosed one by one in a next loop? or just take care
  // of them at once so that I don't have to worry about single or double quote being open in the loop? For now, let's go with the latter.
  while (!is_delimeter(*cursor)) {
    if (**cursor == '\'') {
      (*cursor)++;
      if (!literals_single_quote_open(result->token + index, cursor, &index)) {
        free(result->token);
        free(result);
        return NULL;
      }
      continue;
    } else if (**cursor == '\"') {
      (*cursor)++;
      if (!literals_double_quote_open(result->token + index, cursor, &index)) {
        free(result->token);
        free(result);
        return NULL;
      }
      continue;
    }
    index += consume_next_character(result->token + index, cursor);
  } 

  result->token[index] = '\0';
  return result;
}

static int consume_next_character(char* dest, const char** cursor) {
  switch (**cursor) { // If we need to append special characters that can be expanded, add here
  // These cases are dismissed for now as we assume quote should not be open. Should we decide otherwise, we would open this later.
  // case '\'': 
  //   break;
  // case '\"':
  //   break;
  case '\\': // if encounter \ character, treat next character literally
    *dest = *++(*cursor);
    (*cursor)++;
    return 1;
  case '~': // consuming home directory
    char* home_dir = getenv("HOME");
    strcpy(dest, home_dir);
    (*cursor)++;
    return strlen(home_dir);
  default: // normal character
    *dest = *(*cursor)++;
    return 1;
  }
}

static bool is_delimeter(const char* cursor) {
  return *cursor == '\0' ||  *cursor == ' ' || *cursor == '>' || (*cursor == '1' && cursor[1] == '>') || (*cursor == '2' && cursor[1] == '>');
}

static bool literals_single_quote_open(char* dest, const char** cursor_in_raw_args, int* dest_cursor) {
  while (**cursor_in_raw_args != '\0' && **cursor_in_raw_args != '\'') {
    *dest++ = *(*cursor_in_raw_args)++;
    (*dest_cursor)++;
  }
  return *(*cursor_in_raw_args)++ == '\'';
}

static bool literals_double_quote_open(char* dest, const char** cursor_in_raw_args, int* dest_cursor) {
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

static bool is_output_redir(const char* cursor) { // "1>" or ">>"
  return *cursor == '>' || (*cursor == '1' && cursor[1] == '>');
}

static bool is_err_redir(const char* cursor) { // "2>"
  return *cursor == '2' && cursor[1] == '>';
};

static bool is_output_appd_redir(const char* cursor) { // ">>" or "1>>"
  return (*cursor == '>' && cursor[1] == '>') || (*cursor == '1' && cursor[1] == '>' && cursor[1] == '>');
};

static bool is_err_appd_redir(const char* cursor) { // "2>>"
  return *cursor == '2' && cursor[1] == '>' && cursor[1] == '>';
};