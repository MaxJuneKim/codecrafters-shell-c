#include <stdlib.h>
#include "global_vars.h"

const char* const commands[total_commands] = {"echo", "type", "exit", "pwd", "cd"};
const char special_characters[2] = {'~', '\0'}; // TO ADD: $
char** ALL_EXECUTABLES = NULL;
size_t PATH_EXECUTABLES_COUNT = 0;