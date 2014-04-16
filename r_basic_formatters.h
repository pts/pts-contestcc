#ifndef R_BASIC_FORMATTERS_H
#define R_BASIC_FORMATTERS_H 1

#ifndef __cplusplus
#error This is a C++ header.
#endif

#include "r_formatter.h"

// TODO(pts): Add shortcuts for writing int to stdout (etc.).
DEFINE_FORMATTER_COPY(int);
DEFINE_FORMATTER_PTR(char);

#endif // R_BASIC_FORMATTERS_H
