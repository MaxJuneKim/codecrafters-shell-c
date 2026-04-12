#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "echo.h"

struct Output echo(const char** arguments) {
  const char** str_cursor = arguments + 1;
  struct Output output;
  output.output = (char*)malloc(sizeof(char) * 2048);
  output.error = NULL;
  int index = 0;

  while (*str_cursor != NULL) {
    int n = strlen(*str_cursor);
    snprintf(output.output + index, n + 1, "%s", *str_cursor++);
    output.output[index + n] = ' ';
    index += (n + 1);
  }

  output.output[index - 1] = '\n';
  output.output[index] = '\0';
  return output;
}