#ifndef R_TXCHAR_H
#define R_TXCHAR_H 1

#ifndef __cplusplus
#error This is a C++ header.
#endif

#include <stdint.h>

template<class T>class TXChar {};
template<>struct TXChar<char> {
  typedef void *tag_type;
};
template<>struct TXChar<int8_t> {
  typedef void *tag_type;
};
template<>struct TXChar<uint8_t> {
  typedef void *tag_type;
};

#endif  // R_TXCHAR_H
