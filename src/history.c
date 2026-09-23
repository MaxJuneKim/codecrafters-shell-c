#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <parse_arg.h>

#include "history.h"

char* historic_commands[prev_commands_size + 1] = { NULL };

static size_t add_cursor = 0;
static size_t offset = 0;
static size_t history_cursor = 0;
static size_t next_append_cursor = 0;

void load_prev_comm(char* buf) { 
  if (history_cursor == offset) {
    // do nothing
  } else if (history_cursor <= 0) {
    history_cursor = add_cursor < offset ? prev_commands_size : add_cursor - 1;
  } else {
    history_cursor--;
  }
  strcpy(buf, historic_commands[history_cursor] ? historic_commands[history_cursor]: "");
}

void forward_comm(char* buf) {
  if (history_cursor == add_cursor) {
    // do nothing
  } else if (history_cursor >= prev_commands_size) 
    history_cursor = 0;
  else 
    history_cursor++;
  strcpy(buf, historic_commands[history_cursor] ? historic_commands[history_cursor]: "");
}

void load_history_from_file(char* path_to_history_file) { 
  if (path_to_history_file == NULL) return;
  FILE* read = fopen(path_to_history_file, "r");
  size_t i = 0;
  char tmp_buf[1024];

  if (read == NULL) return;
  while (i++ < prev_commands_size && fgets(tmp_buf, 1024, read) != NULL) {
    size_t bytes = strlen(tmp_buf);

    historic_commands[add_cursor] = (char*)malloc(sizeof(char) * bytes);
    tmp_buf[bytes - 1] = '\0'; // truncating the new line
    strcpy(historic_commands[add_cursor], tmp_buf);

    if (add_cursor >= prev_commands_size) add_cursor = 0;
    else add_cursor++;
    free(historic_commands[add_cursor]);
  }

  history_cursor = add_cursor;
  fclose(read);
}

void store_history(char* path_to_history_file) {
  if (path_to_history_file == NULL) return;
  FILE* history = fopen(path_to_history_file, "w");
  if (history == NULL) return;

  size_t i = 0;
  size_t read_cursor = offset;
  while (i++ < prev_commands_size && historic_commands[read_cursor] != NULL) {
    fprintf(history, "%s\n", historic_commands[read_cursor]);
    if (read_cursor == prev_commands_size) read_cursor = 0;
    else read_cursor++;
  }
  fclose(history);
}

void append_history(char* path_to_history_file) {
  if (path_to_history_file == NULL) return;
  FILE* history = fopen(path_to_history_file, "a");
  if (history == NULL) return;

  size_t i = 0;
  size_t read_cursor = next_append_cursor;
  while (i++ < prev_commands_size && historic_commands[read_cursor] != NULL && read_cursor != add_cursor) {
    fprintf(history, "%s\n", historic_commands[read_cursor]);
    if (read_cursor == prev_commands_size) read_cursor = 0;
    else read_cursor++;
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
    historic_commands[add_cursor] = NULL;
    offset = add_cursor == prev_commands_size ? 0 : add_cursor + 1;
  }
  history_cursor = add_cursor;
  if (add_cursor == next_append_cursor) next_append_cursor = offset;
}

/*
  TODO: From users' perspective, it would make more sense that when argument is a number greater than capacity, the
  size opts to the maximum capacity but logic that decides whether the passed argument is even beyond the capacity of int
  or just the prev_commands_size can be complicated. Consider these cases:
  history 129837123891263: argument is numeric but way too big. 
    strlen(n) >= digits wouldn't work for the below case so we cannot use this
  history 123historyhistory: alphabets follow the string. It would make sense that size be 123 but
    additional alphabets add length that would prevent us from using strlen(n) to decide if user input is too big
  Consider using strtol to handle integer overflow
  For now, assume that user will pass relatively valid parameter. 
*/
struct Output history(char** arguments) {
  if (*arguments && strcmp(*arguments, "-r") == 0) {
    load_history_from_file(arguments[1]);
    return init_output();
  } else if (*arguments && strcmp(*arguments, "-a") == 0) {
    append_history(arguments[1]); 
    next_append_cursor = add_cursor;
    return init_output();
  } else if (*arguments && strcmp(*arguments, "-w") == 0) {
    store_history(arguments[1]);
    return init_output();
  }

  size_t size;
  if (*arguments == NULL) size = 40; // if argument is empty, stick to default size 40
  else size = atoi(*arguments);

  if (*arguments && **arguments == '0') return init_output();
  else if (*arguments && size == 0) size = 40; // if argument is invalid(0, alpha string), stick to default size 40
  else if (*arguments && size > prev_commands_size) size = prev_commands_size; // if argument is greater than capacity, opt to capacity size

  size_t read_cursor;
  if (offset > add_cursor && add_cursor == 0) read_cursor = prev_commands_size;
  else if (offset == 0 && add_cursor == 0) read_cursor = 0;
  else read_cursor = add_cursor - 1;

  size_t cap = add_cursor < offset ? prev_commands_size : add_cursor;
  size_t index = cap;
  size_t i = 0;
  size_t output_size = 0;

  while (i < size && historic_commands[read_cursor] != NULL) { // calculating size of the output
    output_size += strlen(historic_commands[read_cursor]) + 11;
    index--;
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
  read_cursor = read_cursor == prev_commands_size ? 0 : read_cursor + 1;

  // Actually writing to the result
  while (i > 0 && historic_commands[read_cursor] != NULL) {
    size_t sz = sprintf(result_cursor, "    %zu  %s\n", index + 1, historic_commands[read_cursor]);
    result_cursor += sz;
    index++;
    i--;
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