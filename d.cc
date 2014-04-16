#include "r_fileobj.h"

#include <string>
#include <vector>

#include <stdio.h>
#include <stdint.h>

class Writable {
 public:
  virtual ~Writable() {}
  virtual void vi_write(const void *p, uintptr_t size) = 0;
  virtual void vi_putc(char c) = 0;
};

class FileWritable: public Writable {
 public:
  FileWritable(FILE *f): f_(f) {}
  virtual void vi_write(const void *p, uintptr_t size) {
    fwrite(p, 1, size, f_);  // !! Status
  }
  virtual void vi_putc(char c) {
    putc(c, f_);  // !! Status
  }
 private:
  FILE *f_;
};

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

void fdump(FILE *f, bool b) {
  fprintf(f, "%s", b ? "true" : " false");
}

void fdump(FILE *f, int i) {
  fprintf(f, "%d", i);
}

template<typename T>void fdump(FILE *f, const T *a, uintptr_t size) {
  putc('{', f);
  if (size != 0) {
    fdump(f, a[0]);
    for (uintptr_t i = 1; i < size; ++i) {
      putc(',', f);
      putc(' ', f);
      fdump(f, a[i]);
    }
  }
  putc('}', f);
}

template<typename T, uintptr_t S>void fdump(FILE *f, const T (&a)[S]) {
  fdump(f, a, S);
}

template<typename T>void fdump(FILE *f, const std::vector<T> a) {
  fdump(f, a.data(), a.size());
}

template<class T>class Dumper {
 public:
  Dumper(const T &t): is_dumped_(false), t_(t) {}
  ~Dumper() {
    if (!is_dumped_) {
      // TODO(pts): Dump without flushing?
      dump(stderr);
      putc('\n', stderr);
    }
  }
  void dump(FILE *f) const {
    is_dumped_ = true;
    fdump(f, t_);
  }
 private:
  mutable bool is_dumped_;
  const T &t_;
};

template<class T>Dumper<T> dump(const T &t) { return Dumper<T>(t); }
template<class T>
const FileObj &operator<<(const FileObj &fo, const Dumper<T> &d) {
  d.dump(fo.f());
  return fo;
}
// `s << dump(42)' doesn't work if we have Writable argument here.
// `s << dump(42)' doesn't work if we have non-const reference here.
template<class T>
const Writable &operator<<(const StringWritable &wr, const Dumper<T> &d) {
  (void)d;
  return wr;
}

int main() {
  int a[3] = {55, 66, 77};
  std::vector<int> b;
  b.resize(3);
  b[2] = 44;

  dump(true);
  dump(-42);
  dump(a);
  dump(b);

  // TODO(pts): stdout << dump(42) << "\n";
  std::string s; s << dump(42);  // !! << ".";
  stdout << dump(42) << dump(-6);
  printf("\n");

  return 0;
}
