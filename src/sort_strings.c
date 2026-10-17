#include <stdlib.h>
#include <string.h>

#include "sort_strings.h"

void sort_strings(char** strings, size_t lh, size_t rh) {
  if (lh + 1 >= rh) return;
  size_t mid = (lh + rh) / 2;
  sort_strings(strings, lh, mid);
  sort_strings(strings, mid, rh);

  char* temp_sort_commands_left[ mid - lh ];
  char* temp_sort_commands_right[ rh - mid ];

  for (size_t i = 0; i < mid - lh; i++) temp_sort_commands_left[i] = strings[lh + i];
  for (size_t i = 0; i < rh - mid; i++) temp_sort_commands_right[i] = strings[mid + i];

  size_t left_index = 0;
  size_t right_index = 0;
  size_t index = lh;
  while (left_index < mid - lh && right_index < rh - mid) {
    if (strcmp(temp_sort_commands_left[left_index], temp_sort_commands_right[right_index]) < 0) {
      strings[index++] = temp_sort_commands_left[left_index++];
    } else {
      strings[index++] = temp_sort_commands_right[right_index++];
    }
  }

  while (left_index < mid - lh) strings[index++] = temp_sort_commands_left[left_index++];
  while (right_index < rh - mid) strings[index++] = temp_sort_commands_right[right_index++];
}