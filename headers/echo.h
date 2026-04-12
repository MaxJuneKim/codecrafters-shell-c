#pragma once

#include "types.h"

// Returns a malloc'd string that concatenated all the substrings in the arguments parameter.
extern struct Output echo(const char** arguments);