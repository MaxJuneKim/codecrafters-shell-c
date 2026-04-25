#include <stdlib.h>

#include "types.h"

struct Output init_output() {
  struct Output output;
  output.output = NULL;
  output.error = NULL;
  return output;
}