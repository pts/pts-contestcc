#include "r_fileobj.h"

#include <string>
#include <vector>

#include <stdio.h>
#include <stdint.h>

class StringWritable: public Writable {
 public:
  // This is needed for convenience in s << dump(42).
  StringWritable(std::string &str): str_(&str) {}
  StringWritable(std::string *str): str_(assume_notnull(str)) {}
  virtual void vi_write(const void *p, uintptr_t size) {
    str_->append(static_cast<const char *>(p), size);
  }
  virtual void vi_putc(char c) {
    str_->push_back(c);
  }
 private:
  std::string *str_;
};

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

template<typename T>void wrdump(Writable *wr, const std::vector<T> a) {
  wrdump(wr, a.data(), a.size());
}

// TODO(pts): Don't create a Dumper class instance per type.
template<class T>class Dumper {
 public:
  Dumper(const T &t): is_dumped_(false), t_(t) {}
  ~Dumper() {
    if (!is_dumped_) {
      // TODO(pts): Dump without flushing?
      FileObj fwstderr(stderr);  // TODO(pts): Do it without creating a variable.
      dump(&fwstderr);
      putc('\n', stderr);
    }
  }
  void dump(Writable *wr) const {
    is_dumped_ = true;
    wrdump(wr, t_);
  }
 private:
  mutable bool is_dumped_;
  const T &t_;
};

template<class T>Dumper<T> dump(const T &t) { return Dumper<T>(t); }

// TODO(pts): Make these fewer.
template<class T>
const Writable &operator<<(const Writable &wr, const Dumper<T> &d) {
  d.dump(const_cast<Writable*>(&wr));  // TODO(pts): Fix const_cast.
  return wr;
}
template<class T>
const Writable &operator<<(const FileObj &wr, const Dumper<T> &d) {
  d.dump(const_cast<FileObj*>(&wr));  // TODO(pts): Fix const_cast.
  return wr;
}
// `s << dump(42)' doesn't work if we have Writable argument here.
// `s << dump(42)' doesn't work if we have non-const reference here.
template<class T>
const Writable &operator<<(const StringWritable &wr, const Dumper<T> &d) {
  d.dump(const_cast<StringWritable*>(&wr));  // TODO(pts): Fix const_cast.
  return wr;
}

template<class T>class Formatter {};
//  // TODO(pts): Make it a compile error not to override.
//  void format(Writable *wr);  // Never implemented.
//};

//template<class T>class Dummy {};
//template<>class Dummy<const char*> { public: typedef int t; };

template<>class Formatter<char*> {
 public:
  typedef const Writable &return_type;
  static void format(Writable *wr, const char *msg) {
    wr->vi_write(msg, strlen(msg));
  }
};

template<>class Formatter<int> {
 public:
  typedef const Writable &return_type;
  static void format(Writable *wr, int v) {
    char tmp[sizeof(int) * 3 + 1];
    sprintf(tmp, "%d", v);
    wr->vi_write(tmp, strlen(tmp));
  }
};

// TODO(pts): Make these fewer.
//
// Formatter<T>::return_type is tricky here, because that's not defined for
// types T without a Formatter<T>, so the compiler will just silently skip
// over this template function.
template<class T>
typename Formatter<T>::return_type operator<<(const StringWritable &wr, const T &t) {
  Formatter<T>::format(const_cast<StringWritable*>(&wr), t);  // TODO(pts): Fix const_cast.
  return wr;
}
template<class T>
typename Formatter<T*>::return_type operator<<(const StringWritable &wr, const T *t) {
  Formatter<T*>::format(const_cast<StringWritable*>(&wr), t);  // TODO(pts): Fix const_cast.
  return wr;
}
template<class T>
typename Formatter<T>::return_type operator<<(const FileObj &wr, const T &t) {
  Formatter<T>::format(const_cast<FileObj*>(&wr), t);  // TODO(pts): Fix const_cast.
  return wr;
}
template<class T>
typename Formatter<T*>::return_type operator<<(const FileObj &wr, const T *t) {
  Formatter<T*>::format(const_cast<FileObj*>(&wr), t);  // TODO(pts): Fix const_cast.
  return wr;
}
template<class T>
typename Formatter<T>::return_type operator<<(const Writable &wr, const T &t) {
  Formatter<T>::format(const_cast<Writable*>(&wr), t);  // TODO(pts): Fix const_cast.
  return wr;
}
template<class T>
typename Formatter<T*>::return_type operator<<(const Writable &wr, const T *t) {
  Formatter<T*>::format(const_cast<Writable*>(&wr), t);  // TODO(pts): Fix const_cast.
  return wr;
}

int main() {
  char const *msg = "Bye";
  int a[3] = {55, 66, 77};
  std::vector<int> b;
  b.resize(3);
  b[2] = 44;

  dump(true);
  dump(-42);
  dump(a);
  dump(b);

  // TODO(pts): stdout << dump(42) << "\n";
  std::string s; s << dump(42) << dump(-5);  // !! << ".";
  // s << Formatter<const char*>(";.");  // Works.
  // s << Formatter(";.");  // Doesn't compile.
  s << ";." << "x" << msg << -42 << msg;
  // s << 12.34;
  printf("<%s>\n", s.c_str());
  // Unfortunately, `stdout << "HI"' will never compile, because both sides
  // of `<<` are basic types, so `operator<<' declarations are not
  // considered.
  FileObj(stdout) << "HI:" << dump(42) << dump(-6);
  stdout << dump(-7) << dump(-89);
  printf("\n");

  return 0;
}
