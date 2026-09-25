#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "tab.h"
#include "global_vars.h"
#include "sort_command.h"

static void auto_complete_to_longest_common_prefix(char** matching_executables, char *input, size_t* cursor);
static void cmd_auto_complete(char* input, char** matching_executables, size_t* cursor, bool* second_tab);
static void file_auto_complete(char* input, char** matching_executables, size_t* input_cursor, bool* second_tab);

// tab function does not have ownership of any of the passed instance of these arguments, thus, should not free any of them
// TODO: current implementation of command tab assumes that the command will not have space character in them and thus need not be enclosed in quotes
// Try to support advanced auto completion for these kinds of commands. Same for file auto completion
void tab(char* input, char** matching_executables, size_t* cursor, bool* second_tab) {
  size_t i = 0;
  while (i < *cursor) {
    if (input[i] == ' ') break;
    else i++;
  }

  if (i < *cursor) file_auto_complete(input, matching_executables, cursor, second_tab);
  else cmd_auto_complete(input, matching_executables, cursor, second_tab);
}

static void cmd_auto_complete(char* input, char** matching_executables, size_t* cursor, bool* second_tab) {
  input[*cursor] = '\0'; // temporarily place null terminating character for strcmp
  if (strcmp(input, "ech") == 0) { // 
    input[(*cursor)++] = 'o';
    input[(*cursor)++] = ' ';
    printf("%c ", 'o');
    *second_tab = false;
  } else if (strcmp(input, "exi") == 0) {
    input[(*cursor)++] = 't';
    input[(*cursor)++] = ' ';
    printf("%c ", 't');
    *second_tab = false;
  } else if (*second_tab) { // second tab
    // TODO: This is a bit of challenge but, let's try using Trie for performance in the future for performance improvement
    if (*matching_executables) { // There are some matching executables
      fputc('\n', stdout);
      for (size_t i = 0; matching_executables[i] != NULL; i++) {
        printf("%s  ", matching_executables[i]);
      }
      printf("\n$ %s", input);
    } else {
      fputc('\x07', stdout);
    }
  } else { // first tab
    char** executable = ALL_EXECUTABLES; // cursor for ALL_EXECUTABLES global variable
    size_t matching_count = 0;
    while (*executable) { // for each binary in ALL_EXECUTABLES
      if (strncmp(*executable, input, *cursor) == 0) { // Match found
        matching_executables[matching_count++] = *executable; // *Shallow copy*. 
      }
      executable++;
    }
    matching_executables[matching_count] = NULL;
    if (matching_count > 1) { // multiple matches, prepare for second tab
      fputc('\x07', stdout); // Does not move the cursor of stdout
      *second_tab = true;
      auto_complete_to_longest_common_prefix(matching_executables, input, cursor);
    } else if (matching_count == 1) { // Single match, autocomplete to the match
      printf("%s ", *matching_executables + *cursor); // printing the rest of the name for the binary
      char* cpy_cursor = *matching_executables + *cursor;
      while (*cpy_cursor != '\0') { // copying to the input also
        // *(input + (*cursor)++) = *cpy_cursor++;
        input[(*cursor)++] = *cpy_cursor++;
      }
      *(input + (*cursor)++) = ' ';
    } else {
      fputc('\x07', stdout); // Does not move the cursor of stdout
    }
  }
}

// TODO: support autocompletion of files that have a space character(s) in them
static void file_auto_complete(char* input, char** matching_executables, size_t* input_cursor, bool* second_tab) {
  input[*input_cursor] = '\0'; // temporarily place null terminating character for strcmp
  // Going back to the start of the current argument. 
  // Edge case: What if user opened with quotes? What if user already opened and closed quotes in the current argument? 
  // ex) cmd "this is my...    or    cmd hello"this"is...
  // Edge case: what if current character is a space character? In other words, what if current argument hasn't started?
  size_t cursor = *input_cursor;
  while (input[cursor] != ' ') cursor--;
  cursor += 1;

  // find matching files
  // char** matching_files = (char**)malloc(sizeof(char*) * FILES_COUNT);
  size_t prefix_sz = *input_cursor - cursor;
  for (size_t i = 0; i < FILES_COUNT; i++) {
    if (strncmp(input + cursor, FILES_IN_CUR_DIR[i], prefix_sz) == 0) {
      printf("%s ", FILES_IN_CUR_DIR[i] + prefix_sz);

      size_t file_cursor = prefix_sz;
      while (FILES_IN_CUR_DIR[i][file_cursor] != '\0') {
        input[(*input_cursor)++] = FILES_IN_CUR_DIR[i][file_cursor++];
      }
      input[(*input_cursor)++] = ' ';
      break;
    }
  }
}

// When user presses tab and there are multiple matches, auto-complete to longest common prefixes of those matching executables
static void auto_complete_to_longest_common_prefix(char** matching_executables, char *input, size_t* cursor) {
  // matching_executables should not be modified here
  if (*matching_executables == NULL) return; // empty strings

  while (true) { // For each index
    size_t j = 0;
    for (; matching_executables[j + 1]; j++) { // for each string
      if (matching_executables[j][*cursor] == '\0' || 
          matching_executables[j + 1][*cursor] == '\0' || 
          matching_executables[j][*cursor] != matching_executables[j + 1][*cursor]) {
        return;
      }
    }
    fputc(matching_executables[j][*cursor], stdout);
    input[*cursor] = matching_executables[j][*cursor];
    (*cursor)++;
  }
}