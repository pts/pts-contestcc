#ifndef R_DUMP_BASIC_H
#define R_DUMP_BASIC_H 1

#ifndef __cplusplus
#error This is a C++ header.
#endif

#include <stdint.h>
#include <string.h>

#include <string>

#include "r_strpiece.h"

namespace r {

void wrdump_low(char v, std::string *out);
void wrdump_low(const StrPiece &v, std::string *out);
void wrdump_low(const std::string &v, std::string *out);
void wrdump_low(const char *v, uintptr_t size, std::string *out);
void wrdump_low(const char *v, std::string *out);

}  // namespace r

#endif  // R_DUMP_BASIC_H
