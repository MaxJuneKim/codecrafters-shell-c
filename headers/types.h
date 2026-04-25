#pragma once

struct Output {
  char* output;
  char* error;
};

extern struct Output init_output();