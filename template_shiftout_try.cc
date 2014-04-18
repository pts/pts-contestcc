#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <string>

#include "r_fileobj.h"
#include "r_status.h"

// We need this class for operator<<, because FILE* is not an user-defined
// type.
struct FileWrapper {
  inline FileWrapper(FILE *f): f(assume_notnull(f)) {}
  inline operator FILE*() const { return f; }
  // Don't add more fields, so FileWrapper can be copied easily.
  FILE *f;
};

class FileWritable {
 public:
  inline FileWritable(FILE *f): f_(assume_notnull(f)) {}
  inline FileWritable(const FileObj &fo): f_(fo.f()) {}
  inline ~FileWritable() { if (ferror(f_)) die_on_error(); }
  inline operator FileWrapper() const { return FileWrapper(f_); }
  inline FILE *f() const { return f_; }
  // This would make a call ambiguous.
  // inline operator FILE*() const { return f_; }
 private:
  static void die_on_error();
  FILE *f_;  // Owned externally.
};

void FileWritable::die_on_error() {
  die("Error writing to FILE* by operator<<.");
}

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

template<class T>class TWritable {};

template<>struct TWritable<FileWrapper> {
  typedef void *tag_type;
  // Not using `const FileWrapper&', because FileWrapper is small.
  static inline void write(const void *data, uintptr_t size,
                           FileWrapper wr) {
    // Don't check the return value, FileWritable will check ferror(...).
    fwrite(data, 1, size, wr.f);
  }
  static inline void write(const char *msg, FileWrapper wr) {
    // Don't check the return value, FileWritable will check ferror(...).
    fputs(msg, wr.f);
  }
  static inline void write(char c, const FileWrapper wr) {
    // Don't check the return value, FileWritable will check ferror(...).
    putc(c, wr.f);
  }
};

template<class T>class TString {};

template<>struct TString<std::string> {
  typedef void *tag_type;
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

// TODO(pts): Check how much slower this is than manual calls to putc + ferror.
// TODO(pts): Speed it up as putchar for stdout. Is it faster?
template<class W>static inline
typename TypePair<const W&, typename TWritable<W>::tag_type>::first_type
operator<<(const W &wr, char v) {
  TWritable<W>::write(v, wr);
  return wr;
}

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

// Made this a template for symmetry of error reporting.
template<class W>static inline
typename TypePair<std::string&, typename TString<W>::tag_type >::first_type
operator<<(W &wstr, char v) {
  wstr.push_back(v);  // Optimized implementation.
  return wstr;
}

template<class V>static inline
typename TypePair<std::string&,
                  typename TFormatter<const V&>::max_type>::first_type
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

// Writing to FileWritable.

template<class V>static inline
typename TypePair<FileWrapper,
                  typename TFormatter<const V&>::max_type>::first_type
operator<<(const FileWritable &fwr, const V &v) {
  FileWrapper wr(fwr);
  wr << v;
  return wr;
}

template<class V>static inline
typename TypePair<FileWrapper,
                  typename TFormatter<V>::max_type>::first_type
operator<<(const FileWritable &fwr, V v) {
  FileWrapper wr(fwr);
  wr << v;
  return wr;
}

// ---

int main() {
  // To make FileWritable(stdout) or `fo(stdout)' work, we need `const W&'.
  fprintf(FileWritable(stdout) << 42 << ',' << -5 << C(), ".\n");
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
  s << '+' << cr;  // No copy of C.
  s << D();
  printf("</C>\n");
  s << "Foo" << std::string("Bar");
  FileWritable(stdout) << true;
  // FileObj(stdout) << 4.5;  // Doesn't compile, TFormatter<double> not defined.
  printf("S=(%s)\n", s.c_str());
  return 0;
}
