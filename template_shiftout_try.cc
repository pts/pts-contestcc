#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <string>

#include "r_status.h"

class FileObj {
 public:
  FileObj(FILE *f): f_(assume_notnull(f)) {}
  FILE *f() const { return f_; }
  operator FILE*() { return f_; }
  Status flush() const {
    return fflush(f_) == 0;
  }
  // TODO(pts): move to write(FILE *f, ...)?
  Status write(const void *p, uintptr_t size) const {
    // TODO(pts): Add proper error message. (Also elsewhere.)
    return fwrite(p, 1, size, f_) == size;
  }
  Status write(const char *msg) const {
    return write(msg, strlen(msg));
  }
  // Implements method from Writable.
  // TODO(pts): Move this to the .cc file.
  // !! TODO(pts): Don't add virtual methods, it's just overhead for each object.
  //virtual void vi_write(const void *p, uintptr_t size) {
  //  Status(fwrite(p, 1, size, f_) == size);
  //}
  // Implements method from Writable.
  // TODO(pts): Move this to the .cc file.
  //virtual void vi_putc(char c) {
  //  Status(putc(c, f_) > 0);
  //}

  // Most convenience functions are in `>>'.
 private:
  FILE *f_;  // Owned externally.
};

#if 0  // TODO(pts): Remove.
class StringWritable {
 public:
  // This is needed for convenience in my_string << dump(42).
  StringWritable(std::string &str): str_(&str) {}  // !! needed?
  StringWritable(std::string *str): str_(assume_notnull(str)) {}
  std::string *str() const { return str_; }
  const char *c_str() const { return str_->c_str(); }
  // !! TODO(pts): Don't add virtual methods, it's just overhead for each object.
  //virtual void vi_write(const void *p, uintptr_t size) {
  //  str_->append(static_cast<const char *>(p), size);
  //}
  //virtual void vi_putc(char c) {
  //  str_->push_back(c);
  //}
 private:
  std::string *str_;
};
#endif

class C {
 public:
  C() { printf("+C\n"); }
  ~C() { printf("~C\n"); }
  C(const C&) { printf("*C\n"); }
  C& operator=(const C&) { printf("=C\n"); return *this; }
};

class D {
 public:
  D() { printf("+D\n"); }
  ~D() { printf("~D\n"); }
  D(const D&) { printf("*D\n"); }
  D& operator=(const D&) { printf("=D\n"); return *this; }

  typedef void *format_append_type;  // Corresponds to format_append.
  void format_append(std::string *out) const {
    out->append("D()");
  }
};

template<class V>class TFormatter {};

template<>class TFormatter<bool> {
 public:
  enum max_type { max_buf_size = 5 };  // Including the trailing \0.
  static void format(bool v, char *buf) {
    strcpy(buf, v ? "true" : "false");
  }
};

template<>class TFormatter<int> {
 public:
  enum max_type { max_buf_size = 12 };
  static void format(int v, char *buf) {
    sprintf(buf, "%d", v);  // TODO(pts): Faster.
  }
};

struct Piece {
  inline Piece(const char *data, uintptr_t size)
      : data(data), size(size) {}
  const char *data;
  uintptr_t size;
};

template<>class TFormatter<const char*> {
 public:
  // TODO(pts): Write choose instructions.
  // If both piece_type and append_type are feasible, use append_type if
  // with piece_type you'd do dynamic memory allocation.
  typedef void *piece_type;
  static inline Piece format_piece(const char *v) {
    return Piece(v, strlen(v));
  }
};

template<>class TFormatter<std::string> {
 public:
  typedef void *piece_type;
  static inline Piece format_piece(const std::string &v) {
    return Piece(v.data(), v.size());
  }
};

template<>class TFormatter<const C&> {
 public:
  typedef void *append_type;
  static void format_append(const C&, std::string *out) {
    out->append("C()");
  }
};

//template<>struct TFormatter<bool> {
//  typedef void *max_type;
//};

template<class T>class TWritable {};

#if 0  // TODO(pts): Remove.
template<>struct TWritable<std::string> {
  typedef void *tag_type;
  static inline void write(const char *data, uintptr_t size,
                           const std::string &wr) {
    // This const_cast is needed. Without fake consts we'd need a
    // helper class (e.g. StringWritable) and extra templates.
    const_cast<std::string&>(wr).append(data, size);
  }
  static inline void write(const char *msg, const std::string &wr) {
    const_cast<std::string&>(wr).append(msg);
  }
};
#endif

template<>struct TWritable<FileObj> {
  typedef void *tag_type;
  static inline void write(const char *data, uintptr_t size,
                           const FileObj &wr) {
    wr.write(data, size);  // TODO(pts): Move `Status' away from inline.
  }
  static inline void write(const char *msg, const FileObj &wr) {
    wr.write(msg);  // TODO(pts): Move `Status' away from inline.
  }
};

template<class First, class Second, class Third>struct TypeTriplet {
  typedef First first_type;
  typedef Second second_type;
  typedef Third third_type;
};

template<class First, class Second>struct TypePair {
  typedef First first_type;
  typedef Second second_type;
};

// --- Generic writing to TWritable<W>.
//
// For the functions below, the return type is just `const W&', but it's
// formulated in a way to prevent the template from matching if either
// TWritable<W> or TFormatter<V> is not specialized. This is a tricky use of
// http://en.wikipedia.org/wiki/SFINAE .

template<class W, class V>static inline
typename TypeTriplet<const W&, typename TWritable<W>::tag_type,
                     typename TFormatter<const V&>::max_type>::first_type
