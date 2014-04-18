#ifndef R_TYPETUPLE_H
#define R_TYPETUPLE_H 1

template<class First, class Second, class Third>struct TypeTriplet {
  typedef First first_type;
  typedef Second second_type;
  typedef Third third_type;
};

template<class First, class Second>struct TypePair {
  typedef First first_type;
  typedef Second second_type;
};

#endif // R_TYPETUPLE_H
