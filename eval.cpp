#include "eval.hpp"
#include "utils.hpp"
#include "exceptions.hpp"
#include "parse.hpp"
#include "prelude.hpp"

#include <iostream>
#include <tuple>
#include <cstring>

Env& default_env(std::istream& is) {
  static Env env = prelude(is);
  return env;
}

std::pair<Env, SExp> eval_list(std::istream& is, Env env, SExp sexp) {
  if(null(sexp)) return std::make_pair(env, nil);
  auto e = eval(is, env, car(sexp));
  auto tail = eval_list(is, e.first, cdr(sexp));
  return std::make_pair(tail.first, cons(e.second, tail.second));
}

SExp eval_cons(SExp sexp) {
  auto invalid = [&](){ raise_with_str(ConsInvalidApplicationException, show(sexp)); };
  if(null(sexp)) invalid();
  auto car_ = car(sexp);
  auto cdr_ = cdr(sexp);
  if(null(cdr_)) invalid();
  auto cadr = car(cdr_);
  auto cddr = cdr(cdr_);
  if(!null(cddr)) invalid();
  return cons(car_, cadr);
}

SExp eval_car(SExp sexp) {
  return car(car(sexp));
}
SExp eval_cdr(SExp sexp) {
  return cdr(car(sexp));
}
SExp eval_atom(SExp sexp) {
  return atomp(car(sexp)) ? TRUE : FALSE;
}
SExp eval_eq(SExp sexp) {
  auto car_ = car(sexp);
  auto cdr_ = cdr(sexp);
  auto cadr = car(cdr_);
  return eq(car_, cadr);
}

SExp eval_add(SExp sexp, int diff) {
  sexp = car(sexp);
  assert(integerp(sexp));
  return make_Integer(cast<Tag::Integer>(sexp) + diff);
}

SExp eval_sign(SExp sexp) {
  sexp = car(sexp);
  assert(integerp(sexp));
  int d = cast<Tag::Integer>(sexp);
  int sign = d < 0 ? -1 : (d != 0);
  return make_Integer(sign);
}

SExp eval_read(std::istream& is, SExp) {
  return parse_SExpr(is);
}

[[noreturn]] void fail(SExp sexp) {
  std::cerr << "*** fail *** " << show(sexp) << std::endl;
  raise(FailException);
}

SExp eval_primitive(std::istream& is, std::string prim, SExp sexp) {
  if(prim == "cons") {
    return eval_cons(sexp);
  }
  if(prim == "car") {
    return eval_car(sexp);
  }
  if(prim == "cdr") {
    return eval_cdr(sexp);
  }
  if(prim == "atom") {
    return eval_atom(sexp);
  }
  if(prim == "eq") {
    return eval_eq(sexp);
  }
  if(prim == "inc") {
    return eval_add(sexp, 1);
  }
  if(prim == "dec") {
    return eval_add(sexp, -1);
  }
  if(prim == "sign") {
    return eval_sign(sexp);
  }
  if(prim == "read") {
    return eval_read(is, sexp);
  }
  if(prim == "fail") {
    fail(sexp);
  }

  raise(NeverComeException);
}

std::pair<Env, SExp> eval_if(std::istream& is, Env env, SExp sexp) {
  auto cond = car(sexp);
  auto true_branch = car(cdr(sexp));
  auto false_branch = car(cdr(cdr(sexp)));
  auto cdddr = cdr(cdr(cdr(sexp)));
  if(!null(cdddr)) {
    raise_with_str(IfInvalidApplicationException, show(sexp));
  }
  std::tie(env, cond) = eval(is, env, cond);
  if(to_bool(eq(cond, FALSE))) {
    return eval(is, env, false_branch);
  } else {
    return eval(is, env, true_branch);
  }
}

std::pair<Env, SExp> eval_define(std::istream& is, Env env, SExp sexp) {
  auto sym = car(sexp);
  auto val = car(cdr(sexp));
  auto cddr = cdr(cdr(sexp));
  assert(symbolp(sym));
  if(!null(cddr)) {
    raise_with_str(DefineInvalidApplicationException, show(sexp));
  }
  auto v = eval(is, env, val);
  insert(env, cast<Tag::Symbol>(sym), v.second);
  return std::make_pair(env, sym);
}

std::pair<Env, SExp> eval_lambda(Env env, SExp sexp) {
  auto args = car(sexp);
  auto body = cdr(sexp);
  return std::make_pair(env, make_Lambda(env, args, body));
}

std::pair<Env, SExp> eval_macro(Env env, SExp sexp) {
  auto sym = car(sexp);
  assert(symbolp(sym));
  auto args = car(cdr(sexp));
  auto body = car(cdr(cdr(sexp)));
  insert(env, cast<Tag::Symbol>(sym), make_Macro(env, args, body));
  return std::make_pair(env, sym);
}

std::pair<Env, SExp> eval_specialforms(std::istream& is, std::string form, Env env, SExp sexp) {
  if(form == "if") {
    return eval_if(is, env, sexp);
  }
  if(form == "define") {
    return eval_define(is, env, sexp);
  }
  if(form == "lambda") {
    return eval_lambda(env, sexp);
  }
  if(form == "quote") {
    return std::make_pair(env, car(sexp));
  }
  if(form == "defmacro") {
    return eval_macro(env, sexp);
  }
  raise(NeverComeException);
}

SExp reverse_impl(SExp list, SExp result) {
  if(null(list)) return result;
  return reverse_impl(cdr(list), cons(car(list), result));
}
SExp reverse(SExp list) {
  return reverse_impl(list, nil);
}

