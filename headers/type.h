#pragma once

// execute "type" command. if the argument to this command is a name of one of the built-in command, 
// print "{command} is a shell builtin". if not, it will print "{command}: not found"
extern char* executeType(const char* argCommand);