#include "r_shiftin.h"

#include <string>

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

Status read_dec(FILE *f, int nbytes, uint64_t *out) {
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
    // TODO(pts): Use faster implementation if nbytes < sizeof(int64_t).
    if (n10 / 10 != n || n10c < n10 || n10c > limit) {
      return "Out of bounds when reading unsigned decimal.";
    }
    n = n10c;
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
      // Not by default. TODO(pts): Add another reader.
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