SExp copy_list_impl(SExp list, SExp result) {
  if(null(list)) return result;
  return copy_list_impl(cdr(list), cons(car(list), result));
}
SExp copy_list(SExp list) {
  return reverse(copy_list_impl(list, nil));
}

SExp replace(char const* sym, SExp actual, SExp expanded);
SExp replace_impl(char const* sym, SExp actual, SExp expanded, SExp result) {
  if(null(expanded)) return result;
  auto it = car(expanded);
  if(symbolp(it)) {
    if(!std::strcmp(sym, cast<Tag::Symbol>(it))) {
      return replace_impl(sym, actual, cdr(expanded), cons(actual, result));
    }
    return replace_impl(sym, actual, cdr(expanded), cons(it, result));
  }
  if(!atomp(it)) {
    return replace_impl(sym, actual, cdr(expanded), cons(replace(sym, actual, it), result));
  }
  if(null(it)) {
    return replace_impl(sym, actual, cdr(expanded), cons(nil, result));
  }
  raise(NeverComeException);
}
SExp replace(char const* sym, SExp actual, SExp expanded) {
  return reverse(replace_impl(sym, actual, expanded, nil));
}

SExp expand_macro(SExp macro, SExp args) {
  auto macro_args_ = macro_args(macro);
  auto macro_body_ = macro_body(macro);
  auto expanded = copy_list(macro_body_);
  while(!null(macro_args_)) {
    auto dummy = cast<Tag::Symbol>(car(macro_args_));
    auto actual = car(args);
    expanded = replace(dummy, actual, expanded);
    macro_args_ = cdr(macro_args_);
    args = cdr(args);
  }
  return expanded;
}

Env push_symbols(Env env, SExp dummies, SExp actuals) {
  auto invalid = [&](){ raise_with_str(LambdaInvalidApplicationException, "dummies: " + show(dummies) + ", actuals: " + show(actuals)); };
  if(null(dummies)) {
    if(!null(actuals)) {
      invalid();
    }
    return env;
  }
  auto dummy = car(dummies);
  auto actual = car(actuals);
  if(!symbolp(dummy)) {
    invalid();
  }
  insert(env, cast<Tag::Symbol>(dummy), actual);
  return push_symbols(env, cdr(dummies), cdr(actuals));
}

std::pair<Env, SExp> application(std::istream& is, Env env_, SExp lambda, SExp args_) {
  auto [outer_env, apply_args] = eval_list(is, env_, args_);
  Env lambda_env = expand_env(env(lambda));
  auto lambda_args = args(lambda);
  if(symbolp(lambda_args)) {
    insert(lambda_env, cast<Tag::Symbol>(lambda_args), apply_args);
  } else {
    lambda_env = push_symbols(lambda_env, lambda_args, apply_args);
  }
  auto body_ = body(lambda);
  SExp ret = nil;
  while(!null(body_)) {
    std::tie(lambda_env, ret) = eval(is, lambda_env, car(body_));
    body_ = cdr(body_);
  }
  return std::make_pair(outer_env, ret);
}

auto const primitives = std::experimental::make_array<std::string>("cons", "car", "cdr", "atom", "eq", "fail", "inc", "dec", "sign", "read");
auto const specialforms = std::experimental::make_array<std::string>("if", "define", "defmacro", "quote", "lambda");

std::pair<Env, SExp> eval(std::istream& is, Env env, SExp sexp) {
  if(null(sexp) || integerp(sexp)) return std::make_pair(env, sexp);
  if(symbolp(sexp)) return std::make_pair(env, lookup_symbol(env, cast<Tag::Symbol>(sexp)));
  auto car_ = car(sexp);
  auto cdr_ = cdr(sexp);
  if(!atomp(car_)) {
    std::tie(env, car_) = eval(is, env, car_);
  }
  if(!(symbolp(car_) || lambdap(car_))) {
    raise_with_str(InvalidApplicationException, show(car_));
  }
  if(symbolp(car_)) {
    if(in<std::string>(cast<Tag::Symbol>(car_), primitives)) {
      auto l = eval_list(is, env, cdr_);
      // eval_list で評価は終了しているので、その後envは変化しない。
      return std::make_pair(l.first, eval_primitive(is, cast<Tag::Symbol>(car_), l.second));
    }
    if(in<std::string>(cast<Tag::Symbol>(car_), specialforms)) {
      return eval_specialforms(is, cast<Tag::Symbol>(car_), env, cdr_);
    }
    std::tie(env, car_) = eval(is, env, car_);
  }
  if(macrop(car_)) {
    return eval(is, env, expand_macro(car_, cdr_));
  }
  if(lambdap(car_)) {
    return application(is, env, car_, cdr_);
  }

  raise(NeverComeException);
}

SExp eval(std::istream& is, SExp sexp) {
  auto r = eval(is, default_env(is), sexp);
  return r.second;
}

std::pair<Env, SExp> eval(std::istream& is, Env env, std::vector<SExp> const& sexps) {
  SExp ret{nil};
  for(auto sexp: sexps) {
    std::tie(env, ret) = eval(is, env, sexp);
  }
  return std::make_pair(env, ret);
}

SExp eval(std::istream& is, std::vector<SExp> const& sexps) {
  auto r = eval(is, default_env(is), sexps);
  return r.second;
}

[[noreturn]] void repl(std::istream& is) {
  auto env = default_env(is);
  while(true) {
    auto sexp = parse_SExpr(is);
    std::tie(env, sexp) = eval(is, env, sexp);
    std::cout << "#=> " << show(sexp) << std::endl;
    skip_spaces(is);
  }
}
