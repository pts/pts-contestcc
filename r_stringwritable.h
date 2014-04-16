#ifndef R_STRINGWRITABLE_H
#define R_STRINGWRITABLE_H 1

#ifndef __cplusplus
#error This is a C++ header.
#endif

#include <string>

#include "r_status.h"
#include "r_writable.h"

class StringWritable: public Writable {
 public:
  // This is needed for convenience in my_string << dump(42).
  StringWritable(std::string &str): str_(&str) {}
  StringWritable(std::string *str): str_(assume_notnull(str)) {}
  virtual void vi_write(const void *p, uintptr_t size) {
    str_->append(static_cast<const char *>(p), size);
  }
  virtual void vi_putc(char c) {
    str_->push_back(c);
  }
 private:
  std::string *str_;
};

#endif // R_STRINGWRITABLE_H
