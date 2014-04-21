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
  enum max_type { max_buf_size = 6 };  // Including the trailing \0.
  static inline void format(bool v, char *buf) {
    strcpy(buf, v ? "true" : "false");
  }
};

// --- Integral types.

// No need to define TFormatter<int8_t> and TFormatter<uint8_t>, because
// char matches these types, and the template with the `char v' argument
// defined in r_shiftout_base.h already formats char.

template<>class TFormatter<int32_t> {
 public:
  typedef void *tag_type;
  enum max_type { max_buf_size = 12 };
  static void format(int32_t v, char *buf);
};

template<>class TFormatter<uint32_t> {
 public:
  typedef void *tag_type;
  enum max_type { max_buf_size = 11 };
  static void format(uint32_t v, char *buf);
};

template<>class TFormatter<int16_t> {
 public:
  typedef void *tag_type;
  enum max_type { max_buf_size = 7 };
  static inline void format(int16_t v, char *buf) {
    TFormatter<int32_t>::format(v, buf);  // Implicit cast.
  }
};

template<>class TFormatter<uint16_t> {
 public:
  typedef void *tag_type;
  enum max_type { max_buf_size = 6 };
  static inline void format(uint16_t v, char *buf) {
    TFormatter<uint32_t>::format(v, buf);  // Implicit cast.
  }
};

template<>class TFormatter<int64_t> {
 public:
  typedef void *tag_type;
  enum max_type { max_buf_size = 21 };
  static void format(int64_t v, char *buf);
};

template<>class TFormatter<uint64_t> {
 public:
  typedef void *tag_type;
  enum max_type { max_buf_size = 20 };
  static void format(uint64_t v, char *buf);
};

// TODO(pts): Implement write_hex somewhere else.

// --- Floating point types.


template<>class TFormatter<float> {
 public:
  typedef void *tag_type;
  enum max_type { max_buf_size = 17 };
  static void format(float v, char *buf);
};

template<>class TFormatter<double> {
 public:
  typedef void *tag_type;
  // len(repr(float(-1.0000012345e-300))) == 24.
  enum max_type { max_buf_size = 25 };
  static void format(double v, char *buf);
};

template<>class TFormatter<long double> {
 public:
  typedef void *tag_type;
  // len('-1.18973149535723176502e+4932') == 29.
  // len('-3.64519953188247460253e-4951') == 29.
  // Using 32 instead of 30 as a safety margin.
  enum max_type { max_buf_size = 32 };
  static void format(long double v, char *buf);
};


// --- String-like types.

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

template<>class TFormatter<StrPiece> {
 public:
  typedef void *tag_type;
  typedef void *str_piece_type;
  static inline const StrPiece format_str_piece(const StrPiece &v) {
    return v;
  }
};

#endif // R_TFORMATTER_BASIC_H
