#ifndef R_FORMATTER_H
#define R_FORMATTER_H 1

#ifndef __cplusplus
#error This is a C++ header.
#endif

#include "r_writable.h"

// Declarations below are highly magical.
//
// Example custom type definition in the .h file:
//
//   DEFINE_FORMATTER_REF(MyClass);
//
// Example custom type definition in the .cc file:
//
//   void Formatter<MyClass>::format(Writable *wr, const MyClass &c) {
//     wr->vi_write("MyClass!", 8);
//   }
//
// Example usage:
//
//   sout << MyClass() << "\n";  // Prints "MyClass!\n".

// It's important that return_type is not defined here, but only in the
// template specializations (i.e. DEFINE_FORMATTER).
template<class T>class Formatter {};

#define DEFINE_FORMATTER(type, argtype) \
    template<>struct Formatter<type> { \
      typedef const Writable &return_type; \
      static void format(Writable *wr, argtype); \
    };
#define DEFINE_FORMATTER_PTR(type) DEFINE_FORMATTER(type*, type const*)
#define DEFINE_FORMATTER_REF(type) DEFINE_FORMATTER(type, type const&)
#define DEFINE_FORMATTER_COPY(type) DEFINE_FORMATTER(type, type)

#endif // R_FORMATTER_H
