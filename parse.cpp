#include "parse.hpp"

#include <sstream>

#include "exceptions.hpp"
#include "utils.hpp"

bool number_char(char c) {
  return ('0' <= c && c <= '9');
}

bool identifier_char(char c) {
  auto chars = std::experimental::make_array('-', '+', '?', '#');
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
    raise_with_char(UnexpectedCharException, c);
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
  skip_spaces(is);
  int c_ = is.peek();
  if(c_ == EOF) {
    raise(UnexpectedEoFException);
  }
  char c;
  is.get(c);

  switch(c) {
  case '\'': {
    return cons(
      make_Symbol("quote"),
      cons(
        parse_SExpr(is),
        nil)
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
      raise(NeverComeException);
    }
  }
  }
}

std::vector<SExp> parse(std::istream& is) {
  std::vector<SExp> v;
  int c;
  while(c = is.peek(), c != EOF) {
    v.push_back(parse_SExpr(is));
    skip_spaces(is);
  }
  return v;
}

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
    ss << "()";
  } else if(symbolp(sexp)) {
    ss << std::string{cast<Tag::Symbol>(sexp)};
  } else if(lambdap(sexp)) {
    if(atomp(args(sexp))) {
      ss << "(lambda " << show(args(sexp)) << ' ' << show_list(body(sexp)) << ')';
    } else {
      ss << "(lambda " << show_list(args(sexp)) << ' ' << show_list(body(sexp)) << ')';
    }
  } else if(macrop(sexp)) {
    ss << "(defmacro )";
  }
  return ss.str();
}

std::string show(std::vector<SExp> const& sexps) {
  std::stringstream ss;
  for(auto sexp: sexps) {
    ss << show(sexp) << std::endl;
  }
  return ss.str();
}

std::string show(Tag tag) {
  switch(tag) {
  case Tag::Pair:
    return "Pair";
  case Tag::Nil:
    return "Nil";
  case Tag::String:
    return "String";
  case Tag::Integer:
    return "Integer";
  case Tag::Symbol:
    return "Symbol";
  case Tag::Lambda:
    return "Lambda";
  case Tag::Macro:
    return "Macro";
  default:
    raise(NeverComeException);
  }
}
