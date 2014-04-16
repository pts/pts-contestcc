#include "r_basic_formatters.h"

#include <stdio.h>
#include <string.h>

// Prepending `template<>' would prevent this from compiling.
void Formatter<char*>::format(Writable *wr, const char *msg) {
  wr->vi_write(msg, strlen(msg));
}

void Formatter<int>::format(Writable *wr, int v) {
  char tmp[sizeof(int) * 3 + 1];
  sprintf(tmp, "%d", v);  // TODO(pts): Faster.
  wr->vi_write(tmp, strlen(tmp));
}
