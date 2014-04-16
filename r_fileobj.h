#ifndef R_FILEOBJ_H
#define R_FILEOBJ_H 1

#ifndef __cplusplus
#error This is a C++ header.
#endif

#include "r_status.h"
#include "r_writable.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

class FileObj: public Writable {
 public:
  FileObj(FILE *f): f_(assume_notnull(f)) {}
  FILE *f() const { return f_; }
  operator FILE*() { return f_; }
  Status flush() const {
    return fflush(f_) == 0;
  }
  // TODO(pts): move to write(FILE *f, ...)?
  Status write(const void *p, uintptr_t size) const {
    return fwrite(p, 1, size, f_) == size;
  }
  Status write(const char *msg) const {
    return write(msg, strlen(msg));
  }
  // Implements method from Writable.
  // TODO(pts): Move this to the .cc file.
  virtual void vi_write(const void *p, uintptr_t size) {
    Status(fwrite(p, 1, size, f_) == size);
  }
  // Implements method from Writable.
  // TODO(pts): Move this to the .cc file.
  virtual void vi_putc(char c) {
    Status(putc(c, f_) > 0);
  }

  // Most convenience functions are in `>>'.
 private:
  FILE *f_;  // Owned externally.
};

#endif // R_FILEOBJ_H
