// C++ disadvantage: memory errors (e.g. segfault), no array bounds checking
// C++ disadvantage: no stack trace
// C++ disadvantage: no unliminted ints by default
// C++ disadvantage: no easy int overflow detection

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <string>

void die(const char *msg) {
  fprintf(stderr, "fatal: %s\n", msg);
  exit(1);
}

static inline bool is_space(int c) {
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
  while (is_space(c = getc(f))) {}
  if (c < 0) return "EOF when reading word.";
  out->clear();
  out->push_back(c);
  for (; !is_space(c = getc(f)); out->push_back(c)) {}
  if (c >= 0) ungetc(c, f);
  return true;
}

std::string read_word(FILE *f) {
  std::string result;
  read_word(f, &result);  // die(...)s on error.
  return result;
}

// TODO(pts): Implement unsigned version.
Status read_dec(FILE *f, int nbytes, int64_t *out) {
  bool is_neg = false;
  int c;
  while ((c = getc(f)) >= 0 && is_space(c)) {}
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

// TODO(pts): Dumping: void operator~(const std::string &s) {}

int main(int argc, char **argv) {
  (void)argc; (void)argv;
  int8_t i8 = read_dec(stdin);
  printf("i8=(%d)\n", i8);  // TODO(pts): Add type-safe printf, long long.
  int16_t i16 = read_dec(stdin);
  // float f = DecReader(stdin);  // Ambiguous conversion, doesn't compile.
  printf("i16=(%d)\n", i16);
  std::string a;
  if (!read_word(stdin, &a)) die("No word.");
  printf("a=(%s)\n", a.c_str());
  std::string b = read_word(stdin);
  printf("b=(%s)\n", b.c_str());
  return 0;
}
