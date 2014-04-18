#include "r_tformatter_basic.h"

#include <stdio.h>

// Prepending `template<>' would prevent this from compiling.
void TFormatter<int32_t>::format(int32_t v, char *buf) {
  // Formats in decimal.
  char *p = buf, *q, c;
  if (v < 0) {
    *p++ = '-';
    v = -v;
  }
  q = p;
  do { *q++ = v % 10 + '0'; v /= 10; } while (v > 0);
  *q-- = '\0';
  while (p < q) {  // Reverse.
    c = *p;
    *p++ = *q;
    *q-- = c;
  }
}

void TFormatter<uint32_t>::format(uint32_t v, char *buf) {
  // Formats in decimal.
  char *p = buf, *q, c;
  q = p;
  do { *q++ = v % 10 + '0'; v /= 10; } while (v > 0);
  *q-- = '\0';
  while (p < q) {  // Reverse.
    c = *p;
    *p++ = *q;
    *q-- = c;
  }
}

void TFormatter<int64_t>::format(int64_t v, char *buf) {
  // Formats in decimal.
  if (static_cast<uint64_t>(v) >> 32 == 0) {  // Shortcut.
    return TFormatter<int32_t>::format(v, buf);
  }
  // Same code below as for int32_t, but v is larger.
  char *p = buf, *q, c;
  if (v < 0) {
    *p++ = '-';
    v = -v;
  }
  q = p;
  do { *q++ = v % 10 + '0'; v /= 10; } while (v > 0);
  *q-- = '\0';
  while (p < q) {  // Reverse.
    c = *p;
    *p++ = *q;
    *q-- = c;
  }
}

void TFormatter<uint64_t>::format(uint64_t v, char *buf) {
  // Formats in decimal.
  if (static_cast<uint64_t>(v) >> 32 == 0) {  // Shortcut.
    return TFormatter<int32_t>::format(v, buf);
  }
  // Same code below as for uint32_t, but v is larger.
  char *p = buf, *q, c;
  q = p;
  do { *q++ = v % 10 + '0'; v /= 10; } while (v > 0);
  *q-- = '\0';
  while (p < q) {  // Reverse.
    c = *p;
    *p++ = *q;
    *q-- = c;
  }
}
