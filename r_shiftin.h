// TODO(pts): Add reading bool (can't ungetc fully).
// TODO(pts): Implement write_hex etc.
// TODO(pts): wrap(stdin) instead of `FileWrapper(stdin) >> stdin'.
// TODO(pts): Move literal, dec etc. to a namespace.
// TODO(pts): Add line(&str);

#ifndef R_SHIFTIN_H
#define R_SHIFTIN_H 1

#ifndef __cplusplus
#error This is a C++ header.
#endif

#include "r_filewrapper.h"
#include "r_sinouterr.h"
#include "r_status.h"
#include "r_typetuple.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <string>

// TODO(pts): Make input parsing more generic, i.e. make it work on
// std::string. This is a lot of work and can lead to code duplication,
// because we want to make the FILE* path fast.

// For eGlibc, math.h defines this, for uClibc, it doesn't.
// #define NAN (__builtin_nanf (""))

static inline bool is_whitespace(int c) {
  return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

static inline bool is_digit(int c) {
  return c - '0' + 0U <= 9;
}

Status read_word(FILE *f, std::string *out);
std::string read_word(FILE *f);  // With error handling.

// Ignores optional whitespace in front of the number.
//
// TODO(pts): Implement unsigned version.
Status read_dec(FILE *f, int nbytes, int64_t *out);

static inline Status read_dec(FILE *f, int64_t *out) {
  return read_dec(f, 8, out);
}
static inline Status read_dec(FILE *f, int32_t *out) {
  int64_t i64; Status s = read_dec(f, 4, &i64); *out = i64; return s;
}
static inline Status read_dec(FILE *f, int16_t *out) {
  int64_t i64; Status s = read_dec(f, 2, &i64); *out = i64; return s;
}
static inline Status read_dec(FILE *f, int8_t  *out) {
  int64_t i64; Status s = read_dec(f, 1, &i64); *out = i64; return s;
}

class DecReader {
 public:
  inline DecReader(FILE *f): f_(f) {}
  inline operator int8_t() {
    int64_t i64; read_dec(f_, 1, &i64); return i64;
  }
  inline operator int16_t() {
    int64_t i64; read_dec(f_, 2, &i64); return i64;
  }
  inline operator int32_t() {
    int64_t i64; read_dec(f_, 4, &i64); return i64;
  }
  inline operator int64_t() {
    int64_t i64; read_dec(f_, 8, &i64); return i64;
  }
 private:
  FILE *f_;
};

// Number not red if result is not saved.
static inline DecReader read_dec(FILE *f) { return DecReader(f); }

// ---

class DecInI8 {
 public:
  inline DecInI8(int8_t *p): p_(notnull(p)) {}
  inline int8_t *get() const { return p_; }
 private:
  int8_t *p_;
};

class DecInI16 {
 public:
  inline DecInI16(int16_t *p): p_(notnull(p)) {}
  inline int16_t *get() const { return p_; }
 private:
  int16_t *p_;
};

static inline const FileWrapper &operator>>(
    const FileWrapper &f, const DecInI8 &out) {
  read_dec(f.f, out.get());
  return f;
}
static inline const FileWrapper &operator>>(
    const FileWrapper &f, const DecInI16 &out) {
  read_dec(f.f, out.get());
  return f;
}

// SUXX: These need a valid copy-constructor even if they don't copy.
static inline DecInI8  dec(int8_t  *p) { return DecInI8(p); }
static inline DecInI16 dec(int16_t *p) { return DecInI16(p); }

Status read_literal(FILE *f, const char *msg, uintptr_t size);

class LiteralIn {
 public:
  inline LiteralIn(const char *msg, uintptr_t size): msg_(msg), size_(size) {}
  inline LiteralIn(const char *msg): msg_(msg), size_(strlen(msg)) {}
  inline Status read(FILE *f) const { return read_literal(f, msg_, size_); }
 private:
  const char * const msg_;  // Owned externally.
  const uintptr_t size_;
};

static inline const FileWrapper &operator>>(
    const FileWrapper &f, const LiteralIn &in) {
  in.read(f.f);
  return f;
}
static inline const FileWrapper &operator>>(const FileWrapper &f, const char *in) {
  LiteralIn(in).read(f.f);
  return f;
}


static inline LiteralIn literal(const char *msg) { return msg; }
static inline LiteralIn literal(const char *msg, uintptr_t size) {
  return LiteralIn(msg, size);
}

Status read_literal(FILE *f, char expc);

class LiteralCharIn {
 public:
  inline LiteralCharIn(char c): c_(c) {}
  inline Status read(FILE *f) const { return read_literal(f, c_); }
 private:
  const char c_;
};

static inline const FileWrapper &operator>>(
    const FileWrapper &f, const LiteralCharIn &in) {
  in.read(f.f);
  return f;
}
static inline const FileWrapper &operator>>(const FileWrapper &f, char c) {
  LiteralCharIn(c).read(f.f);
  return f;
}

static inline LiteralCharIn literal(char c) { return c; }

// Check that next character is not a whitespace.
Status peek_nows(FILE *f);

class NowsIn {};
extern NowsIn nows;

static inline const FileWrapper &operator>>(const FileWrapper &f, NowsIn) {
  peek_nows(f.f); return f;
}

// Check that next character is not a whitespace.
Status peek_eof(FILE *f);
class EofIn {};
extern EofIn eof;

static inline const FileWrapper &operator>>(const FileWrapper &f, EofIn) {
  peek_eof(f.f); return f;
}

// --- Starting the operator>> chain.

template<class S, class V>static inline
typename TypePair<FileWrapper, typename TStdStream<S>::tag_type>::first_type
operator>>(const S &s, const V &v) {
  FileWrapper fo(s);
  fo >> v;
  return fo;
}

// This also makes it work for FileObj.
template<class V>static inline FileWrapper operator>>(FILE *f, const V &v) {
  FileWrapper fo(f);
  fo >> v;
  return fo;
}

#endif  // R_SHIFTIN_H
