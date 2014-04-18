#ifndef R_SHIFTOUT_FILE_H
#define R_SHIFTOUT_FILE_H 1

#include <stdio.h>

#include "r_die.h"
#include "r_fileobj.h"
#include "r_filewrapper.h"
#include "r_tformatter.h"
#include "r_twritable.h"
#include "r_typetuple.h"

class FileShiftout {
 public:
  inline FileShiftout(FILE *f): f_(assume_notnull(f)) {}
  inline FileShiftout(const FileObj &fo): f_(fo.f()) {}
  inline ~FileShiftout() { if (ferror(f_)) die_on_error(); }
  inline operator FileWrapper() const { return FileWrapper(f_); }
  inline FILE *f() const { return f_; }
  // This would make a call ambiguous.
  // inline operator FILE*() const { return f_; }
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

// --- Writing to FileShiftout.
//
// This includes automatic error checking at the end of the output.

template<class V>static inline
typename TypePair<FileWrapper,
                  typename TFormatter<const V&>::tag_type>::first_type
operator<<(const FileShiftout &fwr, const V &v) {
  FileWrapper wr(fwr);
  wr << v;
  return wr;
}

template<class V>static inline
typename TypePair<FileWrapper,
                  typename TFormatter<V>::tag_type>::first_type
operator<<(const FileShiftout &fwr, V v) {
  FileWrapper wr(fwr);
  wr << v;
  return wr;
}

template<class V>static inline
typename TypePair<FileWrapper,
                  typename V::format_append_type>::first_type
operator<<(const FileShiftout &fwr, const V &v) {
  FileWrapper wr(fwr);
  wr << v;
  return wr;
}

#endif  // R_SHIFTOUT_FILE_H
