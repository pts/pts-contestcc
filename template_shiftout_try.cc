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
  StringWritable(std::string &str): str_(&str) {}
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

// Goal (1): Unify these two below, and make them write the int.
// Goal (2): Add support for writing `const char*', still unified.

//template<class T>static inline typename Formatter<T>::return_type
const StringWritable &operator<<(const StringWritable &wr, int) {
  //Formatter<T>::format(const_cast<StringWritable*>(&wr), t);  // TODO(pts): Fix const_cast.
  return wr;
}

const FileObj &operator<<(const FileObj &wr, int) {
  //Formatter<T>::format(const_cast<StringWritable*>(&wr), t);  // TODO(pts): Fix const_cast.
  return wr;
}

// This doesn't make (S) work, because std::string() is not an l-value.
//StringWritable operator<<(std::string &str, int) {
//  StringWritable sw(str);
//  // !! ...
//  return sw;
//}

int main() {
  fprintf((FileObj(stdout) << 42 << -5).f(), ".\n");
  // SUXX: No way to make it work like this.
  // This doesn't work without the explicit operator<<(std:: string &,...).
  // printf("%s!\n", (std::string() << 42 << -5).c_str()); // (S).
  std::string s;
  // This wouldn't work if operator<< accepted `StringWritable&' instead of
  // `const StringWritable&'.
  printf("%s;\n", (s << 42 << -5).c_str());
  return 0;
}
