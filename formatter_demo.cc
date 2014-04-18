#include "r_shiftout.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <string>

class C {
 public:
  C() { printf("+C\n"); }
  ~C() { printf("~C\n"); }
  C(const C&) { printf("*C\n"); }
  C& operator=(const C&) { printf("=C\n"); return *this; }
};

class D {
 public:
  D() { printf("+D\n"); }
  ~D() { printf("~D\n"); }
  D(const D&) { printf("*D\n"); }
  D& operator=(const D&) { printf("=D\n"); return *this; }

  typedef void *format_append_type;  // Corresponds to format_append.
  void format_append(std::string *out) const {
    out->append("D()");
  }
};

template<>class TFormatter<const C&> {
 public:
  typedef void *tag_type;
  typedef void *append_type;
  static void format_append(const C&, std::string *out) {
    out->append("C()");
  }
};

int main() {
  // This also does error checking, but it's a tiny bit slower than using
  // FileShiftout(stdout) instead.
  FileObj(stdout) << 987 << '$';
  // To make FileShiftout(stdout) or `fo(stdout)' work, we need `const W&'.
  fprintf(FileShiftout(stdout) << 42 << ',' << -5 << C(), ".\n");
  // SUXX: No way to make it work like this.
  // This doesn't work without the explicit operator<<(std:: string &,...).
  // printf("%s!\n", (std::string() << 42 << -5).c_str()); // (S).
  std::string s;
  // This wouldn't work if operator<< accepted `StringWritable&' instead of
  // `const StringWritable&'.
  printf("%s;\n", (s << 42 << -5).c_str());
  C c;
  printf("<C>\n");
  const C &cr(c);
  s << c;   // No copy of C.
  s << '+' << cr;  // No copy of C.
  s << D();
  printf("</C>\n");
  s << "Foo" << std::string("Bar");
  {
    D d;
    stdout << std::string("Baz") << '^';
    stdout << d << '^';
  }
  // Since both stdout and true are basic types, FileShiftout(...) is needed.
  // TODO(pts): Add wrap(stdout) and add wrap(',') (in addition to literal(",")).
  FileShiftout(stdout) << true;
  // FileObj(stdout) << 4.5;  // Doesn't compile, TFormatter<double> not defined.
  printf("S=(%s)\n", s.c_str());
  return 0;
}
