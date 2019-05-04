#include "sexp.hpp"
#include "allocator.hpp"

#include <cstring>

union Value {
  Pair* pair;
  char const* symbol;
  int integer;
  Lambda* lambda;
};

struct SExp_ {
  Tag _tag;
  Value _value;
  SExp_() : _tag{Tag::Nil}, _value{} {}
  SExp_(Tag t, Value v) : _tag{t}, _value{v} {}
};

struct Pair {
  SExp _car;
  SExp _cdr;
  Pair(SExp car, SExp cdr) : _car{car}, _cdr{cdr} {}
};

struct Lambda {
  Env env;
  SExp args;
  SExp body;
};

SExp::SExp() {
  auto ss = Allocator<SExp>.New();
  s = ss.s;
}
SExp::SExp(SExp const& ss) {
  s = ss.s;
}
SExp& SExp::operator=(SExp const& ss) {
  s = ss.s;
  return *this;
}
SExp::SExp(SExp::_inner_type s_) {
  s = s_;
}

void SExp::mark() {
  s->first = true;
}

int cast_<Tag::Integer>::operator()(SExp const& sexp) {
  assert(sexp->_tag == Tag::Integer);
  return sexp->_value.integer;
}
char const* cast_<Tag::Symbol>::operator()(SExp const& sexp) {
  assert(sexp->_tag == Tag::Symbol);
  return sexp->_value.symbol;
}
Lambda const* cast_<Tag::Lambda>::operator()(SExp const& sexp) {
  assert(sexp->_tag == Tag::Lambda);
  return sexp->_value.lambda;
}

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

bool macrop(SExp sexp) {
  return sexp->_tag == Tag::Macro;
}

bool null(SExp sexp) {
  return sexp->_tag == Tag::Nil;
}

Tag type(SExp sexp) {
  return sexp->_tag;
}

bool to_bool(SExp sexp) {
  if(sexp->_tag != Tag::Symbol) return true;
  return std::strcmp(sexp->_value.symbol, "#f");
}

SExp const nil{};
SExp const TRUE = make_Symbol("#t");
SExp const FALSE = make_Symbol("#f");

char* copy_str(char const* str) {
  // 現代のコードではない。あとでGCを書く。
  size_t len = std::strlen(str);
  char* new_str = new char[len + 1];
  std::strcpy(new_str, str);
  return new_str;
}

SExp make_Symbol(char const* str) {
  Value v;
  v.symbol = copy_str(str); // leak
  return Allocator<SExp>.New(
    Tag::Symbol,
    v
  );
}

SExp make_Integer(int n) {
  Value v;
  v.integer = n;
  return Allocator<SExp>.New(
    Tag::Integer,
    v
  );
}

SExp make_Lambda(Env env, SExp args, SExp body) {
  Value v;
  v.lambda = new Lambda{env, args, body}; // leak
  return Allocator<SExp>.New(
    Tag::Lambda,
    v
  );
}

SExp make_Macro(Env env, SExp args, SExp body) {
  Value v;
  v.lambda = new Lambda{env, args, body}; // leak
  return Allocator<SExp>.New(
    Tag::Macro,
    v
  );
}

SExp cons(SExp car, SExp cdr) {
  Value v;
  v.pair = new Pair{ car, cdr }; // will leak
  return Allocator<SExp>.New(
    Tag::Pair,
    v
  );
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

SExp macro_args(SExp lambda) {
  assert(lambda->_tag == Tag::Macro);
  return lambda->_value.lambda->args;
}
SExp macro_body(SExp lambda) {
  assert(lambda->_tag == Tag::Macro);
  return lambda->_value.lambda->body;
}
