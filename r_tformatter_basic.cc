#include "r_tformatter_basic.h"

#include <stdio.h>

// Prepending `template<>' would prevent this from compiling.
void TFormatter<int>::format(int v, char *buf) {
  sprintf(buf, "%d", v);  // TODO(pts): Faster.
}
