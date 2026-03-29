#pragma once

// Parse raw input of string into each argument separated by space bar delimiter. In single quote, all characters will be treated literally
// expand special characters into actual value
extern char** parse_args(const char* raw_args);