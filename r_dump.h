#ifndef R_DUMP_H
#define R_DUMP_H 1

#ifndef __cplusplus
#error This is a C++ header.
#endif

#include "r_shiftout.h"
#include "r_tformatter_basic.h"

#include <string>
#include <vector>
// TODO(pts): Can we provide wrdump(..., const std::set<T> &a) only if
// std::set is available (e.g. after `#include <set>')?

#include <stdio.h>
#include <stdint.h>

// We don't care too much about performane (hence no `static inline' for
// functions etc.), because dumping is for debugging.

// TODO(pts): Add dumping of more types.

// Making it a template so that multiple .cc files can define it.
template<class T>void wrdump(T, std::string *out);

// TODO(pts): Add most of the implementations to the .cc file -- or at least
// share if included multiple times.
template<>void wrdump(bool v, std::string *out) {
  out->append(v ? "true" : "false");
}

template<>void wrdump(int v, std::string *out) {
  char buf[TFormatter<int>::max_buf_size];
  TFormatter<int>::format(v, buf);
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

template<class T, uintptr_t S>void wrdump(const T (&v)[S], std::string *out) {
  wrdump(v, S, out);
}

// TODO(pts): Move to r_stl_formatters.h.
template<class T>void wrdump(const std::vector<T> &v, std::string *out) {
  wrdump(v.data(), v.size(), out);
}

template<class T>void dump_buffnl(FILE *f, const T &t) {
  std::string out;
  wrdump(t, &out);
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
