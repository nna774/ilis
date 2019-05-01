#include "sexp.hpp"

#include <cstdlib>
#include <cstring>

SExp eq(SExp lhs, SExp rhs) {
  if(lhs->_tag != rhs->_tag) return FALSE;
  if(lhs->_tag == Tag::Integer) return lhs->_value.integer == rhs->_value.integer ? TRUE : FALSE;
  if(lhs->_tag == Tag::Symbol) return !std::strcmp(lhs->_value.symbol, rhs->_value.symbol) ? TRUE : FALSE;
  return lhs == rhs ? TRUE : FALSE;
}

bool atomp(SExp sexp) {
  return sexp->_tag != Tag::Pair;
}

bool integerp(SExp sexp) {
  return sexp->_tag == Tag::Integer;
}

bool symbolp(SExp sexp) {
  return sexp->_tag == Tag::Symbol;
}

bool lambdap(SExp sexp) {
  return sexp->_tag == Tag::Lambda;
}

bool null(SExp sexp) {
  return sexp->_tag == Tag::Nil;
}

Tag type(SExp sexp) {
  return sexp->_tag;
}

SExp const nil = new SExp_{};
SExp const TRUE = make_Symbol("#t");
SExp const FALSE = make_Symbol("#f");

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
  return new SExp_ {
    Tag::Symbol,
    v,
  };
}

SExp make_Integer(int n) {
  Value v;
  v.integer = n;
  return new SExp_ {
    Tag::Integer,
    v,
  };
}

SExp make_Lambda(Env env, SExp args, SExp body) {
  Value v;
  v.lambda = new Lambda{env, args, body}; // leak
  return new SExp_ {
    Tag::Lambda,
    v,
  };
}

SExp cons(SExp car, SExp cdr) {
  Value v;
  v.pair = new Pair{ car, cdr }; // will leak
  return new SExp_ {
    Tag::Pair,
    v,
  };
}

SExp car(SExp sexp) {
  assert(sexp->_tag == Tag::Pair);
  return sexp->_value.pair->_car;
}
SExp cdr(SExp sexp) {
  assert(sexp->_tag == Tag::Pair);
  return sexp->_value.pair->_cdr;
}

Env env(SExp lambda) {
  assert(lambda->_tag == Tag::Lambda);
  return lambda->_value.lambda->env;
}
SExp args(SExp lambda) {
  assert(lambda->_tag == Tag::Lambda);
  return lambda->_value.lambda->args;
}
SExp body(SExp lambda) {
  assert(lambda->_tag == Tag::Lambda);
  return lambda->_value.lambda->body;
}
