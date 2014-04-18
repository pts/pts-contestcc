// C++ disadvantage: memory errors (e.g. segfault), no array bounds checking
// C++ disadvantage: no stack trace
// C++ disadvantage: no unliminted ints by default
// C++ disadvantage: no easy int overflow detection
// C++ disadvantage: uninitialized variables (there is a warning)
// C++ disadvantage: no automatic memory management (not a big problem)
// C++ disadvantage: no regexps (not a problem most of the time)
// C++ disadvantage: printf on long double doesn't work on Windows (MinGW).

#include "r_status.h"
#include "r_fileobj.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <string>

// For eGlibc, math.h defines this, for uClibc, it doesn't.
// #define NAN (__builtin_nanf (""))

template <typename T, uintptr_t S>
inline unsigned arraysize(const T (&)[S]) { return S; }

static inline bool is_whitespace(int c) {
  return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

static inline bool is_digit(int c) {
  return c - '0' + 0U <= 9;
}

Status read_word(FILE *f, std::string *out) {
  int c;
  while (is_whitespace(c = getc(f))) {}
  if (c < 0) return "EOF when reading word.";
  out->clear();
  out->push_back(c);
  for (; !is_whitespace(c = getc(f)); out->push_back(c)) {}
  if (c >= 0) ungetc(c, f);
  return true;
}

std::string read_word(FILE *f) {
  std::string result;
  read_word(f, &result);  // die(...)s on error.
  return result;
}

// Ignores optional whitespace in front of the number.
//
// TODO(pts): Implement unsigned version.
Status read_dec(FILE *f, int nbytes, int64_t *out) {
  bool is_neg = false;
  int c;
  while ((c = getc(f)) >= 0 && is_whitespace(c)) {}
  if (c == '-') {
    is_neg = true;
    c = getc(f);
  }
  if (!is_digit(c)) {
    if (c >= 0) ungetc(c, f);
    return "EOF when reading signed decimal.";
  }
  uint64_t n = c - '0';
  const uint64_t limit = ((uint64_t)1 << (8 * nbytes - 1)) - 1 + is_neg;
  while (is_digit(c = getc(f))) {
    const uint64_t n10 = 10 * n;
    const uint64_t n10c = n10 + c - '0';
    // TODO(pts): Use faster implementation if nbytes < sizeof(int64_t).
    if (n10 / 10 != n || n10c < n10 || n10c > limit) {
      return "Out of bounds when reading signed decimal.";
    }
    n = n10c;
  }
  if (c >= 0) ungetc(c, f);
  *out = is_neg ? -n : n;
  return true;
}

Status read_dec(FILE *f, int64_t *out) { return read_dec(f, 8, out); }
Status read_dec(FILE *f, int32_t *out) {
  int64_t i64; Status s = read_dec(f, 4, &i64); *out = i64; return s;
}
Status read_dec(FILE *f, int16_t *out) {
  int64_t i64; Status s = read_dec(f, 2, &i64); *out = i64; return s;
}
Status read_dec(FILE *f, int8_t  *out) {
  int64_t i64; Status s = read_dec(f, 1, &i64); *out = i64; return s;
}

class DecReader {
 public:
  DecReader(FILE *f): f_(f) {}
  operator int8_t() {
    int64_t i64; read_dec(f_, 1, &i64); return i64;
  }
  operator int16_t() {
    int64_t i64; read_dec(f_, 2, &i64); return i64;
  }
  operator int32_t() {
    int64_t i64; read_dec(f_, 4, &i64); return i64;
  }
  operator int64_t() {
    int64_t i64; read_dec(f_, 8, &i64); return i64;
  }
 private:
  FILE *f_;
};

DecReader read_dec(FILE *f) { return DecReader(f); }

// ---

class DecInI8 {
 public:
  DecInI8(int8_t *p): p_(notnull(p)) {}
  int8_t *get() const { return p_; }
 private:
  int8_t *p_;
};
class DecInI16 {
 public:
  DecInI16(int16_t *p): p_(notnull(p)) {}
  int16_t *get() const { return p_; }
 private:
  int16_t *p_;
};

static inline const FileObj &operator>>(const FileObj &f, const DecInI8 &out) {
  read_dec(f.f(), out.get());
  return f;
}
static inline const FileObj &operator>>(const FileObj &f, const DecInI16 &out) {
  read_dec(f.f(), out.get());
  return f;
}

DecInI8  dec(int8_t  *p) { return p; }
DecInI16 dec(int16_t *p) { return p; }

Status read_literal(FILE *f, const char *msg, uintptr_t size) {
  for (; size > 0; ++msg, --size) {
    int c = getc(f);
    // TODO(pts): Report what literal (msg0).
    if (c < 0) return "EOF when reading literal.";
    if ((char)c != msg[0]) {
      ungetc(c, f);
      return "Unexpected input when reading literal.";
    }
  }
  return true;
}

class LiteralIn {
 public:
  LiteralIn(const char *msg, uintptr_t size): msg_(msg), size_(size) {}
  LiteralIn(const char *msg): msg_(msg), size_(strlen(msg)) {}
  Status read(FILE *f) const { return read_literal(f, msg_, size_); }
 private:
  const char * const msg_;  // Owned externally.
  const uintptr_t size_;
};

// TODO(pts): Move literal, dec etc. to a namespace.
// TODO(pts): Add line(&str);
LiteralIn literal(const char *msg) { return msg; }
LiteralIn literal(const char *msg, uintptr_t size) {
  return LiteralIn(msg, size);
}

static inline const FileObj &operator>>(const FileObj &f, const LiteralIn &in) {
  in.read(f.f());
  return f;
}

static inline const FileObj &operator>>(const FileObj &f, const char *in) {
  LiteralIn(in).read(f.f());
  return f;
}

// Check that next character is not a whitespace.
Status peek_nows(FILE *f) {
  const int c = getc(f);
  if (c >= 0) {
    const bool is_ws = is_whitespace(c);
    ungetc(c, f);
    if (is_ws) return "Unexpected whitespace.";
  }
  return true;
}

class NowsIn {};
extern NowsIn nows;

static inline const FileObj &operator>>(const FileObj &f, NowsIn) {
  peek_nows(f.f()); return f;
}

// Check that next character is not a whitespace.
Status peek_eof(FILE *f) {
  const int c = getc(f);
  if (c >= 0) {
    ungetc(c, f);
    return "Expected EOF.";
  }
  return true;
}

class EofIn {};
extern EofIn eof;

static inline const FileObj &operator>>(const FileObj &f, EofIn) {
  peek_eof(f.f()); return f;
}

// TODO(pts): Added reading bool (can't ungetc fully).

// --- Output.

class Io {};
extern Io io;

FileObj sin(stdin);
// Any of these work:
//
//   sout << "Hello\n";
//   fflush(sout);
FileObj sout(stdout);
FileObj serr(stderr);

// TODO(pts): Implement write_hex etc.

// io << stdout << "Hello!\n";
static inline FileObj operator<<(Io, FILE *f) { return f; }
// io >> stdin >> literal(",");
static inline FileObj operator>>(Io, FILE *f) { return f; }
static inline const FileObj &operator<<(Io, const FileObj &fo) { return fo; }
static inline const FileObj &operator<<(const FileObj &fo, const char *msg) {
  fo.write(msg);
  return fo;
}
static inline const FileObj &operator<<(const FileObj &fo, const std::string &str) {
  fo.write(str.data(), str.size());
  return fo;
}
static inline const FileObj &operator<<(const FileObj &fo, bool v) {
  fo.write(v ? "true" : "false");
  return fo;
}
static inline const FileObj &operator<<(const FileObj &fo, int8_t v) {
  write_dec(&fo, (int64_t)v);
  return fo;
}
static inline const FileObj &operator<<(const FileObj &fo, int16_t v) {
  write_dec(&fo, (int64_t)v);
  return fo;
}
static inline const FileObj &operator<<(const FileObj &fo, int32_t v) {
  write_dec(&fo, (int64_t)v);
  return fo;
}
static inline const FileObj &operator<<(const FileObj &fo, int64_t v) {
  write_dec(&fo, v);
  return fo;
}
static inline const FileObj &operator<<(const FileObj &fo, uint8_t v) {
  write_dec(&fo, (uint64_t)v);
  return fo;
}
static inline const FileObj &operator<<(const FileObj &fo, uint16_t v) {
  write_dec(&fo, (uint64_t)v);
  return fo;
}
static inline const FileObj &operator<<(const FileObj &fo, uint32_t v) {
  write_dec(&fo, (uint64_t)v);
  return fo;
}
static inline const FileObj &operator<<(const FileObj &fo, uint64_t v) {
  write_dec(&fo, v);
  return fo;
}
static inline const FileObj &operator<<(const FileObj &fo, float v) {
  char buf[24];
  fmt_float(v, buf);
  fo.write(buf);
  return fo;
}
static inline const FileObj &operator<<(const FileObj &fo, double v) {
  char buf[24];
  fmt_double(v, buf);
  fo.write(buf);
  return fo;
}

class Flush {};
extern Flush flush;

static inline const FileObj &operator<<(const FileObj &fo, Flush) {
  fo.flush();
  return fo;
}

// TODO(pts): Add operator<< for StringOutObj.

// ---

// TODO(pts): Dumping: void operator~(const std::string &s) {}

int main() {
  sout << true;
  fflush(sout);
  sout << "Hello, " << -42 << "," << 123e200 << "," << 1.23f << "!\n" << flush;
  // Quirk: doesn't read anything, because result is not saved.
  // read_dec(stdin);
#if 0
  int8_t i8 = read_dec(stdin);
  printf("i8=(%d)\n", i8);
  int16_t i16 = read_dec(stdin);  // Polymorphic, does proper bounds checking.
#else
  int8_t i8;
  int16_t i16;
  // stdin >> dec(&i8) >> dec(&i16);
  stdin >> dec(&i8) >> "," >> nows >> dec(&i16);  // Has error handling.
  printf("i8=(%d)\n", i8);
#endif
  // float f = DecReader(stdin);  // Ambiguous conversion, doesn't compile.
  printf("i16=(%d)\n", i16);
  std::string a;
  if (!read_word(stdin, &a)) die("No word.");
  printf("a=(%s)\n", a.c_str());
  std::string b = read_word(stdin);
  printf("b=(%s)\n", b.c_str());
  stdin >> literal("\n") >> eof;  // Doesn't compile without literal(...).
  return 0;
}
