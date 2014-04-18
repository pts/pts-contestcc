#ifndef R_FILEOBJ_H
#define R_FILEOBJ_H 1

#ifndef __cplusplus
#error This is a C++ header.
#endif

#include "r_status.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

class FileObj {
 public:
  FileObj(FILE *f): f_(assume_notnull(f)) {}
  FILE *f() const { return f_; }
  operator FILE*() { return f_; }
  Status flush() const {
    return fflush(f_) == 0;
  }
  // TODO(pts): move to write(FILE *f, ...)?
  Status write(const void *p, uintptr_t size) const {
    // TODO(pts): Add proper error message. (Also elsewhere.)
    return fwrite(p, 1, size, f_) == size;
  }
  Status write(const char *msg) const {
    return write(msg, strlen(msg));
  }
  // Most convenience functions are in `>>'.
 private:
  FILE *f_;  // Owned externally.
};

#endif // R_FILEOBJ_H
