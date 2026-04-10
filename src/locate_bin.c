#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "locate_bin.h"

// Legacy Code. Not deleted for Academic purpose of demonstrating using preprocessors to detect current operating system.
// REVISIT NOTE: The way I remember is, processors on different operating systems have different set of defined variables
// Windows preprocessor would have __WIN32 and __WIN64 defined but not MAC_OS or Linux preprocessor for example.
// This is how we can tell which operating system we are running on.

char* locate_bin(const char* argCommand) {
  // getting PATH variable
  #if defined(__WIN32) || defined(__WINT64)
    char* path_value = GetEnvironmentVariable("PATH");
  #elif defined(__linux__) || defined(__unix__) || defined(TARGET_OS_MAC)
    char* path_value = getenv("PATH");
  #endif

  char all_paths[strlen(path_value) + 1];
  strcpy(all_paths, path_value);

  // testing each path
  #if defined(__WIN32)
    char delimiter[2] = ";"
  #elif defined(__linux__) || defined(__unix__) || defined(TARGET_OS_MAC)
    char delimiter[2] = ":";
  #endif

  char* cur_path = strtok(all_paths, delimiter);
  char* cur_full_path = (char*)malloc(sizeof(char) * 1024);
  
  while (cur_path != NULL) {
    // check if the file exists and permission is executable
    strcpy(cur_full_path, cur_path);
    strcat(cur_full_path, "/");
    strcat(cur_full_path, argCommand);
    if (access(cur_full_path, X_OK) == 0) { 
      return cur_full_path;
    }
    cur_path = strtok(NULL, delimiter);
  }

  free(cur_full_path);
  return NULL;
}