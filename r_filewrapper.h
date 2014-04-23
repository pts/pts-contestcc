#ifndef R_FILEWRAPPER_H
#define R_FILEWRAPPER_H 1

#ifndef __cplusplus
#error This is a C++ header.
#endif

#include <stdio.h>

// A low-overhead wrapper struct for a FILE*.
//
// We need this class for operator<<, because FILE* is not an user-defined
// type.
class FileWrapper {
 public:
  inline explicit FileWrapper(FILE *f): f(f) {}
  inline operator FILE*() const { return f; }
  // Don't add more fields, so FileWrapper can be copied easily.
  FILE *f;
};

#endif  // R_FILEWRAPPER
