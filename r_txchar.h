#ifndef R_TXCHAR_H
#define R_TXCHAR_H 1

#include <stdint.h>

namespace r {

template<class T>class TXChar {};
template<>struct TXChar<char>    { typedef void *tag_type; };
template<>struct TXChar<int8_t>  { typedef void *tag_type; };
template<>struct TXChar<uint8_t> { typedef void *tag_type; };

}  // namespace r

#endif  // R_TXCHAR_H
