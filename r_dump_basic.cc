#include "r_dump_basic.h"

#include <inttypes.h>

void wrdump_low(char v, std::string *out) {
  char buf[6], *p = buf;
  *p++ = '\'';
  if (v - 32U < 127 - 32U) {
    if (v == '\'' || v == '\\') *p++ = '\\';
    *p++ = v;
  } else {
    *p++ = '\\';
    // These single-character sequences are compatible with Python and Java
    // as well, in addition to all old and new C and C++.
    if (v == '\t') {
      *p++ = 't';
    } else if (v == '\n') {
      *p++ = 'n';
    } else if (v == '\r') {
      *p++ = 'r';
    } else if (v == '\f') {
      *p++ = 'f';
    } else {
      unsigned u = (uint8_t)v;
      *p++ = '0' + (u >> 6);
      *p++ = '0' + ((u >> 3) & 7);
      *p++ = '0' + (u & 7);
    }
  }
  *p++ = '\'';
  out->append(buf, p -buf);
}
