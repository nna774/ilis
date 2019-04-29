#pragma once

#include <cassert>

struct SExp;
struct Pair;

enum class Tag {
  Pair,
  Nil,
  String,
  Integer,
  Symbol,
  Lambda,
};

union Value {
  Pair* pair;
  char const* symbol;
  int integer;
};

struct SExp {
  Tag _tag;
  Value _value;
  SExp(Tag t, Value v) : _tag(t), _value(v) {}
};

struct Pair {
  SExp _car;
  SExp _cdr;
  Pair(SExp car, SExp cdr) : _car{car}, _cdr{cdr} {}
};

bool atomp(SExp sexp);

bool integerp(SExp sexp);

bool symbolp(SExp sexp);

bool null(SExp sexp);

template<enum Tag t>
struct cast_{};

template<>
struct cast_<Tag::Integer> {
  int operator()(SExp const& sexp) {
    assert(sexp._tag == Tag::Integer);
    return sexp._value.integer;
  }
};
template<>
struct cast_<Tag::Symbol> {
  char const* operator()(SExp const& sexp) {
    assert(sexp._tag == Tag::Symbol);
    return sexp._value.symbol;
  }
};

template<enum Tag t>
cast_<t> cast = cast_<t>();
