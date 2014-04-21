#include <stdint.h>

#include "r_dump.h"

#define DUMP(type) dump(#type, (type)(0))

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

  // TODO(pts): Undefined reference to sout with g++ -O0.
  sout << "---\n";

  // dump(C());  // This won't compile. 

  char const *msg = "Bye";
  int a[3] = {55, 66, 77};
  std::vector<int> b;
  b.resize(3);
  b[2] = 44;

  dump(true);
  dump(-42);
  dump(a);
  dump(b);
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