#pragma once

#include "types.h"

#define prev_commands_size 800

// Before the current session starts, retrieve previous commands from .my_shell_history file
extern void load_history_from_file();

// When user presses an up arrow, automatically load previous command into the given buffer
extern void load_prev_comm(char* buf);

// Automatically type forward command into the given buffer when user presses a down arrow,
extern void forward_comm(char* buf);

// When session ends, store the previous commands to ~/.my_shell_history
extern void store_history();

// array of poionters to constant characters.
extern char* historic_commands[prev_commands_size + 1];

/*
  Each type a command is entered, record it into historic_commands
*/
extern void add_to_history(const char* command);

/* 
  Returns a list of previous commands. Shows 40 by default
  Maybe overriden with n argument, where n is the number of commands to be shown
  Stores upto 800 previous commands. Use ring buffer when space runs out 
*/
extern struct Output write_history(char* n);