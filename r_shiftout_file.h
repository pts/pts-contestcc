#ifndef R_SHIFTOUT_FILE_H
#define R_SHIFTOUT_FILE_H 1

#include <stdio.h>

#include "r_die.h"
#include "r_fileobj.h"
#include "r_filewrapper.h"
#include "r_sinouterr.h"
#include "r_tfilewrapper.h"
#include "r_tformatter.h"
#include "r_twritable.h"
#include "r_typetuple.h"

// TODO(pts): Does calling ferror autoflush? If so, disable it.

namespace r {

class FileShiftout {
 public:
  // Unfortunately we need these Sin, Sout, Serr constructors for `operator<<'
  // on FileShiftout, because C++ won't do a 2-step conversion: Sout --> FILE*
  // --> FileShifout.
  // TODO(pts): Does `const Sout&' or `Sout' generate more efficient code?
  // TODO(pts): Can we use TStdStream somehow?
  inline FileShiftout(const Sin&): f_(stdin) {}
  inline FileShiftout(const Sout&): f_(stdout) {}
  inline FileShiftout(const Serr&): f_(stderr) {}
  // error: default template arguments may not be used in function templates without -std=c++0x or -std=gnu++0x
  // template<class S, class U = typename TStdStream<S>::tag_type>inline FileShiftout(const S &s): f_(s) {}
  inline FileShiftout(FILE *f): f_(assume_notnull(f)) {}
  inline FileShiftout(const FileObj &fo): f_(fo.f()) {}
  inline ~FileShiftout() { if (ferror(f_)) die_on_error(); }
  // inline operator FileWrapper() const { return FileWrapper(f_); }
  inline FILE *f() const { return f_; }
  // This would make a call ambiguous.
  inline operator FILE*() const { return f_; }
 private:
  static void die_on_error();
  FILE *f_;  // Owned externally.
};

template<>struct TWritable<FileWrapper> {
  typedef void *tag_type;
  // Not using `const FileWrapper&', because FileWrapper is small.
  static inline void write(const void *data, uintptr_t size,
                           FileWrapper wr) {
    // Don't check the return value, FileShiftout will check ferror(...).
    fwrite(data, 1, size, wr.f);
  }
  static inline void write(const char *msg, FileWrapper wr) {
    // Don't check the return value, FileShiftout will check ferror(...).
    fputs(msg, wr.f);
  }
  static inline void write(char c, const FileWrapper wr) {
    // Don't check the return value, FileShiftout will check ferror(...).
    putc(c, wr.f);
  }
};

// --- Flushing using operator<<.
//
// Usage: `stdout << flush'.

class Flush {};
extern Flush flush;

template<class W>static inline
typename TypePair<const FileWrapper&,
                  typename TFileWrapper<W>::tag_type >::first_type
operator<<(const W &wf, Flush) {
  fflush(wf.f);
  return wf;
}

// --- Writing to FileShiftout.
//
// This includes automatic error checking at the end of the output.

// Helper template and specialization for keeping the operator<< with `char v'
// argument a template, for symmetry and error reporting.
template<class T>class TFileShiftout {};
template<>struct TFileShiftout<FileShiftout> {
  typedef void *tag_type;
};

// Made this a template for symmetry of error reporting.
template<class V>static inline
typename TypePair<FileWrapper, typename TXChar<V>::tag_type>::first_type
operator<<(const FileShiftout &fwr, V v) {
  FileWrapper wr(fwr.f());
  wr << v;
  return wr;
}

template<class V>static inline
typename TypePair<FileWrapper,
                  typename TFormatter<const V&>::tag_type>::first_type
operator<<(const FileShiftout &fwr, const V &v) {
  FileWrapper wr(fwr.f());
  wr << v;
  return wr;
}

template<class V>static inline
typename TypePair<FileWrapper,
                  typename TFormatter<V>::tag_type>::first_type
operator<<(const FileShiftout &fwr, V v) {
  FileWrapper wr(fwr.f());
  wr << v;
  return wr;
}

template<class V>static inline
typename TypePair<FileWrapper,
                  typename V::format_append_type>::first_type
operator<<(const FileShiftout &fwr, const V &v) {
  FileWrapper wr(fwr.f());
  wr << v;
  return wr;
}

}  // namespace r

#endif  // R_SHIFTOUT_FILE_H
