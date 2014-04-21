#ifndef R_DUMP_H
#define R_DUMP_H 1

#ifndef __cplusplus
#error This is a C++ header.
#endif

#include "r_shiftout.h"
#include "r_tformatter_basic.h"

#include <string>

#include <stdio.h>
#include <stdint.h>

#include "r_dump_basic.h"
#include "r_typetuple.h"

// We don't care too much about performane (hence no `static inline' for
// functions etc.), because dumping is for debugging.

// TODO(pts): !! Add dumping of std::string.

// Making wrdump a template even for non-parametric types such as signed char
// and string, so that g++ won't report the list of overloaded non-template
// functions.

// Don't declare this, it creates an alternative of the wrdump()s below.
// template<class T>void wrdump(T, std::string *out);

template<class T>class TDumpCustom {};
template<>struct TDumpCustom<char> { typedef void *tag_type; };
template<class T>static inline void wrdump(
    T v,
    typename TypePair<std::string*, typename TDumpCustom<T>::tag_type>
        ::first_type out) {
  wrdump_low(v, out);
}

// `char', `signed char' and `unsigned char' are distinct types.
template<class T>class TDump8 {};
template<>struct TDump8<signed char>   { typedef void *tag_type; };
template<>struct TDump8<unsigned char> { typedef void *tag_type; };
template<class T>static inline void wrdump(
    T v,
    typename TypePair<std::string*, typename TDump8<T>::tag_type>
        ::first_type out) {
  char buf[TFormatter<int>::max_buf_size];
  TFormatter<int>::format(v, buf);
  out->append(buf);
}

// Basic types which get dumped using a max_buf_size formatter.
template<class T>class TDumpBasic {};
template<>struct TDumpBasic<bool>               { typedef void *tag_type; };
template<>struct TDumpBasic<signed   short>     { typedef void *tag_type; };
template<>struct TDumpBasic<unsigned short>     { typedef void *tag_type; };
template<>struct TDumpBasic<signed   int>       { typedef void *tag_type; };
template<>struct TDumpBasic<unsigned int>       { typedef void *tag_type; };
template<>struct TDumpBasic<signed   long>      { typedef void *tag_type; };
template<>struct TDumpBasic<unsigned long>      { typedef void *tag_type; };
template<>struct TDumpBasic<signed   long long> { typedef void *tag_type; };
template<>struct TDumpBasic<unsigned long long> { typedef void *tag_type; };
template<>struct TDumpBasic<float>              { typedef void *tag_type; };
template<>struct TDumpBasic<double>             { typedef void *tag_type; };
template<>struct TDumpBasic<long double>        { typedef void *tag_type; };

// TODO(pts): Add most of the implementations to the .cc file -- or at least
// share if included multiple times.
template<class T>static inline void wrdump(
    T v,
    typename TypePair<std::string*, typename TDumpBasic<T>::tag_type>
        ::first_type out) {
  char buf[TFormatter<T>::max_buf_size];
  TFormatter<T>::format(v, buf);
  out->append(buf);
}

template<class T>void wrdump(const T *a, uintptr_t size, std::string *out) {
  out->push_back('{');
  if (size != 0) {
    wrdump(a[0], out);
    for (uintptr_t i = 1; i < size; ++i) {
      out->append(", ", 2);
      wrdump(a[i], out);
    }
  }
  out->push_back('}');
}

template<class T, uintptr_t S>static inline
void wrdump(const T (&v)[S], std::string *out) {
  wrdump(v, S, out);
}

// r_dump_stl.h defines instances of TDumpDataSize.
template<class T>class TDumpDataSize {};
template<class T>static inline void wrdump(
    const T &v,
    typename TypePair<std::string*, typename TDumpDataSize<T>::tag_type>
        ::first_type out) {
  wrdump(v.data(), v.size(), out);
}

// r_dump_stl.h defines instances of TDumpBeginEnd.
template<class T>class TDumpBeginEnd {};
template<class T>static inline void wrdump(
    const T &v,
    typename TypePair<std::string*, typename TDumpBeginEnd<T>::tag_type>
        ::first_type out) {
  // v.begin() and v.end() are pointers.
  wrdump(v.begin(), v.end() - v.begin(), out);
}

// r_dump_stl.h defines instances of TDumpForward.
template<class T>class TDumpForward {};
template<class T>static inline
void wrdump_forward(const T &v, std::string *out) {
  out->push_back('{');
  const typename T::const_iterator end = v.end();
  bool do_comma = false;
  for (typename T::const_iterator it = v.begin(); it != end; ++it) {
    if (do_comma) out->append(", ", 2);
    wrdump(*it, out);
    do_comma = true;
  }
  out->push_back('}');
}
template<class T>static inline void wrdump(
    const T &v,
    typename TypePair<std::string*, typename TDumpForward<T>::tag_type>
        ::first_type out) {
  wrdump_forward(v, out);
}

template<class T>void dump_buffnl(FILE *f, const T &t) {
  std::string out;
  wrdump(t, &out);
  out.push_back('\n');
  if (fwrite(out.data(), 1, out.size(), f) != out.size()) {
    die("Dumping failed.");
  }
}

// TODO(pts): Try to unify t_ and is_dumped_ using NULL.
template<class T>class Dumper {
 public:
  Dumper(const T &t): is_dumped_(false), t_(t) {}
  ~Dumper() {
    if (!is_dumped_) dump_buffnl(stderr, t_);
  }
  const T &release() const {
    is_dumped_ = true;
    return t_;
  }

  // For `s << dump(42)', cooperating with r_shiftout_base.h.
  typedef void *format_append_type;
  void format_append(std::string *out) const {
    is_dumped_ = true;
    wrdump(t_, out);
  }
 private:
  mutable bool is_dumped_;
  const T &t_;
};

template<class T>void dump(const char *msg, const T &t) {
  std::string out(msg);
  // Buffering to a string to reduce stderr autoflush problems.
  wrdump(t, &out);
  out.push_back('\n');
  Status(fwrite(out.data(), 1, out.size(), stderr) == out.size());
}
template<class T1, class T2>void dump(const char *msg, const T1 &t1, const T2 &t2) {
  std::string out(msg);
  wrdump(t1, &out);
  out.append(", ");
  wrdump(t2, &out);
  out.push_back('\n');
  Status(fwrite(out.data(), 1, out.size(), stderr) == out.size());
}
template<class T1, class T2, class T3>void dump(const char *msg, const T1 &t1, const T2 &t2, const T3 &t3) {
  std::string out(msg);
  wrdump(t1, &out);
  out.append(", ");
  wrdump(t2, &out);
  out.append(", ");
  wrdump(t3, &out);
  out.push_back('\n');
  Status(fwrite(out.data(), 1, out.size(), stderr) == out.size());
}
template<class T1, class T2, class T3, class T4>void dump(const char *msg, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4) {
  std::string out(msg);
  wrdump(t1, &out);
  out.append(", ");
  wrdump(t2, &out);
  out.append(", ");
  wrdump(t3, &out);
  out.append(", ");
  wrdump(t4, &out);
  out.push_back('\n');
  Status(fwrite(out.data(), 1, out.size(), stderr) == out.size());
}
// TODO(pts): Should we add even more arguments (T5, T6 etc.)? That may add
// too much code bloat to the generated binary.

// TODO(pts): Document this function more, it's user-visible.
template<class T>Dumper<T> dump(const T &t) { return Dumper<T>(t); }

#endif  // R_DUMP_H
