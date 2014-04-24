#ifndef R_SINOUTERR_H
#define R_SINOUTERR_H 1

#include <stdio.h>

namespace r {

struct Sin { inline operator FILE*() const { return stdin; } };
#ifdef __GNUC__
static Sin sin __attribute__((used));
#else
// This produces a link error with `g++ -O0'.
extern Sin sin;  // No need to actually define it anywhere.
#endif

struct Sout { inline operator FILE*() const { return stdout; } };
#ifdef __GNUC__
static Sout sout __attribute__((used));
#else
extern Sout sout;  // No need to actually define it anywhere.
#endif

struct Serr { inline operator FILE*() const { return stderr; } };
#ifdef __GNUC__
static Serr serr __attribute__((used));
#else
extern Serr serr;  // No need to actually define it anywhere.
#endif

template<class T>class TStdStream {};
template<>struct TStdStream<Sin>  { typedef void *tag_type; };
template<>struct TStdStream<Sout> { typedef void *tag_type; };
template<>struct TStdStream<Serr> { typedef void *tag_type; };

}  // namespace r

#endif  // R_SINOUTERR_H
