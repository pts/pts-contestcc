#include "r_die.h"

#include <stdio.h>
#include <stdlib.h>

void die(const char *msg) {
  fprintf(stderr, "fatal: %s\n", msg);
  exit(1);
}
