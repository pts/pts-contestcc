#include "r_shiftin.h"

//#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <string>

static inline bool is_whitespace(int c) {
  return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

static inline bool is_digit(int c) {
  return c + 0U - '0' <= 9;
}

static inline bool is_alpha(int c) {
  return (c | 32U) - 'a' <= 'z' - 'a' + 0U;
}

static inline bool is_word(int c) {
  return is_alpha(c) || is_digit(c) || c == '_';
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

Status read_dec(FILE *f, unsigned nbytes, int64_t *out) {
  //assert(nbytes <= 8);
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
    if (n10 / 10 != n || n10c < n10 || n10c > limit) {
      return "Out of bounds when reading signed decimal.";
    }
    n = n10c;
  }
  if (c >= 0) ungetc(c, f);
  *out = is_neg ? -n : n;
  return true;
}

Status read_dec(FILE *f, unsigned nbytes, uint64_t *out) {
  //assert(nbytes <= 8);
  int c;
  while ((c = getc(f)) >= 0 && is_whitespace(c)) {}
  if (!is_digit(c)) {
    if (c >= 0) ungetc(c, f);
    return "EOF when reading unsigned decimal.";
  }
  uint64_t n = c - '0';
  const uint64_t limit = (uint64_t)~0 >> (64 - 8 * nbytes);
  while (is_digit(c = getc(f))) {
    const uint64_t n10 = 10 * n;
    const uint64_t n10c = n10 + c - '0';
    if (n10 / 10 != n || n10c < n10 || n10c > limit) {
      return "Out of bounds when reading unsigned decimal.";
    }
    n = n10c;
  }
  if (c >= 0) ungetc(c, f);
  *out = n;
  return true;
}

// Faster implementation (because of faster overflow checking) for <= 4 bytes.
Status read_dec(FILE *f, unsigned nbytes, int32_t *out) {
  //assert(nbytes <= 4);
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
  uint32_t n = c - '0';
  const uint64_t limit = ((uint64_t)1 << (8 * nbytes - 1)) - 1 + is_neg;
  while (is_digit(c = getc(f))) {
    const uint64_t n64 = 10 * (uint64_t)n + (c - '0');
    if (n64 > limit) {
      return "Out of bounds when reading signed decimal.";
    }
    n = n64;
  }
  if (c >= 0) ungetc(c, f);
  *out = is_neg ? -n : n;
  return true;
}

// Faster implementation (because of faster overflow checking) for <= 4 bytes.
Status read_dec(FILE *f, unsigned nbytes, uint32_t *out) {
  //assert(nbytes <= 4);
  int c;
  while ((c = getc(f)) >= 0 && is_whitespace(c)) {}
  if (!is_digit(c)) {
    if (c >= 0) ungetc(c, f);
    return "EOF when reading unsigned decimal.";
  }
  uint32_t n = c - '0';
  const uint64_t limit = (uint32_t)~0 >> (64 - 8 * nbytes);
  while (is_digit(c = getc(f))) {
    const uint64_t n64 = 10 * (uint64_t)n + (c - '0');
    if (n64 > limit) {
      return "Out of bounds when reading unsigned decimal.";
    }
    n = n64;
  }
  if (c >= 0) ungetc(c, f);
  *out = n;
  return true;
}

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

Status read_literal(FILE *f, char expc) {
  int c = getc(f);
  // TODO(pts): Report what literal (msg0).
  if (c < 0) return "EOF when reading literal char.";
  if ((char)c != expc) {
    ungetc(c, f);
    return "Unexpected input when reading literal char.";
  }
  return true;
}

Status peek_nows(FILE *f) {
  const int c = getc(f);
  if (c >= 0) {
    const bool is_ws = is_whitespace(c);
    ungetc(c, f);
    if (is_ws) return "Unexpected whitespace.";
  }
  return true;
}

Status peek_eof(FILE *f) {
  const int c = getc(f);
  if (c >= 0) {
    ungetc(c, f);
    return "Expected EOF.";
  }
  return true;
}

Status read_line(FILE *f, std::string *line) {
  line->clear();
  int c;
  while ((c = getc(f)) >= 0) {
    line->push_back(c);
    if (c == '\n') {
      // Not by default. TODO(pts): Add another reader for that.
      //if (!line->empty() && (*line)[line->size() - 1] == '\r') {
      //  line->resize(line->size() - 1);
      //}
      break;
    }
  }
  return true;
}

Status read_char(FILE *f, char *p) {
  char c = getc(f);
  if (c < 0) return "EOF when reading char.";
  *p = c;
  return true;
}

Status read_bool(FILE *f, bool *out) {
  int c;
  while (is_whitespace(c = getc(f))) {}
  if (c < 0) { on_eof:
    return "EOF when reading bool.";
  }
  // Accept "true" or "false".
  char const *msg = c == 't' ? "rue" : c == 'f' ? "alse" : NULL;
  if (!msg) { on_err:
    ungetc(c, f);
    return "Unexpected input when reading bool.";
  }
  const bool v = c == 't';
  for (; msg[0] != '\0'; ++msg) {
    c = getc(f);
    // TODO(pts): Report what literal (msg0).
    if (c < 0) goto on_eof;
    if ((char)c != msg[0]) goto on_err;
  }
  if ((c = getc(f)) >= 0) {
    ungetc(c, f);
    if (is_word(c)) return "Unexpected word character read after bool.";
  }
  *out = v;
  return true;
}
