#pragma once 

extern void load_all_executables();

// Return the newly allocated string containing the full path of the command if found, or NULL if not found.
extern char* locate_bin(const char* argCommand);