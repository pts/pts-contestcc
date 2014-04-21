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
#include <utility>
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

template<class T>struct TDumper<std::queue<T> > {
  typedef void *tag_type;
  // Dumps in push order.
  static inline void dump(const std::queue<T> &v, std::string *out) {
    // Since there is no .begin(), we need to make a copy in order to dump.
    std::queue<T> copy(v);
    out->push_back('{');
    bool do_comma = false;
    while (!copy.empty()) {
      if (do_comma) out->append(", ");
      wrdump(copy.front(), out);
      copy.pop();
      do_comma = true;
    }
    out->push_back('}');
  }
};

template<class T>struct TDumper<std::priority_queue<T> > {
  typedef void *tag_type;
  // Dumps in increasing order.
  //
  // TODO(pts): Share the implementation with stack<T>.
  static inline void dump(const std::priority_queue<T> &v, std::string *out) {
    // Since there is no .begin(), we need to make a copy in order to dump.
    std::priority_queue<T> copy(v);
    std::vector<T> copyv;
    copyv.resize(copy.size());
    uintptr_t i = copyv.size();
    while (!copy.empty()) {  // Copy reversed.
      copyv[--i] = copy.top();
      copy.pop();
    }
    wrdump_ary(copyv.data(), copyv.size(), out);
  }
};

template<class T>struct TDumper<std::stack<T> > {
  typedef void *tag_type;
  // Dumps in push order.
  static inline void dump(const std::stack<T> &v, std::string *out) {
    // Since there is no .begin(), we need to make a copy in order to dump.
    std::stack<T> copy(v);
    std::vector<T> copyv;
    copyv.resize(copy.size());
    uintptr_t i = copyv.size();
    while (!copy.empty()) {  // Copy reversed.
      copyv[--i] = copy.top();
      copy.pop();
    }
    wrdump_ary(copyv.data(), copyv.size(), out);
  }
};

template<class T1, class T2>struct TDumper<std::pair<T1, T2> > {
  typedef void *tag_type;
  // Dumps in push order.
  static inline void dump(const std::pair<T1, T2> &v, std::string *out) {
    out->push_back('{');
    wrdump(v.first, out);
    out->append(", ");
    wrdump(v.second, out);
    out->push_back('}');
  }
};

// !! TODO(pts): map
// !! TODO(pts): multimap

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

// TODO(pts): Dump unordered_set.
// TODO(pts): Dump unordered_multiset.
// TODO(pts): Dump unordered_map.
// TODO(pts): Dump unordered_multimap.
// TODO(pts): Dump tuple (complicated because variadic).

#endif

#endif  // R_DUMP_STL_H
