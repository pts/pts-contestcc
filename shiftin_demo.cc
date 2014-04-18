#include "r_shiftin.h"
#include "r_shiftout.h"

int main() {
  sout << "Hi!" << endl;
  static const char kSlash[] = "/";
  int8_t x8;
  if (1) sin >> kSlash >> dec(&x8);
  if (0) sin >> dec(&x8) >> "\n";
  if (0) sout >> dec(&x8) >> "\n";
  if (0) FileObj(sout) >> '\n';
  sout << true;
  fflush(sout);
  sout << "Hello, " << -42 << "," << 123e200 << "," << 1.23f << "!\n" << flush;
  // Quirk: doesn't read anything, because result is not saved.
  // read_dec(stdin);
  if (0) {
    int8_t i8 = read_dec(stdin);
    printf("i8a=(%d)\n", i8);
    int16_t i16 = read_dec(stdin);  // Polymorphic, does proper bounds checking.
    printf("i16a=(%d)\n", i16);
  }
  int8_t i8;
  int16_t i16;
  // stdin >> dec(&i8) >> dec(&i16);
  stdin >> dec(&i8) >> "," >> nows >> dec(&i16);  // Has error handling.
  printf("i8=(%d)\n", i8);
  // float f = DecReader(stdin);  // Ambiguous conversion, doesn't compile.
  printf("i16=(%d)\n", i16);
  std::string a;
  if (!read_word(stdin, &a)) die("No word.");
  printf("a=(%s)\n", a.c_str());
  std::string b = read_word(stdin);
  printf("b=(%s)\n", b.c_str());
  stdin >> literal("\n") >> eof;  // Doesn't compile without literal(...).
  return 0;
}
