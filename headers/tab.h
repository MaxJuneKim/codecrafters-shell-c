#pragma once

#include <stdlib.h>
#include <stdbool.h>

extern void tab(char* input, size_t* cursor, bool* second_tab);

extern void init_matching_executables(size_t sz);

extern void init_matching_files();