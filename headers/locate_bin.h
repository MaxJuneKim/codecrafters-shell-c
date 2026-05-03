#pragma once 

// Traverse through all paths in PATH variable and locate all executable bin files into global string array, ALL_EXECUTABLES
extern void load_all_executables();

// Return the newly allocated string containing the full path of the command if found, or NULL if not found.
extern char* locate_bin(const char* argCommand);