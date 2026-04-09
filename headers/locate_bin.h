#pragma once 

// Return the newly allocated string containing the full path of the command if found, or NULL if not found.
extern char* locate_bin(const char* argCommand);