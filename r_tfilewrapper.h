#ifndef R_TFILEWRAPPER_H
#define R_TFILEWRAPPER_H 1

namespace r {

class FileWrapper;

// Helper template and specialization for keeping the operator<< with `flush'
// argument a template, for symmetry and error reporting.
template<class T>class TFileWrapper {};
template<>struct TFileWrapper<FileWrapper> { typedef void *tag_type; };

}  // namespace r

#endif  // R_TFILEWRAPPER_H
