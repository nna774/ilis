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

bool lambdap(SExp sexp) {
  return sexp._tag == Tag::Lambda;
}

bool null(SExp sexp) {
  return sexp._tag == Tag::Nil;
}

Tag type(SExp sexp) {
  return sexp._tag;
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

SExp make_Lambda(Env env, SExp args, SExp body) {
  Value v;
  v.lambda = new Lambda{env, args, body}; // leak
  return SExp {
    Tag::Lambda,
    v,
  };
}

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

Env env(SExp lambda) {
  assert(lambda._tag == Tag::Lambda);
  return lambda._value.lambda->env;
}
SExp args(SExp lambda) {
  assert(lambda._tag == Tag::Lambda);
  return lambda._value.lambda->args;
}
SExp body(SExp lambda) {
  assert(lambda._tag == Tag::Lambda);
  return lambda._value.lambda->body;
}
