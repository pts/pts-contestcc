#ifndef R_WRITABLE_H
#define R_WRITABLE_H 1

#ifndef __cplusplus
#error This is a C++ header.
#endif

#include <stdint.h>

class Writable {
 public:
  virtual ~Writable() {}
  virtual void vi_write(const void *p, uintptr_t size) = 0;
  virtual void vi_putc(char c) = 0;
};

#endif // R_WRITABLE_H
