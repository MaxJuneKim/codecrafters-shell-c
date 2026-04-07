#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

typedef struct quote {
  char* arg;
  struct quote* next;
} quote;

bool is_quote(char c) {
  char quotes[] = {'\'', '"'};
  int size = sizeof(quotes) / sizeof(quotes[0]);

  for (int i = 0; i < size; i++) {
    if (c == quotes[i]) {
      return true;
    }
  }
  return false;
}

char* commands[] = {"echo", "type", "exit", "pwd", "cd", "clear", NULL};

bool command_is_builtin(const char* command) {
  int i = 0;
  while (commands[i] != NULL) {
    if (strcmp(command, commands[i]) == 0) {
      return true;
    }
    i++;
  }
  return false;
}

void str_replace(char *str, char viejo, char nuevo) {
  while (*str != '\0') {
    if (*str == viejo) {
      *str = nuevo;
    }
    str++;
  }
}

char* get_command(char* input, int* length) {
  char* command = malloc(strlen(input));
  //
  int j = 0;
  char last_quote = '\0';
  bool quote_state = false;
  while ((input[*length] != ' ' && input[*length] != '\0' && input[*length] != '\n') || quote_state) {
    if (is_quote(input[*length]) && !quote_state) {
      quote_state = !quote_state;
      last_quote = input[*length];
      (*length)++;
      continue;
    }
    //
    if (is_quote(input[*length]) && input[*length] == last_quote) {
      quote_state = !quote_state;
      last_quote = '\0';
      (*length)++;
      continue;
    }
    //
    if (input[*length] == '\\' && !quote_state) {
      (*length)++;
      command[j] = input[*length];
      j++;
      continue;
    }
    //
    if (input[*length] == '\\' && last_quote == '"') {
      (*length)++;
    }
    //
    command[j] = input[*length];
    (*length)++;
    j++;
  }
  //
  command[j] = '\0';
  //
  return command;
}

char* get_args(char* input, int lenght_command) {
  int length = strlen(input) - lenght_command + 1;
  int final_position = strlen(input);
  //
  char* args = malloc(length + 1);
  //
  int j = 0;
  for (int i = lenght_command + 1; i < final_position; i++) {
    if (input[i] != '\n') {
      args[j] = input[i];
      j++;
    }
  }
  //
  args[j] = '\0';
  //
  return args;
}

int count_args(char* args) {
  int count = 0;
  if (strlen(args) == 0) {
    return 0;
  }
  //
  char *args_copy = strdup(args);
  //
  char *token = strtok(args_copy, " ");
  //
  while (token != NULL) {
    count++;
    token = strtok(NULL, " ");
  }
  //
  return count;
}

int count_quote_args(struct quote* args) {
  int count = 0;
  //
  while (args != NULL) {
    if (args->arg != NULL) {
      count++;
    }
    args = args->next;
  }
  //
  return count;
}

char* get_current_directory() {
  return getcwd(NULL, 0);
}

void clear() {
  printf("\033[H\033[2J\033[3J");
  fflush(stdout);
}

void print_echo(struct quote* queue) {
  while (queue != NULL) {
    struct quote* aux = queue;
    queue = aux->next;
    //
    if (aux->arg != NULL) {
      printf("%s ", aux->arg);
    }
  }
  printf("\n");
}

bool contains_any_operato(struct quote* args) {
  struct quote* aux = args;
  //
  while (aux != NULL) {
    if (strcmp(aux->arg, ">") == 0) {
      return true;
    }
    //
    if (strcmp(aux->arg, "1>") == 0) {
      return true;
    }
    //
    aux = aux->next;
  }
  //
  return false;
}

char* get_executable(char* command) {
  char* executable = NULL;
  char* path = getenv("PATH");
  char* path_copy = strdup(path);
  //
  char* dir = strtok(path_copy, ":");
  while (dir != NULL) {
    char full_path[1024];
    sprintf(full_path, "%s/%s", dir, command);
    //
    if (access(full_path, X_OK) == 0) {
      executable = malloc(strlen(full_path) + 1);
      strcpy(executable, full_path);
      free(path_copy);
      return executable;
    }
    //
    dir = strtok(NULL, ":");
  }
  //
  free(path_copy);
  return executable;
}

