#ifndef R_DUMP_H
#define R_DUMP_H 1

#ifndef __cplusplus
#error This is a C++ header.
#endif

#include "r_stringwritable.h"
#include "r_shiftout.h"
#include "r_fileobj.h"
#include "r_basic_formatters.h"

#include <string>
#include <vector>
// TODO(pts): Can we provide wrdump(..., const std::set<T> &a) only if
// std::set is available (e.g. after `#include <set>')?

#include <stdio.h>
#include <stdint.h>

// TODO(pts): Add dumping of more types.

// TODO(pts): Add most of the implementations to the .cc file -- or at least
// share if included multiple times.
void wrdump(Writable *wr, bool b) {
  const char *msg = b ? "true" : "false";
  wr->vi_write(msg, strlen(msg));
}

void wrdump(Writable *wr, int i) {
  char tmp[sizeof(int) * 3 + 1];
  sprintf(tmp, "%d", i);
  wr->vi_write(tmp, strlen(tmp));
}

template<typename T>void wrdump(Writable *wr, const T *a, uintptr_t size) {
  wr->vi_putc('{');
  if (size != 0) {
    wrdump(wr, a[0]);
    for (uintptr_t i = 1; i < size; ++i) {
      wr->vi_putc(',');
      wr->vi_putc(' ');
      wrdump(wr, a[i]);
    }
  }
  wr->vi_putc('}');
}

template<typename T, uintptr_t S>void wrdump(Writable *wr, const T (&a)[S]) {
  wrdump(wr, a, S);
}

template<typename T>void wrdump(Writable *wr, const std::vector<T> &a) {
  wrdump(wr, a.data(), a.size());
}

template<typename T>void dump_buffnl(FILE *f, const T &t) {
  std::string out;
  StringWritable swout(&out);
  wrdump(&swout, t);
  out.push_back('\n');
  Status(fwrite(out.data(), 1, out.size(), f) == out.size());
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
 private:
  mutable bool is_dumped_;
  const T &t_;
};

template<class T>void dump(const char *msg, const T &t) {
  std::string out(msg);
  // Buffering to a string to reduce stderr autoflush problems.
  StringWritable swout(&out);
  wrdump(&swout, t);
  out.push_back('\n');
  Status(fwrite(out.data(), 1, out.size(), stderr) == out.size());
}
template<class T1, class T2>void dump(const char *msg, const T1 &t1, const T2 &t2) {
  std::string out(msg);
  StringWritable swout(&out);
  wrdump(&swout, t1);
  out.append(", ");
  wrdump(&swout, t2);
  out.push_back('\n');
  Status(fwrite(out.data(), 1, out.size(), stderr) == out.size());
}
template<class T1, class T2, class T3>void dump(const char *msg, const T1 &t1, const T2 &t2, const T3 &t3) {
  std::string out(msg);
  StringWritable swout(&out);
  wrdump(&swout, t1);
  out.append(", ");
  wrdump(&swout, t2);
  out.append(", ");
  wrdump(&swout, t3);
  out.push_back('\n');
  Status(fwrite(out.data(), 1, out.size(), stderr) == out.size());
}
template<class T1, class T2, class T3, class T4>void dump(const char *msg, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4) {
  std::string out(msg);
  StringWritable swout(&out);
  wrdump(&swout, t1);
  out.append(", ");
  wrdump(&swout, t2);
  out.append(", ");
  wrdump(&swout, t3);
  out.append(", ");
  wrdump(&swout, t4);
  out.push_back('\n');
  Status(fwrite(out.data(), 1, out.size(), stderr) == out.size());
}
// TODO(pts): Add even more arguments (T5, T6 etc.).

template<class T>Dumper<T> dump(const T &t) { return Dumper<T>(t); }

template<class T>struct Formatter<Dumper<T> > {
  FORMATTER_COMMON_DECLS
  static inline void format(Writable *wr, const Dumper<T> &d) {
    wrdump(wr, d.release());
  }
};

#endif  // R_DUMP_H
