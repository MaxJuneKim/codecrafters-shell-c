#pragma once

#define total_commands 5

extern const char* const commands[total_commands];
extern const char special_characters[2];

// Sorted, array of all possible executable files in PATH directories
extern char** ALL_EXECUTABLES;
extern size_t PATH_EXECUTABLES_COUNT;