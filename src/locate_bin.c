#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(__WIN32)
  #include <Windows.h>
#endif
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#include "locate_bin.h"
#include "global_vars.h"

// REVISIT NOTE: The way I remember is, processors on different operating systems have different set of defined variables
// Windows preprocessor would have __WIN32 and __WIN64 defined but not MAC_OS or Linux preprocessor for example.
// This is how we can tell which operating system we are running on.

// Helper to check if a file is actually executable
int is_executable(const char *path) {
  struct stat st;
  if (stat(path, &st) == 0 && (st.st_mode & S_IXUSR)) {
      return 1;
  }
  return 0;
}

void load_all_executables() { // pre-load all binaries when this program loads?
  size_t capacity = 1024;
  char** binaries = (char**)malloc(sizeof(char*) * capacity);
  #if defined(__WIN32)
    char* path_env = GetEnvironmentVariable("PATH");
    char delimeter[2] = ";"
  #elif defined(__linux__) || defined(__unix__) || defined(TARGET_OS_MAC)
    char* path_env = getenv("PATH");
    char delimeter[2] = ":";
  #endif
  
  // TODO: This is a bit of challenge but, let's try using Trie for performance in the future.
  size_t count = 0;
  char* all_paths = strdup(path_env);
  char* cur_path = strtok(all_paths, delimeter);
  while (cur_path) {
    char full_path[256];
    DIR *dir = opendir(cur_path); // get directory
    if (dir) {
      struct dirent *entry;
      while ((entry = readdir(dir)) != NULL) { // for each file in the directory
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        snprintf(full_path, sizeof(full_path), "%s/%s", cur_path, entry->d_name);
        if (is_executable(full_path)) {
          binaries[count++] = strdup(entry->d_name);
          if (count >= capacity) { // resizing the array
            capacity *= 2;
            char** temp = binaries;
            binaries = (char**)malloc(sizeof(char*) * capacity);
            for (size_t i = 0; i < count; i++) binaries[i] = temp[i];
          }
        }
      }
      closedir(dir);
    }
    cur_path = strtok(NULL, delimeter);
  }

  all_executables = (char**)malloc(sizeof(char*) * (count + 1));
  for (size_t i = 0; i < count; i++) // copying to the global variables
    all_executables[i] = binaries[i];
  all_executables[count] = NULL;

  // cleanup
  free(binaries);
  free(all_paths);
}

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
  char* cur_full_path = (char*)malloc(sizeof(char) * 256);
  
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