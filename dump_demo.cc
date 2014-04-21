#include <stdint.h>

#include "r_dump.h"
#include "r_dump_stl.h"

// TODO(pts): Add a separator.
#define DUMP(type) dump(#type ": ", (type)(0))

// TODO(pts): Add a custom class which is actually dumpable.
class C {};

int main() {
  DUMP(bool);

  DUMP(char);
  DUMP(unsigned char);
  DUMP(signed char);
  DUMP(short);
  DUMP(unsigned short);
  DUMP(int);
  DUMP(unsigned int);
  DUMP(long);
  DUMP(unsigned long);
  DUMP(long long);
  DUMP(unsigned long long);
  DUMP(int8_t);
  DUMP(uint8_t);
  DUMP(int16_t);
  DUMP(uint16_t);
  DUMP(int32_t);
  DUMP(uint32_t);
  DUMP(int64_t);
  DUMP(uint64_t);

  DUMP(float);
  DUMP(double);
  DUMP(long double);

  sout << "---\n";
  dump("Hel\3456l\234o!\0not-truncated");
  dump((const char*)"Hel\345lo!\0truncated");  // "Hello\345!".
  dump(std::string("Hello!\n"));
  dump(std::string("Hel\0lo!\n", 8));
  dump(StrPiece("Hel\0lo!\n", 8));
  dump(StrPiece("Hello!\n", 7));
  char ca[] = {33, 44, 00, 55};  // No \0 at end.
  dump(ca);

  // dump(C());  // This won't compile, missing wrdump for C.

  char const *msg = "Bye";
  int a[3] = {55, 66, 77};
  std::vector<int> b;
  b.resize(3);
  b[2] = 44;

  dump(true);
  dump(-42);
  dump(a);
  dump(b);
  dump(std::list<int>());
  dump(std::set<int>());
  dump(std::multiset<int>());

#if __GXX_EXPERIMENTAL_CXX0X__ || __cplusplus >= 201100
  sout << "--- C++11:\n";
  dump(std::array<double, 3>());
  // To avoid warning here: g++ -std=c++0x -fno-deduce-init-list
  dump(std::initializer_list<int>({8, 9, 10}));
  // dump({8, 9, 10});  // This doesn't compile, can't infer type int.
#ifndef __clang__
  dump(std::forward_list<int>({8, 9, 10}));
#endif
#endif

  dump("Answer: ", 42);
  dump("Foo: ", 7, 6, 5);

  // TODO(pts): stdout << dump(42) << "\n";
  std::string s; s << dump(42) << dump(-5) << '.';
  // s << Formatter<const char*>(";.");  // Works.
  // s << Formatter(";.");  // Doesn't compile.
  s << ";." << "x" << msg << -42 << msg;
  // s << 12.34;
  printf("<%s>\n", s.c_str());
  // Unfortunately, `stdout << "HI"' will never compile, because both sides
  // of `<<` are basic types, so `operator<<' declarations are not
  // considered.
  FileObj(stdout) << "HI:" << dump(42) << dump(-6);
  stdout << dump(-7) << " " << dump(89) << dump('\'') << dump('\t') << ' ' << dump('\376') << dump('x');
  printf("\n");

  return 0;
}
