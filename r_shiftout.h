#ifndef R_SHIFTOUT_H
#define R_SHIFTOUT_H 1

#include "r_formatter.h"

class StringWritable;
class FileObj;
class Writable;

// TODO(pts): Make these fewer.
//
// `s << dump(42)' doesn't work if we have Writable argument here.
// `s << dump(42)' doesn't work if we have non-const reference here.
//
// Formatter<T>::return_type is tricky here, because that's not defined for
// types T without a Formatter<T>, so the compiler will just silently skip
// over this template function.
template<class T>static inline typename Formatter<T>::return_type
operator<<(const StringWritable &wr, const T &t) {
  Formatter<T>::format(const_cast<StringWritable*>(&wr), t);  // TODO(pts): Fix const_cast.
  return wr;
}
template<class T>static inline typename Formatter<T*>::return_type
operator<<(const StringWritable &wr, const T *t) {
  Formatter<T*>::format(const_cast<StringWritable*>(&wr), t);  // TODO(pts): Fix const_cast.
  return wr;
}
template<class T> static inline typename Formatter<T>::return_type
operator<<(const FileObj &wr, const T &t) {
  Formatter<T>::format(const_cast<FileObj*>(&wr), t);  // TODO(pts): Fix const_cast.
  return wr;
}
template<class T>static inline typename Formatter<T*>::return_type
operator<<(const FileObj &wr, const T *t) {
  Formatter<T*>::format(const_cast<FileObj*>(&wr), t);  // TODO(pts): Fix const_cast.
  return wr;
}
template<class T>static inline typename Formatter<T>::return_type
operator<<(const Writable &wr, const T &t) {
  Formatter<T>::format(const_cast<Writable*>(&wr), t);  // TODO(pts): Fix const_cast.
  return wr;
}
template<class T>static inline typename Formatter<T*>::return_type
operator<<(const Writable &wr, const T *t) {
  Formatter<T*>::format(const_cast<Writable*>(&wr), t);  // TODO(pts): Fix const_cast.
  return wr;
}

#endif // R_SHIFTOUT_H
