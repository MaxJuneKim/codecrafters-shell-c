#pragma once

#define total_commands 6

extern const char* const built_in_commands[total_commands];
extern const char special_characters[2];
extern char** FILES_IN_CUR_DIR;
extern size_t FILES_COUNT;

// Sorted, array of all possible executable files in PATH directories
extern char** ALL_EXECUTABLES;
extern size_t PATH_EXECUTABLES_COUNT;