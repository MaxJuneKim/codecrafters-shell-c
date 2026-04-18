#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "tab.h"
#include "global_vars.h"
#include "sort_command.h"

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

void tab(char* input, char** matching_executables, size_t* cursor, bool* second_tab) {
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
    // TODO: Currnetly, this logic returns first match. Let's try to implement the behavior of actual shell, which is,
    // autocomplete only when there's one possible path. When there are multiple paths, show possible options
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
        *(input + (*cursor)++) = *cpy_cursor++;
      }
      *(input + (*cursor)++) = ' ';
    } else {
      fputc('\x07', stdout); // Does not move the cursor of stdout
    }
  }
}