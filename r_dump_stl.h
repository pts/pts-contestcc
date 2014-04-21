#ifndef R_DUMP_STL_H
#define R_DUMP_STL_H 1

#ifndef __cplusplus
#error This is a C++ header.
#endif

#include <deque>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <vector>
// TODO(pts): Can we provide wrdump(..., const std::set<T> &a) only if
// std::set is already available (e.g. after `#include <set>')?

#if __GXX_EXPERIMENTAL_CXX0X__ || __cplusplus >= 201100
#include <array>
#include <initializer_list>
#ifndef __clang__  // TODO(pts): Disable this with libsdc++-4.4.3 and Clang, compile error.
#include <forward_list>
#include <unordered_map>
#include <unordered_set>
#endif
#endif

#include "r_dump.h"

template<class T>struct TDumper<std::vector<T> > {
  typedef void *tag_type;
  static inline void dump(const std::vector<T> &v, std::string *out) {
    wrdump_ary(v.data(), v.size(), out);
  }
};
template<class T>struct TDumper<std::list<T> > {
  typedef void *tag_type;
  static inline void dump(const std::list<T> &v, std::string *out) {
    wrdump_forward(v, out);
  }
};
template<class T>struct TDumper<std::set<T> > {
  typedef void *tag_type;
  static inline void dump(const std::set<T> &v, std::string *out) {
    wrdump_forward(v, out);
  }
};
template<class T>struct TDumper<std::multiset<T> > {
  typedef void *tag_type;
  static inline void dump(const std::multiset<T> &v, std::string *out) {
    wrdump_forward(v, out);
  }
};

// !! TODO(pts): map
// !! TODO(pts): multimap

// !! TODO(pts): stack
// !! TODO(pts): queue
// !! TODO(pts): priority_queue
// !! TODO(pts): std::stack, std::queue and std::priority_queue can't be dumped, they
// don't provide access to raw data.

#if __GXX_EXPERIMENTAL_CXX0X__ || __cplusplus >= 201100

template<class T, std::size_t N>struct TDumper<std::array<T, N> > {
  typedef void *tag_type;
  static inline void dump(const std::array<T, N> &v, std::string *out) {
    wrdump_ary(v.data(), v.size(), out);
  }
};
template<class T>struct TDumper<std::initializer_list<T> > {
  typedef void *tag_type;
  static inline void dump(const std::initializer_list<T> &v, std::string *out) {
    wrdump_ary(v.begin(), v.end() - v.begin(), out);
  }
};
#ifndef __clang__
template<class T>struct TDumper<std::forward_list<T> > {
  typedef void *tag_type;
  static inline void dump(const std::forward_list<T> &v, std::string *out) {
    wrdump_forward(v, out);
  }
};
#endif

// !! TODO(pts): unordered_set (C++11)
// !! TODO(pts): unordered_multiset (C++11)
// !! TODO(pts): unordered_map (C++11)
// !! TODO(pts): unordered_multimap (C++11)

#endif

#endif  // R_DUMP_STL_H
