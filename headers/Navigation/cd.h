#pragma once

#include "types.h"

// change directory. Output malloc'd message string if the directory does not exist
extern struct Output cd(const char* directory);