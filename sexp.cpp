#include "sexp.hpp"

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
