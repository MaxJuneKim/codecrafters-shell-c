#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "echo.h"

char* echo(const char** arguments) {
  const char** str_cursor = arguments + 1;
  char* output = (char*)malloc(sizeof(char) * 2048);
  int index = 0;

  while (*str_cursor != NULL) {
    int n = strlen(*str_cursor);
    snprintf(output + index, n + 1, "%s", *str_cursor++);
    output[index + n] = ' ';
    index += (n + 1);
  }

  output[index++] = '\n';
  output[index] = '\0';
  return output;
}