#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "cat.h"

void cat(char* file_name) {
  FILE* file_ptr = fopen(file_name, "r");
  if (!file_ptr)
    return;

  char* output_buffer = (char*)malloc(sizeof(char) * 2048);
  while (fgets(output_buffer, 2047, file_ptr) != NULL) {
    printf("%s", output_buffer);
  }
}