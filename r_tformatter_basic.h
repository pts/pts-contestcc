#ifndef R_TFORMATTER_BASIC_H
#define R_TFORMATTER_BASIC_H 1

#ifndef __cplusplus
#error This is a C++ header.
#endif

#include <string.h>

#include <string>

#include "r_str_piece.h"
#include "r_tformatter.h"

template<>class TFormatter<bool> {
 public:
  typedef void *tag_type;
  enum max_type { max_buf_size = 5 };  // Including the trailing \0.
  static inline void format(bool v, char *buf) {
    strcpy(buf, v ? "true" : "false");
  }
};

// TODO(pts): Add for all integral and floating point types.
template<>class TFormatter<int> {
 public:
  typedef void *tag_type;
  enum max_type { max_buf_size = 12 };
  static void format(int v, char *buf);
};

template<>class TFormatter<const char*> {
 public:
  typedef void *tag_type;
  typedef void *str_piece_type;
  static inline StrPiece format_str_piece(const char *v) {
    return StrPiece(v, strlen(v));
  }
};

template<>class TFormatter<std::string> {
 public:
  typedef void *tag_type;
  typedef void *str_piece_type;
  static inline StrPiece format_str_piece(const std::string &v) {
    return StrPiece(v.data(), v.size());
  }
};

#endif // R_TFORMATTER_BASIC_H
