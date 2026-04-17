#include <stdlib.h>
#include <string.h>

#include "sort_command.h"

void sort_commands(char** commands, size_t lh, size_t rh) {
  if (lh + 1 >= rh) return;
  size_t mid = (lh + rh) / 2;
  sort_commands(commands, lh, mid);
  sort_commands(commands, mid, rh);

  char* temp_sort_commands_left[ mid - lh ];
  char* temp_sort_commands_right[ rh - mid ];

  for (size_t i = 0; i < mid - lh; i++) temp_sort_commands_left[i] = commands[lh + i];
  for (size_t i = 0; i < rh - mid; i++) temp_sort_commands_right[i] = commands[mid + i];

  size_t left_index = 0;
  size_t right_index = 0;
  size_t index = lh;
  while (left_index < mid - lh && right_index < rh - mid) {
    if (strcmp(temp_sort_commands_left[left_index], temp_sort_commands_right[right_index]) < 0) {
      commands[index++] = temp_sort_commands_left[left_index++];
    } else {
      commands[index++] = temp_sort_commands_right[right_index++];
    }
  }

  while (left_index < mid - lh) commands[index++] = temp_sort_commands_left[left_index++];
  while (right_index < rh - mid) commands[index++] = temp_sort_commands_right[right_index++];
}