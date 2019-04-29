#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cassert>
#include <algorithm>
#include <experimental/array>

#include <cstdlib>
#include <cstring>

#include "exceptions.hpp"

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

template<size_t N>
bool in(char c, std::array<char, N> chars) {
  return std::any_of(begin(chars), end(chars), [c](char _) { return c == _; });
}

bool number_char(char c) {
  return ('0' <= c && c <= '9');
}

bool identifier_char(char c) {
  auto chars = std::experimental::make_array('-', '+', '?');
  return ('a' <= c && c <= 'z')
      || ('A' <= c && c <= 'Z')
      || number_char(c)
      || in(c, chars)
      ;
}

bool space_char(char c) {
  auto chars = std::experimental::make_array(' ', '\n', '\t');
  return in(c, chars);
}

void skip_spaces(std::istream& is) {
  int c;
  while(c = is.peek(), c != EOF && space_char(c)) {
    char c;
    is.get(c);
  }
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

std::string read_identifier(std::istream& is) {
  int c;
  std::stringstream ss;
  while(c = is.peek(), c != EOF && identifier_char(c)) {
    char c;
    is.get(c);
    ss << c; 
  }
  return ss.str();
}

int read_integer(std::istream& is) {
  int c, res{};
  while(c = is.peek(), c != EOF && number_char(c)) {
    char c;
    is.get(c);
    res = res * 10 + (c - '0');
  }
  return res;
}

SExp parse_SExpr(std::istream& is);

SExp parse_Symbol(std::istream& is) {
  std::string str{read_identifier(is)};
  return make_Symbol(str.c_str());
}

SExp parse_Integer(std::istream& is) {
  int n = read_integer(is);
  return make_Integer(n);
}

SExp parse_List(std::istream& is) {
  char c;
  is.get(c);
  if(c != '(') {
    throw UnexpectedCharException{c};
  }

  int c_;
  std::vector<SExp> sexps;
  skip_spaces(is);
  while(c_ = is.peek(), c_ != EOF && c_ != ')') {
    auto sexp = parse_SExpr(is);
    sexps.push_back(sexp);
    skip_spaces(is);
  }
  is.get(c);
  SExp sexp = nil;
  for(int i{static_cast<int>(sexps.size()) - 1}; i >= 0; --i) {
    sexp = cons(sexps[i], sexp);
  }
  return sexp;
}

SExp parse_SExpr(std::istream& is) {
  int c_ = is.peek();
  if(c_ == EOF) {
    throw UnexpectedEoFException{};
  }
  char c;
  is.get(c);

  switch(c) {
  case '\'': {
    return cons(
      make_Symbol("quote"),
      parse_SExpr(is)
    );
  }
  case '(': {
    is.unget();
    return parse_List(is);
  }
  default: { // symbol or integer
    is.unget();
    if(number_char(c)) {
      return parse_Integer(is);
    } else if(identifier_char(c)) {
      return parse_Symbol(is);
    } else {
      throw NeverComeException{};
    }
  } 
  }
}

std::vector<SExp> parse(std::istream& is) {
  std::vector<SExp> v;
  int c;
  skip_spaces(is);
  while(c = is.peek(), c != EOF) {
    v.push_back(parse_SExpr(is));
    skip_spaces(is);
  }
  return v;
}

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

std::string show(SExp sexp);

std::string show_list_impl(SExp sexp) {
  assert(!atomp(sexp));
  auto car_ = car(sexp);
  auto cdr_ = cdr(sexp);
  if(null(cdr_)) {
    return show(car_);
  }
  if(atomp(cdr_)) {
    return show(car_) + " . " + show(cdr_);
  }
  return show(car_) + ' ' + show_list_impl(cdr_);
}

std::string show_list(SExp sexp) {
  return '(' + show_list_impl(sexp) + ')';
}

std::string show(SExp sexp) {
  std::stringstream ss;
  if(!atomp(sexp)) { // pair
    return show_list(sexp);
  } else if(integerp(sexp)) {
    ss << cast<Tag::Integer>(sexp);
  } else if(null(sexp)) {
    ss << "'()";
  } else if(symbolp(sexp)) {
    ss << std::string{cast<Tag::Symbol>(sexp)};
  } else {
  }
  return ss.str();
}

void show(std::vector<SExp> const& sexps) {
  for(auto sexp: sexps) {
    std::cout << show(sexp) << std::endl;
  }
}

int main(int, char**) {
  auto sexps = parse(std::cin);
  show(sexps);
}
