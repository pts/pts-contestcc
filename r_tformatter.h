#ifndef R_TFORMATTER_H
#define R_TFORMATTER_H 1

namespace r {

// TODO(pts): Add docs how to create specializations.
//
// TODO(pts): Write choose instructions.
// If both str_piece_type and append_type are feasible, use append_type if
// with str_piece_type you'd do dynamic memory allocation.
template<class V>class TFormatter {};

}  // namespace r

#endif // R_TFORMATTER_H
