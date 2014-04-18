#ifndef R_TSTRING_H
#define R_TSTRING_H 1

#ifndef __cplusplus
#error This is a C++ header.
#endif

template<class T>class TString {};

template<>struct TString<std::string> {
  typedef void *tag_type;
};
// Please don't add any other specializations.

#endif // R_TSTRING_H
