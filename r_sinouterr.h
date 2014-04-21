#ifndef R_SINOUTERR_H
#define R_SINOUTERR_H 1

#ifndef __cplusplus
#error This is a C++ header.
#endif

#include <stdio.h>

struct Sin { inline operator FILE*() const { return stdin; } };
extern Sin sin;  // No need to actually define it anywhere.

struct Sout { inline operator FILE*() const { return stdout; } };
extern Sout sout;  // No need to actually define it anywhere.

struct Serr { inline operator FILE*() const { return stderr; } };
extern Serr serr;  // No need to actually define it anywhere.

template<class T>class TStdStream {};
template<>struct TStdStream<Sin>  { typedef void *tag_type; };
template<>struct TStdStream<Sout> { typedef void *tag_type; };
template<>struct TStdStream<Serr> { typedef void *tag_type; };

#endif  // R_SINOUTERR_H