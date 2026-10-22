#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "tab.h"
#include "locate_files.h"
#include "global_vars.h"
#include "sort_strings.h"

static char** matching_executables;
static char** matching_files;

static void auto_complete_to_longest_common_prefix(char** matching_executables, char *input, size_t* cursor);
static void auto_complete_to_longest_common_prefix_files(char** matching_files, char *input, size_t* input_cursor, size_t cursor);
static void cmd_auto_complete(char* input, size_t* cursor, bool* second_tab);
static void file_auto_complete(char* input, char* file_path, size_t* input_cursor, bool* second_tab);

void init_matching_executables(size_t sz) {
  matching_executables = (char**)malloc(sizeof(char*) * sz);
  for (size_t i = 0; i < sz; i++) 
    matching_executables[i] = NULL; 
}

void init_matching_files() {
  matching_files = (char**)malloc(sizeof(char*) * 256);
  for (size_t i = 0; i < 256; i++) 
    matching_files[i] = NULL; 
}

// tab function does not have ownership of any of the passed instance of these arguments, thus, should not free any of them
// TODO: current implementation of command tab assumes that the command will not have space character in them and thus need not be enclosed in quotes
// Try to support advanced auto completion for these kinds of commands. Same for file auto completion
void tab(char* input, size_t* cursor, bool* second_tab) {
  input[*cursor] = '\0'; // temporarily place null terminating character for strcmp
  char* str = strrchr(input, ' '); 
  if (str != NULL) file_auto_complete(input, str + 1, cursor, second_tab);
  else cmd_auto_complete(input, cursor, second_tab);
}

// TODO: More general logic for completing builtin commands
static void cmd_auto_complete(char* input, size_t* cursor, bool* second_tab) {
  // input[*cursor] = '\0'; 
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
static void file_auto_complete(char* input, char* file_path, size_t* input_cursor, bool* second_tab) {
  // Edge case: What if user opened with quotes? What if user already opened and closed quotes in the current argument? 
  // ex) cmd "this is my...    or    cmd hello"this"is...
  // Edge case: what if current character is a space character? In other words, what if current argument hasn't started?
  if (file_path == NULL) return;
  if (*second_tab) {
    char** matching_cursor = matching_files;
    fputc('\n', stdout);
    while (*matching_cursor) {
      printf("%s  ", *matching_cursor);
      matching_cursor++;
    }
    printf("\n$ %s", input);
    return;
  }
  for (char** cursor = matching_files; *cursor != NULL; cursor++) // freeing memory
    free(*cursor);

  char* last_slash = strrchr(file_path, '/');
  char* prefix = last_slash == NULL ? file_path : last_slash + 1; 

  DIR *dir;
  if (last_slash == NULL) {
    dir = opendir(".");
  } else {
    size_t dir_name_sz = prefix - file_path + 1;
    char dir_name[dir_name_sz]; // getting current directory name
    dir_name[dir_name_sz - 1] = '\0';
    for (size_t i = 0; i < dir_name_sz - 1; i++)
      dir_name[i] = file_path[i];
    dir = opendir(dir_name);
  }
  if (!dir) return;

  // find matching files
  size_t prefix_sz = strlen(prefix);
  size_t matching_count = 0;
  struct dirent *entry;

  while ((entry = readdir(dir)) != NULL) { // for each file in directory
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
    if (strncmp(prefix, entry->d_name, prefix_sz) == 0) {
      size_t len = strlen(entry->d_name) + (entry->d_type == DT_DIR ? 2 : 1);
      char* match_file = (char*)malloc(sizeof(char) * len);
      strcpy(match_file, entry->d_name);
      if (entry->d_type == DT_DIR) 
        match_file[len - 2] = '/';
      match_file[len - 1] = '\0';
      matching_files[matching_count++] = match_file;
    }
  }
  sort_strings(matching_files, 0, matching_count);
  matching_files[matching_count] = NULL;

  if (matching_count == 0) {
    fputc('\x07', stdout); // no matching file
  } else if (matching_count > 1) { // multiple matching files
    *second_tab = true;
    fputc('\x07', stdout);
    auto_complete_to_longest_common_prefix_files(matching_files, input, input_cursor, prefix_sz);
  } else if (matching_count == 1) { // 1 matching file
    size_t file_cursor = prefix_sz;
    while (matching_files[0][file_cursor] != '\0') {
      prefix[file_cursor] = matching_files[0][file_cursor];
      file_cursor++;
      *(input_cursor) += 1;
    }
    if (matching_files[0][file_cursor - 1] == '/') {
      printf("%s", matching_files[0] + prefix_sz);
    } else {
      *(input_cursor) += 1;
      prefix[file_cursor] = ' ';
      printf("%s ", matching_files[0] + prefix_sz);
    }
  }
  closedir(dir);
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

// When user presses tab and there are multiple matches, auto-complete to longest common prefixes of those matching files
// TODO: merge this code with auto_complete_to_longest_common_prefix to create more general logic that covers both commands and files auto-completion
static void auto_complete_to_longest_common_prefix_files(char** matching_files, char *input, size_t* input_cursor, size_t cursor) {
  if (*matching_files == NULL) return;

  while (true) {
    size_t j = 0;
    for (; matching_files[j + 1]; j++) { // for each string
      if (matching_files[j][cursor] == '\0' || 
          matching_files[j + 1][cursor] == '\0' || 
          matching_files[j][cursor] != matching_files[j + 1][cursor]) {
        return;
      }
    }
    fputc(matching_files[j][cursor], stdout);
    input[*input_cursor] = matching_files[j][cursor];
    (*input_cursor)++;
    cursor++;
  }
}