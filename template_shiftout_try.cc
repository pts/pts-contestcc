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

class StringWritable {
 public:
  // This is needed for convenience in my_string << dump(42).
  StringWritable(std::string &str): str_(&str) {}  // !! needed?
  StringWritable(std::string *str): str_(assume_notnull(str)) {}
  std::string *str() const { return str_; }
  const char *c_str() const { return str_->c_str(); }
  //virtual void vi_write(const void *p, uintptr_t size) {
  //  str_->append(static_cast<const char *>(p), size);
  //}
  //virtual void vi_putc(char c) {
  //  str_->push_back(c);
  //}
 private:
  std::string *str_;
};

class C {
 public:
  C() { printf("+C\n"); }
  ~C() { printf("~C\n"); }
  C(const C&) { printf("*C\n"); }
  C& operator=(const C&) { printf("=C\n"); return *this; }
};

template<class V>class TFormatter {};

template<>struct TFormatter<int> {
  typedef void *tag_type;
  static void format(int v) {
    printf("format<int>(%d)\n", v);
  }
};

template<>struct TFormatter<const char*> {
  typedef void *tag_type;
  static void format(const char *v) {
    printf("format<const char*>(%s)\n", v);
  }
};

template<>struct TFormatter<const C&> {
  typedef void *tag_type;
  static void format(const C&) {
    printf("format<C>(...)\n");
  }
};

//template<>struct TFormatter<bool> {
//  typedef void *tag_type;
//};

template<class First, class Second>struct TypePair {
  typedef First first_type;
  typedef Second second_type;
};

template<class T>class TWritable {};

template<>struct TWritable<StringWritable> {
  typedef const StringWritable &constref_type;
};

template<>struct TWritable<FileObj> {
  typedef const FileObj &constref_type;
};

// Goal (1): Unify these two below, and make them write the int.
// Goal (2): Add support for writing `const char*', still unified.

// candidate template ignored: couldn't infer template argument 'T'
//template<class T>static inline typename TWritable<T>::constref_type operator<<(
//    typename TWritable<T>::constref_type wr, int) {

// The return type is just `const W&', but it's formulated in a way to prevent
// the template from matching if either TWritable<W> or TFormatter<V> is
// not specialized. This is a tricky use of
// http://en.wikipedia.org/wiki/SFINAE .
template<class W, class V>static inline
typename TypePair<typename TWritable<W>::constref_type,
                  typename TFormatter<const V&>::tag_type >::first_type
operator<<(const W &wr, const V &v) {
  (void)v;
  TFormatter<const V&>::format(v);
  // TFormatter::<V> format_short(V);
  //Formatter<T>::format(const_cast<StringWritable*>(&wr), t);  // TODO(pts): Fix const_cast.
  return wr;
}

template<class W, class V>static inline
typename TypePair<typename TWritable<W>::constref_type,
                  typename TFormatter<V>::tag_type >::first_type
operator<<(const W &wr, V v) {
  (void)v;
  TFormatter<V>::format(v);
  // TFormatter::<V> format_short(V);
  //Formatter<T>::format(const_cast<StringWritable*>(&wr), t);  // TODO(pts): Fix const_cast.
  return wr;
}

// This doesn't make (S) work, because std::string() is not an l-value.
// But makes `s << ...' work.
//
// The return type is just `StringWritable', but it's formulated in a way to
// prevent the template from matching if either TWritable<W> or
// TFormatter<V> is not specialized. This is a tricky use of
// http://en.wikipedia.org/wiki/SFINAE .
template<class V>static inline
typename TypePair<StringWritable,
                  typename TFormatter<V>::tag_type >::first_type
operator<<(std::string &str, V v) {
  StringWritable sw(&str);
  sw << v;
  return sw;
}
template<class V>static inline
typename TypePair<StringWritable,
                  typename TFormatter<const V&>::tag_type >::first_type
operator<<(std::string &str, const V &v) {
  StringWritable sw(&str);
  sw << v;
  return sw;
}

int main() {
  fprintf((FileObj(stdout) << 42 << -5).f(), ".\n");
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
  printf("</C>\n");
  s << "Foo";
  // FileObj(stdout) << true;  // Doesn't compile, TFormatter<bool> not defined.
  return 0;
}
