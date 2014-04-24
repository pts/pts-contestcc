#ifndef R_DIE_H
#define R_DIE_H 1

namespace r {

void die(const char *msg);

template<class T>static inline T *notnull(T *t) {
  if (!t) die("NULL pointer found.");
  return t;
}

template<class T>static inline const T *notnull(const T *t) {
  if (!t) die("NULL pointer found.");
  return t;
}

#define assume_notnull(t) (t)

}  // namespace r

#endif  // R_DIE_H
