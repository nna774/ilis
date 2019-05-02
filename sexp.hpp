#pragma once

#include "env.hpp"

#include <cassert>

struct Pair;
struct Lambda;

enum class Tag {
  Pair,
  Nil,
  String,
  Integer,
  Symbol,
  Lambda,
};

SExp eq(SExp lhs, SExp rhs);
bool to_bool(SExp);

struct Lambda {
  Env env;
  SExp args;
  SExp body;
};

bool atomp(SExp sexp);
bool integerp(SExp sexp);
bool symbolp(SExp sexp);
bool lambdap(SExp sexp);
bool null(SExp sexp);

Tag type(SExp);

template<enum Tag t>
struct cast_;

template<>
struct cast_<Tag::Integer> {
  int operator()(SExp const& sexp);
};
template<>
struct cast_<Tag::Symbol> {
  char const* operator()(SExp const& sexp);
};
template<>
struct cast_<Tag::Lambda> {
  Lambda const* operator()(SExp const& sexp);
};

template<enum Tag t>
cast_<t> cast = cast_<t>{};

SExp make_Symbol(char const* str);
SExp make_Integer(int n);
SExp make_Lambda(Env, SExp args, SExp body);

extern SExp const nil;
extern SExp const TRUE;
extern SExp const FALSE;

SExp cons(SExp car, SExp cdr);

SExp car(SExp sexp);
SExp cdr(SExp sexp);

Env env(SExp lambda);
SExp args(SExp lambda);
SExp body(SExp lambda);
