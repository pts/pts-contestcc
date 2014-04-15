// C++ disadvantage: memory errors (e.g. segfault), no array bounds checking
// C++ disadvantage: no stack trace
// C++ disadvantage: no unliminted ints by default
// C++ disadvantage: no easy int overflow detection
// C++ disadvantage: uninitialized variables (there is a warning)
// C++ disadvantage: no automatic memory management (not a big problem)
// C++ disadvantage: no regexps (not a problem most of the time)

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <string>

// For eGlibc, math.h defines this, for uClibc, it doesn't.
// #define NAN (__builtin_nanf (""))

void die(const char *msg) {
  fprintf(stderr, "fatal: %s\n", msg);
  exit(1);
}

static inline bool is_whitespace(int c) {
  return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

static inline bool is_digit(int c) {
  return c - '0' + 0U <= 9;
}

template<class T>static inline T *notnull(T *t) {
  if (!t) die("NULL pointer found.");
  return t;
}

template<class T>static inline const T *notnull(const T *t) {
  if (!t) die("NULL pointer found.");
  return t;
}

#define assume_notnull(t) (t)

class Status {
 public:
  // TODO(pts): Don't inline these constructs and destrutors?
  Status(bool is_ok)
      : msg_(is_ok ? NULL : "Something failed."), is_used_(false) {}
  // msg is owned externally.
  Status(char const *msg): msg_(msg), is_used_(false) {}
  ~Status() {
    // TODO(pts): Show stack trace.
    if (!is_used_ && msg_) die(msg_);
  }
  bool ok() const { is_used_ = true; return !msg_; }
  operator bool() const { return ok(); }

 private:
  const char *msg_;
  mutable bool is_used_;
};

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

class FileObj {
 public:
  FileObj(FILE *f): f_(assume_notnull(f)) {}
  FILE *f() const { return f_; }
  Status flush() const {
    return fflush(f_) == 0;
  }
  // TODO(pts): move to write(FILE *f, ...)?
  Status write(const void *p, uintptr_t size) const {
    return fwrite(p, 1, size, f_) == size;
  }
  Status write(const char *msg) const {
    return write(msg, strlen(msg));
  }
  // TODO(pts): Don't inline write_dec.
  // TODO(pts): Add int32_t.
  Status write_dec(int64_t v) const {
    char buf[21], *p = buf, *q, c;
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
    return write(buf);
  }
  Status write_dec(uint64_t v) const {
    char buf[20], *p = buf, *q, c;
    q = p;
    do { *q++ = v % 10 + '0'; v /= 10; } while (v > 0);
    *q-- = '\0';
    while (p < q) {  // Reverse.
      c = *p;
      *p++ = *q;
      *q-- = c;
    }
    return write(buf);
  }
  // TODO(pts): Implement write_hex etc.

  // Most convenience functions are in `>>'.
 private:
  FILE *f_;  // Owned externally.
};

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

// --- Float formatting.

// TODO(pts): Also for long double.
void fmt_double(double d, char outbuf[32]) {
  // Assuming IEEE 754 64-bit double.
  struct AssertDoubleSizeIs8 { int DoubleSizeIs8 : sizeof(d) == 8; };
  char *p;
  int c;
  sprintf(outbuf, "%.17g", d);
  for (c = 0, p = outbuf; *p && *p != 'e'; ++p) {
    c += *p - '0' + 0U <= 9U;
  }
  /* Used all digits, e.g. "1.2300000000000001e+202", "0.3333333333333333" */
  if (c >= 17) {  /* Can be 17 or 18. */
    char tmp[32];
    double d16;
    sprintf(tmp, "%.16g", d);
    if (1 == sscanf(tmp, "%lf", &d16) && d == d16) {
      strcpy(outbuf, tmp);
    }
  }
}

void fmt_float(float d, char outbuf[24]) {
  // Assuming IEEE 754 32-bit float.
  struct AssertFloatSizeIs4 { int FloatSizeIs4 : sizeof(d) == 4; };
  char *p;
  int c;
  sprintf(outbuf, "%.9g", d);
  for (c = 0, p = outbuf; *p && *p != 'e'; ++p) {
    c += *p - '0' + 0U <= 9U;
  }
  if (c >= 9) {
    char tmp[24];
    float d8;
    sprintf(tmp, "%.8g", d);
    if (1 == sscanf(tmp, "%f", &d8) && d == d8) {
      strcpy(outbuf, tmp);
    }
  }
}


// --- Output.

class Io {};
extern Io io;

FileObj sin(stdin);
// sout << "Hello\n";
FileObj sout(stdout);
FileObj serr(stderr);
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
  fo.write_dec((int64_t)v);
  return fo;
}
static inline const FileObj &operator<<(const FileObj &fo, int16_t v) {
  fo.write_dec((int64_t)v);
  return fo;
}
static inline const FileObj &operator<<(const FileObj &fo, int32_t v) {
  fo.write_dec((int64_t)v);
  return fo;
}
static inline const FileObj &operator<<(const FileObj &fo, int64_t v) {
  fo.write_dec(v);
  return fo;
}
static inline const FileObj &operator<<(const FileObj &fo, uint8_t v) {
  fo.write_dec((uint64_t)v);
  return fo;
}
static inline const FileObj &operator<<(const FileObj &fo, uint16_t v) {
  fo.write_dec((uint64_t)v);
  return fo;
}
static inline const FileObj &operator<<(const FileObj &fo, uint32_t v) {
  fo.write_dec((uint64_t)v);
  return fo;
}
static inline const FileObj &operator<<(const FileObj &fo, uint64_t v) {
  fo.write_dec(v);
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
  // TODO(pts): Add type-safe printf, long long.
  printf("i16=(%d)\n", i16);
  std::string a;
  if (!read_word(stdin, &a)) die("No word.");
  printf("a=(%s)\n", a.c_str());
  std::string b = read_word(stdin);
  printf("b=(%s)\n", b.c_str());
  stdin >> literal("\n") >> eof;  // Doesn't compile without literal(...).
  return 0;
}
