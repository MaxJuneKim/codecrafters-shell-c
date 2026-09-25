#include <stdlib.h>
#include "global_vars.h"

const char* const built_in_commands[total_commands] = {"echo", "type", "exit", "pwd", "cd", "history"};
const char special_characters[2] = {'~', '\0'}; // TO ADD: $

char** FILES_IN_CUR_DIR = NULL;
size_t FILES_COUNT = 0;

char** ALL_EXECUTABLES = NULL;
size_t PATH_EXECUTABLES_COUNT = 0;