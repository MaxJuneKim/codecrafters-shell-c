#pragma once

struct Argument {
  char** arguments;
  char** output_terminals;
};

// Parse raw input of string into each argument separated by space bar delimiter. In single quote, all characters will be treated literally
// expand special characters into actual value
extern struct Argument* parse_args(const char* raw_args);