#ifndef R_STRMSG_H
#define R_STRMSG_H 1

#ifndef __cplusplus
#error This is a C++ header.
#endif

#include <stdint.h>
#include <string.h>

#include <string>

class StrMsg {
 public:
  inline StrMsg(const std::string &str): data(str.data()), size(str.size()) {}
  inline StrMsg(const char *data): data(data), size(strlen(data)) {}
  inline StrMsg(const char *data, uintptr_t size)
      : data(data), size(size) {}

  const char *data;
  uintptr_t size;
};

#endif  // R_STRMSG_H
