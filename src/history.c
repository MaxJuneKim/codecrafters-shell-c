#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "history.h"

char* historic_commands[prev_commands_size + 1] = { NULL };

static size_t add_cursor = 0;
static size_t offset = 0;

void load_history_from_file() { 
  char* home_dir = getenv("HOME");
  char history_full_path[60];
  snprintf(history_full_path, 60, "%s/%s", home_dir, ".my_shell_history");
  
  FILE* read = fopen(history_full_path, "r");
  size_t tmp_sz = 0;
  size_t i = 0;

  if (read == NULL) return;

  // while ((fgets(buffer, size, read))) != NULL) {
  while (i++ < prev_commands_size && getline(historic_commands + add_cursor, &tmp_sz, read) != -1) {
    historic_commands[add_cursor][strlen(historic_commands[add_cursor]) - 1] = '\0';
    add_cursor++;
  }
  fclose(read);
}

void store_history() {
  const char *home = getenv("HOME");
  char history_full_path[50];
  snprintf(history_full_path, 50, "%s/%s", home, ".my_shell_history");

  FILE* history = fopen(history_full_path, "w");
  if (history == NULL) return;

  size_t i = 0;
  size_t read_cursor = offset;
  while (i < prev_commands_size && historic_commands[read_cursor] != NULL) {
    if (read_cursor > prev_commands_size) 
      read_cursor = 0;
    fprintf(history, "%s\n", historic_commands[read_cursor++]);

    i++;
  }
  fclose(history);
}

void add_to_history(const char* command) {
  if (*command == '\0') return; // skip empty command
  historic_commands[add_cursor] = (char*)malloc(sizeof(char) * strlen(command) + 1);
  strcpy(historic_commands[add_cursor++], command);

  if (add_cursor > prev_commands_size) 
    add_cursor = 0;

  if (historic_commands[add_cursor] != NULL) {
    free(historic_commands[add_cursor]);
    historic_commands[add_cursor] = nullptr;
    offset = add_cursor + 1;
  }
}

struct Output write_history(size_t size) {
  size_t read_cursor = add_cursor == 0 ? prev_commands_size : add_cursor - 1;
  size_t i = 0;
  size_t output_size = 0;

  while (i < size && historic_commands[read_cursor] != NULL) { // calculating size of the output
    output_size += strlen(historic_commands[read_cursor]) + 11;
    i++;
    if (read_cursor == 0) {
      read_cursor = prev_commands_size;
    } else {
      read_cursor--;
    }
  }

  struct Output result = init_output();
  result.output = (char*)malloc(sizeof(char) * (output_size + 1));
  char* result_cursor = result.output;

  i = 0; // resetting cursors
  read_cursor = read_cursor == prev_commands_size ? 0 : read_cursor + 1;

  // Actually writing to the result
  while (i < size && historic_commands[read_cursor] != NULL) {
    size_t sz = sprintf(result_cursor, "    %zu  %s\n", i, historic_commands[read_cursor]);
    result_cursor += sz;
    i++;
    if (read_cursor == prev_commands_size) {
      read_cursor = 0;
    } else {
      read_cursor++;
    }
  }

  // *result_cursor = '\0';
  result.output[output_size] = '\0';
  return result;
}