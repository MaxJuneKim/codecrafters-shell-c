#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int main(int argc, char *argv[]) {
  // Flush after every printf
  setbuf(stdout, NULL);

  char command[1024];

  // TODO: Uncomment the code below to pass the first stage
  printf("$ ");
  fgets(command, sizeof(command), stdin);

  bool validCommand = false;
  if (validCommand) {}
  else {
    printf("%s: command not found\n", command);
  }

  return 0;
}
