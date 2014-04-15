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
  Status(bool is_ok): is_ok_(is_ok), is_used_(false) {}
  ~Status() {
    // TODO(pts): Show stack trace.
    if (!is_used_ && !is_ok_) die("Something failed.");
  }
  bool ok() const { is_used_ = true; return is_ok_; }
  operator bool() const { return ok(); }

 private:
  bool is_ok_;
  mutable bool is_used_;
};

Status read_word(FILE *f, std::string *out) {
  int c;
  while (is_space(c = getc(f))) {}
  if (c < 0) return false;
  out->clear();
  out->push_back(c);
  for (; !is_space(c = getc(f)); out->push_back(c)) {}
  if (c >= 0) ungetc(c, f);
  return true;
}

std::string read_word(FILE *f) {
  std::string result;
  read_word(f, &result);  // die(...) on error.
  return result;
}

int main(int argc, char **argv) {
  (void)argc; (void)argv;
  std::string a;
  if (!read_word(stdin, &a)) die("No word.");
  printf("a=(%s)\n", a.c_str());
  std::string b = read_word(stdin);
  printf("b=(%s)\n", b.c_str());
  return 0;
}
