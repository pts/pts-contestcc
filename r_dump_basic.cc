#include "r_dump_basic.h"

#include <inttypes.h>

#include "r_die.h"
#include "r_dump.h"

namespace r {

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

static void wrdump_piece(char const *v_data, uintptr_t v_size,
                         std::string *out) {
  uintptr_t len = 2 + v_size;
  const char * const q = v_data + v_size;

  // Precalculate length to append.
  for (const char *r = v_data; r != q; ++r) {
    const char v = *r;
    if (v - 32U < 127 - 32U) {
      if (v == '"' || v == '\\') ++len;
    } else {
      len += (v == '\t' || v == '\n' || v == '\r' || v == '\f') ? 1 : 3;
    }
  }

  uintptr_t size = out->size();
  out->resize(size + len);
  char *p = &(*out)[size];
  *p++ = '"';
  for (const char *r = v_data; r != q; ++r) {
    const char v = *r;
    if (v - 32U < 127 - 32U) {
      if (v == '"' || v == '\\') *p++ = '\\';
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
  }
  *p++ = '"';
  if (p != len + &(*out)[size]) die("assert: Wrong string dump len.");
}

void wrdump_low(const char *v, uintptr_t size, std::string *out) {
  if (size > 0 && v[size - 1] == '\0') {
    wrdump_piece(v, size - 1, out);
  } else {
    // This would print decimal numbers rather than character codes.
    // wrdump_ary(reinterpret_cast<const uint8_t*>(v), size, out);
    wrdump_ary(v, size, out);
  }
}

void wrdump_low(const StrPiece &v, std::string *out) {
  out->append("StrPiece(");
  wrdump_piece(v.data, v.size, out);
  out->append(", ");
  wrdump(v.size, out);
  out->push_back(')');
}

void wrdump_low(const std::string &v, std::string *out) {
  const bool is_wrapped = v.find('\0') != std::string::npos;
  if (is_wrapped) out->append("std::string(");
  wrdump_piece(v.data(), v.size(), out);
  if (is_wrapped) {
    out->append(", ");
    wrdump(v.size(), out);
    out->push_back(')');
  }
}

void wrdump_low(const char *v, std::string *out) {
  return wrdump_piece(v, strlen(v), out);
}

}  // namespace r
