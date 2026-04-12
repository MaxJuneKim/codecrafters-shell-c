#pragma once

#include <stdio.h>

struct Argument {
  char** arguments;
  FILE** output_terminals;
  FILE** error_terminals;
};

// Parse raw input of string into each argument separated by space bar delimiter. In single quote, all characters will be treated literally
// expand special characters into actual value
extern struct Argument parse_args(const char* raw_args);

// free memory occupied by Argument
extern void free_arg(struct Argument args);