#ifndef R_STR_PIECE_H
#define R_STR_PIECE_H 1

#include <stdint.h>
#include <string.h>

namespace r {

struct StrPiece {
  inline StrPiece(const char *data): data(data), size(strlen(data)) {}
  inline StrPiece(const char *data, uintptr_t size)
      : data(data), size(size) {}
  const char *data;
  uintptr_t size;
};

}  // namespace r

#endif  // R_STR_PIECE_H
