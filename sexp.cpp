#include "sexp.hpp"

#include <cstdlib>
#include <cstring>

bool atomp(SExp sexp) {
  return sexp._tag != Tag::Pair;
}

bool integerp(SExp sexp) {
  return sexp._tag == Tag::Integer;
}

bool symbolp(SExp sexp) {
  return sexp._tag == Tag::Symbol;
}

bool null(SExp sexp) {
  return sexp._tag == Tag::Nil;
}

char* copy_str(char const* str) {
  // 現代のコードではない。あとでGCを書く。
  size_t len = std::strlen(str);
  char* new_str = static_cast<char*>(std::malloc(len + 1));
  std::strcpy(new_str, str);
  return new_str;
}

SExp make_Symbol(char const* str) {
  Value v;
  v.symbol = copy_str(str); // leak
  return SExp {
    Tag::Symbol,
    v,
  };
}

SExp make_Integer(int n) {
  Value v;
  v.integer = n;
  return SExp {
    Tag::Integer,
    v,
  };
}

SExp make_Nil() {
  return SExp {
    Tag::Nil,
    Value{},
  };
}

SExp const nil = make_Nil();

SExp cons(SExp car, SExp cdr) {
  Value v;
  v.pair = new Pair{ car, cdr }; // will leak
  return SExp {
    Tag::Pair,
    v,
  };
}

SExp car(SExp sexp) {
  assert(sexp._tag == Tag::Pair);
  return sexp._value.pair->_car;
}
SExp cdr(SExp sexp) {
  assert(sexp._tag == Tag::Pair);
  return sexp._value.pair->_cdr;
}