operator<<(const W &wr, const V &v) {
  char buf[TFormatter<const V&>::max_buf_size];
  TFormatter<const V&>::format(v, buf);
  TWritable<W>::write(buf, wr);
  return wr;
}

template<class W, class V>static inline
typename TypeTriplet<const W&, typename TWritable<W>::tag_type,
                     typename TFormatter<V>::max_type>::first_type
operator<<(const W &wr, V v) {
  char buf[TFormatter<V>::max_buf_size];
  TFormatter<V>::format(v, buf);
  TWritable<W>::write(buf, wr);
  return wr;
}

template<class W, class V>static inline
typename TypeTriplet<const W&, typename TWritable<W>::tag_type,
                     typename TFormatter<const V&>::piece_type>::first_type
operator<<(const W &wr, const V &v) {
  Piece piece(TFormatter<const V&>::format_piece(v));
  TWritable<W>::write(piece.data, piece.size, wr);
  return wr;
}

template<class W, class V>static inline
typename TypeTriplet<const W&, typename TWritable<W>::tag_type,
                     typename TFormatter<V>::piece_type>::first_type
operator<<(const W &wr, V v) {
  Piece piece(TFormatter<V>::format_piece(v));
  TWritable<W>::write(piece.data, piece.size, wr);
  return wr;
}

template<class W, class V>static inline
typename TypeTriplet<const W&, typename TWritable<W>::tag_type,
                     typename TFormatter<const V&>::append_type>::first_type
operator<<(const W &wr, const V &v) {
  std::string str;
  TFormatter<const V&>::format_append(v, &str);
  TWritable<W>::write(str.data(), str.size(), wr);
  return wr;
}

template<class W, class V>static inline
typename TypeTriplet<const W&, typename TWritable<W>::tag_type,
                     typename TFormatter<V>::append_type>::first_type
operator<<(const W &wr, V v) {
  std::string str;
  TFormatter<V>::format_append(v, &str);
  TWritable<W>::write(str.data(), str.size(), wr);
  return wr;
}

template<class W, class V>static inline
typename TypeTriplet<const W&, typename TWritable<W>::tag_type,
                     typename V::format_append_type>::first_type
operator<<(const W &wr, const V &v) {
  std::string str;
  v.format_append(&str);
  TWritable<W>::write(str.data(), str.size(), wr);
  return wr;
}

// --- Writing to std::string.
//
// We could implement a TWritable<std::string>, but some of the
// implementations below are a bit more optimized. They also don't need
// const_cast.

template<class V>static inline
typename TypePair<std::string&,
                  typename TFormatter<const V&>::max_type>::first_Type
operator<<(std::string &wstr, const V &v) {
  char buf[TFormatter<const V&>::max_buf_size];
  TFormatter<const V&>::format(v, buf);
  wstr.append(buf);
  return wstr;
}

template<class V>static inline
typename TypePair<std::string&,
                  typename TFormatter<V>::max_type>::first_type
operator<<(std::string &wstr, V v) {
  char buf[TFormatter<V>::max_buf_size];
  TFormatter<V>::format(v, buf);
  wstr.append(buf);
  return wstr;
}

template<class V>static inline
typename TypePair<std::string&,
                  typename TFormatter<const V&>::piece_type>::first_type
operator<<(std::string &wstr, const V &v) {
  Piece piece(TFormatter<const V&>::format_piece(v));
  wstr.append(piece.data, piece.size);
  return wstr;
}

template<class V>static inline
typename TypePair<std::string&,
                  typename TFormatter<V>::piece_type>::first_type
operator<<(std::string &wstr, V v) {
  Piece piece(TFormatter<V>::format_piece(v));
  wstr.append(piece.data, piece.size);
  return wstr;
}

template<class V>static inline
typename TypePair<std::string&,
                  typename TFormatter<const V&>::append_type>::first_type
operator<<(std::string &wstr, const V &v) {
  TFormatter<const V&>::format_append(v, &wstr);  // Optimized implementation.
  return wstr;
}

template<class V>static inline
typename TypePair<std::string&,
                  typename TFormatter<V>::append_type>::first_type
operator<<(std::string &wstr, V v) {
  TFormatter<V>::format_append(v, &wstr);  // Optimized implementation.
  return wstr;
}

template<class V>static inline
typename TypePair<std::string&,
                  typename V::format_append_type>::first_type
operator<<(std::string &wstr, const V &v) {
  v.format_append(&wstr);  // Optimized implementation.
  return wstr;
}

// ---

int main() {
  // To make FileObj(stdout) or `fo(stdout)' work, we need `const W&'.
  fprintf((FileObj(stdout) << 42 << -5 << C()).f(), ".\n");
  // SUXX: No way to make it work like this.
  // This doesn't work without the explicit operator<<(std:: string &,...).
  // printf("%s!\n", (std::string() << 42 << -5).c_str()); // (S).
  std::string s;
  // This wouldn't work if operator<< accepted `StringWritable&' instead of
  // `const StringWritable&'.
  printf("%s;\n", (s << 42 << -5).c_str());
  C c;
  printf("<C>\n");
  const C &cr(c);
  s << c;   // No copy of C.
  s << cr;  // No copy of C.
  s << D();
  printf("</C>\n");
  s << "Foo" << std::string("Bar");
  FileObj(stdout) << true;
  // FileObj(stdout) << 4.5;  // Doesn't compile, TFormatter<double> not defined.
  printf("S=(%s)\n", s.c_str());
  return 0;
}
