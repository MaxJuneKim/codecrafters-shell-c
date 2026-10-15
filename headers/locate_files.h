#pragma once

// Let files in the specified directory and put them into the dest pointer(array of strings)
// It returns the number of files in dir. It should not free dest
extern size_t get_files_in_dir(char** dest, const char* const dir_path);