void execute(char* program, struct quote* args) {
  int count = count_quote_args(args) + 1;
  //
  char* exe[count + 1];
  exe[0] = program;
  //
  int i = 1;
  //
  struct quote* current = args;
  char* redirect_path = NULL;
  while (current != NULL && redirect_path == NULL) {
    if (strcmp(current->arg, ">") == 0 || strcmp(current->arg, "1>") == 0) {
      current = current->next;
      if (current != NULL) {
        redirect_path = current->arg;
      }
    } else {
      exe[i++] = current->arg;
    }
    current = current->next;
  }
  //
  exe[i] = NULL;
  //
  pid_t pid = fork();
  if (pid == 0) {
    dup2(STDOUT_FILENO, STDERR_FILENO);
    //
    if (redirect_path != NULL) {
      int fd = open(redirect_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
      if (fd < 0) {
        perror("open");
        _exit(1);
      }
      dup2(fd, STDOUT_FILENO);
      close(fd);
    }
    //
    execvp(exe[0], exe);
    //
    fflush(stderr);
  } else {
    wait(NULL); // Espera a que cualquier hijo termine
  }
  //
  while (args != NULL) {
    struct quote* tmp = args;
    args = args->next;
    free(tmp->arg);
    free(tmp);
  }
}

void handle_type(char* args) {
  if (command_is_builtin(args)) {
    printf("%s is a shell builtin\n", args);
    return;
  }
  //
  char* executable = get_executable(args);
  if (executable != NULL) {
    printf("%s is %s\n", args, executable);
    free(executable);
    return;
  }
  //
  printf("%s: not found\n", args);
}

void handle_absolute_path(const char* path) {
  if (strcmp(path, "~") == 0) {
    if (chdir(getenv("HOME")) == -1) {
      printf("error");
    }
    return;
  }
  //
  if (chdir(path) == -1) {
    printf("cd: %s: No such file or directory\n", path);
  }
}

struct quote* handle_args(char* args) {
  if (strlen(args) == 0) {
    return NULL;
  }
  //
  struct quote* queue = malloc(sizeof(quote));
  queue->arg = NULL;
  queue->next = NULL;
  //
  struct quote* aux_queue = queue;
  int length = strlen(args);
  char* aux = malloc(length + 1);
  //
  bool quote_state = false;
  bool last_was_space = false;
  char last_quote = '\0';
  int j = 0;
  for (int i = 0; i < length; i++) {
    if (is_quote(args[i])) {
      if (last_quote == '\0') {
        quote_state = !quote_state;
        last_quote = args[i];
        continue;
      }
      //
      if (last_quote == args[i]) {
        quote_state = !quote_state;
        last_quote = '\0';
        continue;
      }
    }
    //
    if (args[i] == ' ' && !quote_state) {
      if (!last_was_space && j > 0) {
        aux[j] = '\0';
        j = 0;
        aux_queue->arg = strdup(aux);
        aux_queue->next = malloc(sizeof(quote));
        aux_queue = aux_queue->next;
        aux_queue->arg = NULL;
        aux_queue->next = NULL;
        last_was_space = true;
      }
      continue;
    }
    //
    if (args[i] == '\\' && !quote_state) {
      i++;
      aux[j] = args[i];
      last_was_space = false;
      j++;
      continue;
    }
    //
    if (args[i] == '\\' && last_quote == '"') {
      i++;
    }
    //
    aux[j] = args[i];
    last_was_space = false;
    j++;

  }
  //
  if (j > 0) {
    aux[j] = '\0';
    aux_queue->arg = strdup(aux);
  }
  //
  free(aux);
  //
  return queue;
}

void write_file(char* str, char* path) {
  FILE *archivo;
  archivo = fopen(path, "w");
  //
  if (archivo == NULL) {
    return;
  }
  //
  fputs(str, archivo);
  fputs("\n", archivo);
  //
  fclose(archivo);
}

void handle_redirect_operator(struct quote* queue) {
  while (queue != NULL) {
    struct quote* aux = queue;
    queue = aux->next;
    //
    if (queue != NULL) {
      //
      if (strcmp(queue->arg, ">") == 0 || strcmp(queue->arg, "1>") == 0) {
        queue = queue->next;
        write_file(aux->arg, queue->arg);
        queue = aux->next;
      }
      //
    };
  }
}

bool run_command(char* command, char* args) {
  //
  if (strcmp(command, "type") == 0) {
    handle_type(args);
    return true;
  }
  //
  if (strcmp(command, "echo") == 0) {
    struct quote* queue = handle_args(args);
    //
    if (!contains_any_operato(queue)) {
      print_echo(queue);
    } else {
      handle_redirect_operator(queue);
    }
    //
    while (queue != NULL) {
      struct quote* aux = queue;
      queue = aux->next;
      free(aux->arg);
      free(aux);
    }
    //
    return true;
  }
  //
  if (strcmp(command, "exit") == 0) {
    free(args);
    free(command);
    exit(0);
  }
  //
  if (strcmp(command, "pwd") == 0) {
    char *directory = get_current_directory();
    printf("%s\n", directory);
    free(directory);
    return true;
  }
  //
  if (strcmp(command, "cd") == 0) {
    handle_absolute_path(args);
    return true;
  }
  //
  if (strcmp(command, "clear") == 0) {
    clear();
    return true;
  }
  //
  char* executable = get_executable(command);
  if (executable != NULL) {
    struct quote* queue = handle_args(args);
    execute(command, queue);
    free(executable);
    return true;
  }
  //
  return false;
}

void interpreter(char* input) {
  int lenght_command = 0;
  char* command = get_command(input, &lenght_command);
  char* args = get_args(input, lenght_command);
  //
  if (!run_command(command, args)) {
    printf("%s: command not found\n", command);
  }
  //
  free(args);
  free(command);
}

int main(int argc, char *argv[]) {
  setbuf(stdout, NULL);

  while (1) {
    printf("$ ");

    char input[1024];
    fgets(input, 1024, stdin);

    interpreter(input);

    fflush(stdout);
  }
  return 0;
}