#include "r_ldblgood.h"
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

void TFormatter<double>::format(double v, char *buf) {
  // Assuming IEEE 754 64-bit double.
  struct AssertDoubleSizeIs8 { int DoubleSizeIs8 : sizeof(v) == 8; };
  char *p;
  int c;
  sprintf(buf, "%.17g", v);
  for (c = 0, p = buf; *p && *p != 'e'; ++p) {
    c += *p - '0' + 0U <= 9U;
  }
  /* Used all digits, e.g. "1.2300000000000001e+202", "0.3333333333333333" */
  if (c >= 17) {  /* Can be 17 or 18. */
    char tmp[max_buf_size];
    double d16;
    sprintf(tmp, "%.16g", v);
    if (1 == sscanf(tmp, "%lf", &d16) && v == d16) {
      strcpy(buf, tmp);
    }
  }
}

void TFormatter<float>::format(float v, char *buf) {
  // Assuming IEEE 754 32-bit float.
  struct AssertFloatSizeIs4 { int FloatSizeIs4 : sizeof(v) == 4; };
  char *p;
  int c;
  sprintf(buf, "%.9g", v);
  for (c = 0, p = buf; *p && *p != 'e'; ++p) {
    c += *p - '0' + 0U <= 9U;
  }
  if (c >= 9) {
    char tmp[max_buf_size];
    float d8;
    sprintf(tmp, "%.7g", v);  /* For 34.56, 7 digits is enough. */
    if (1 == sscanf(tmp, "%f", &d8) && v == d8) {
      strcpy(buf, tmp);
    } else {  // TODO(pts): Do we need to do 9, or only 8 and 7?
      sprintf(tmp, "%.8g", v);
      if (1 == sscanf(tmp, "%f", &d8) && v == d8) {
        strcpy(buf, tmp);
      }
    }
  }
}

#ifdef LDBL_GOOD
void TFormatter<long double>::format(long double v, char *buf) {
  // long double on GCC Linux, useful bits are 80. Aligned to 12 bytes on
  // i386 and 16 bytes on x86_64.
  struct AssertLongDoubleSizeIs12 {
      int LongDoubleSizeIs12 : sizeof(v) == 12 || sizeof(v) == 16; };
  char *p;
  int c;
  LDBL_GOOD(sprintf)(buf, "%.19Lg", v);
  for (c = 0, p = buf; *p && *p != 'e'; ++p) {
    c += *p - '0' + 0U <= 9U;
  }
  if (c >= 9) {
    char tmp[max_buf_size];
    long double d8;
    LDBL_GOOD(sprintf)(tmp, "%.18Lg", v);
    // Old (Ubuntu Lucid 4.2.1-sjlj) MinGW versions don't have __mingw_sscanf,
    // so this won't compile.
    // TODO(pts): Implement sscanf for ldbl, based on newer MinGW sources.
    if (1 == LDBL_GOOD(sscanf)(tmp, "%Lf", &d8) && v == d8) {
      strcpy(buf, tmp);
    }
  }
}
#endif
