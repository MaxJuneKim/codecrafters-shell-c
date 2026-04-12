#pragma once

#include "types.h"

// execute "type" command. if the argument to this command is a name of one of the built-in command, 
// print "{command} is a shell builtin". if not, it will print "{command}: not found"
extern struct Output executeType(const char* argCommand);