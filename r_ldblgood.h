#ifndef R_LDBL_GOOD_H
#define R_LDBL_GOOD_H 1

#ifndef __cplusplus
#error This is a C++ header.
#endif

// printf on Win32 doesn't support long double, but __mingw_sprintf does if
// compiling with MinGW. So we use it. Cygwin printf is good for long double.
#undef LDBL_GOOD
#if defined(_WIN32) && !defined(__CYGWIN__)
#ifdef __MINGW32__
#define LDBL_GOOD(x) __mingw_##x  /* E.g. __mingw_sprintf. */
#endif
#else
#define LDBL_GOOD(x) x
#endif

#endif  // R_LDBL_GOOD_H
