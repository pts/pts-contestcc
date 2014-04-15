// C++ disadvantage: memory errors (e.g. segfault), no array bounds checking
// C++ disadvantage: no stack trace
// C++ disadvantage: no unliminted ints by default
// C++ disadvantage: no easy int overflow detection
// C++ disadvantage: uninitialized variables (there is a warning)
// C++ disadvantage: no automatic memory management (not a big problem)

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <string>

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

class Status {
 public:
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

class DecOutI8 {
 public:
  DecOutI8(int8_t *p): p_(p) {}  // TODO(pts): CHECK_NOTNULL(p).
  int8_t *get() const { return p_; }
 private:
  int8_t *p_;
};
class DecOutI16 {
 public:
  DecOutI16(int16_t *p): p_(p) {}  // TODO(pts): CHECK_NOTNULL(p).
  int16_t *get() const { return p_; }
 private:
  int16_t *p_;
};

FILE *operator>>(FILE *f, const DecOutI8 &out) {
  read_dec(f, out.get());
  return f;
}
FILE *operator>>(FILE *f, const DecOutI16 &out) {
  read_dec(f, out.get());
  return f;
}

DecOutI8  dec(int8_t  *p) { return p; }
DecOutI16 dec(int16_t *p) { return p; }

Status read_literal(FILE *f, const char *msg, intptr_t size) {
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

class LiteralOut {
 public:
  LiteralOut(const char *msg, intptr_t size): msg_(msg), size_(size) {}
  LiteralOut(const char *msg): msg_(msg), size_(strlen(msg)) {}
  Status read(FILE *f) const { return read_literal(f, msg_, size_); }
 private:
  const char * const msg_;  // Owned externally.
  const intptr_t size_;
};

// TODO(pts): Move literal, dec etc. to a namespace.
LiteralOut literal(const char *msg) { return msg; }
LiteralOut literal(const char *msg, intptr_t size) {
  return LiteralOut(msg, size);
}

FILE *operator>>(FILE *f, const LiteralOut &out) { out.read(f); return f; }

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

class NowsOut {};
NowsOut nows;

FILE *operator>>(FILE *f, NowsOut) { peek_nows(f); return f; }


// TODO(pts): Dumping: void operator~(const std::string &s) {}

int main(int argc, char **argv) {
  (void)argc; (void)argv;
#if 0
  int8_t i8 = read_dec(stdin);
  printf("i8=(%d)\n", i8);
  int16_t i16 = read_dec(stdin);
#else
  int8_t i8;
  int16_t i16;
  // stdin >> dec(&i8) >> dec(&i16);  // Has error handling.
  stdin >> dec(&i8) >> literal(",") >> nows >> dec(&i16);
  // TODO(pts): stdin >> dec(&i8) >> "," >> nows >> dec(&i16) >> "\n" >> eof;
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
  return 0;
}
