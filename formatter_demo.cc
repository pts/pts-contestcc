#include "r.h"

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

namespace r {
template<>class TFormatter<const C&> {
 public:
  typedef void *tag_type;
  typedef void *append_type;
  static void format_append(const C&, std::string *out) {
    out->append("C()");
  }
};
}  // namespace r

int main() {
  // TODO(pts): Which of these does error checking?
  FileShiftout(stdout) << 34LL << '$';  // Does error checking (because of FileShiftout).
  FileObj(stdout) << 987 << '$';  // Does error checking, implicit FileShiftout.
  FileWrapper(stdout) << "Hell" << -42;  // No error checking (because of FileWrapper).
  FileWrapper(sout) << "Help" << 567;  // No error checking (because of FileWrapper).
  sout << 65 << '%';  // Does error checking.
  sout << 'Z' << 66;  // Does error checking.
  stdout << StrPiece("Say", 3);
  stdout << std::string("Fox");
  // stdout << 65 << '%';  // Doesn't compile, because stdout and 65 are basic types.
  // To make FileShiftout(stdout) or `fo(stdout)' work, we need `const W&'.
  fprintf(FileShiftout(stdout) << 42U << ',' << -5LL << C(), ".\n");
  // SUXX: No way to make it work like this.
  // This doesn't work without the explicit operator<<(std:: string &,...).
  // printf("%s!\n", (std::string() << 42 << -5).c_str()); // (S).
  std::string s;
  // This wouldn't work if operator<< accepted `StringWritable&' instead of
  // `const StringWritable&'.
  printf("%s;\n", (s << 43ULL << -7).c_str());
  C c;
  printf("<C>\n");
  const C &cr(c);
  s << c;   // No copy of C.
  s << 34.56 << '+' << 34.56f;
  s << '+' << cr;  // No copy of C.
  s << D();
  s << (cat() << "x" << false);
  printf("</C>\n");
  s << "Foo" << std::string("Bar");
  {
    D d;
    stdout << std::string("Baz") << '^';
    stdout << d << '^';
  }
  // Since both stdout and true are basic types, FileShiftout(...) is needed.
  // TODO(pts): Add wrap(stdout) and add wrap(',') (in addition to literal(",")).
  FileShiftout(stdout) << true << (int8_t)33 << (uint8_t)44;
  FileShiftout(stdout) << (uint8_t)44 << '&' << 1.L / 3;
  FileShiftout(stdout) << std::string("ZAP");
  // FileObj(stdout) << 4.5;  // Doesn't compile, TFormatter<double> not defined.
  printf("S=(%s)\n", s.c_str());
  fflush(sout);
  return 0;
}
