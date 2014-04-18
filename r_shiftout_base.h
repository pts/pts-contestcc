#ifndef R_SHIFTOUT_BASE_H
#define R_SHIFTOUT_BASE_H 1

#include <string>

#include "r_str_piece.h"
#include "r_tformatter.h"
#include "r_tstring.h"
#include "r_twritable.h"
#include "r_typetuple.h"

// --- Generic writing to TWritable<W>.
//
// The plan is that the C++ compiler does the type inference, and its
// optimizer elminates most of the boilerplate code.
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
                     typename TFormatter<const V&>::str_piece_type>::first_type
operator<<(const W &wr, const V &v) {
  const StrPiece str_piece(TFormatter<const V&>::format_str_piece(v));
  TWritable<W>::write(str_piece.data, str_piece.size, wr);
  return wr;
}

template<class W, class V>static inline
typename TypeTriplet<const W&, typename TWritable<W>::tag_type,
                     typename TFormatter<V>::str_piece_type>::first_type
operator<<(const W &wr, V v) {
  const StrPiece str_piece(TFormatter<V>::format_str_piece(v));
  TWritable<W>::write(str_piece.data, str_piece.size, wr);
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
                  typename TFormatter<const V&>::str_piece_type>::first_type
operator<<(std::string &wstr, const V &v) {
  const StrPiece str_piece(TFormatter<const V&>::format_str_piece(v));
  wstr.append(str_piece.data, str_piece.size);
  return wstr;
}

template<class V>static inline
typename TypePair<std::string&,
                  typename TFormatter<V>::str_piece_type>::first_type
operator<<(std::string &wstr, V v) {
  const StrPiece str_piece(TFormatter<V>::format_str_piece(v));
  wstr.append(str_piece.data, str_piece.size);
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

#endif // R_SHIFTOUT_BASE_H
