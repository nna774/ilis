#include "eval.hpp"
#include "utils.hpp"
#include "exceptions.hpp"
#include "parse.hpp"
#include "prelude.hpp"

#include <iostream>
#include <tuple>
#include <cstring>

Env const default_env = prelude();

std::pair<Env, SExp> eval_list(Env env, SExp sexp) {
  if(null(sexp)) return std::make_pair(env, nil);
  auto e = eval(env, car(sexp));
  auto tail = eval_list(e.first, cdr(sexp));
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

[[noreturn]] void fail(SExp sexp) {
  std::cerr << "*** fail *** " << show(sexp) << std::endl;
  raise(FailException);
}

SExp eval_primitive(std::string prim, SExp sexp) {
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
  if(prim == "fail") {
    fail(sexp);
  }

  raise(NeverComeException);
}

std::pair<Env, SExp> eval_if(Env env, SExp sexp) {
  auto cond = car(sexp);
  auto true_branch = car(cdr(sexp));
  auto false_branch = car(cdr(cdr(sexp)));
  auto cdddr = cdr(cdr(cdr(sexp)));
  if(!null(cdddr)) {
    raise_with_str(IfInvalidApplicationException, show(sexp));
  }
  std::tie(env, cond) = eval(env, cond);
  if(to_bool(eq(cond, FALSE))) {
    return eval(env, false_branch);
  } else {
    return eval(env, true_branch);
  }
}

std::pair<Env, SExp> eval_define(Env env, SExp sexp) {
  auto sym = car(sexp);
  auto val = car(cdr(sexp));
  auto cddr = cdr(cdr(sexp));
  assert(symbolp(sym));
  if(!null(cddr)) {
    raise_with_str(DefineInvalidApplicationException, show(sexp));
  }
  auto v = eval(env, val);
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

std::pair<Env, SExp> eval_specialforms(std::string form, Env env, SExp sexp) {
  if(form == "if") {
    return eval_if(env, sexp); 
  }
  if(form == "define") {
    return eval_define(env, sexp);
  }
  if(form == "lambda") {
    return eval_lambda(env, sexp);
  }
  if(form == "quote") {
    return std::make_pair(env, sexp);
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
  if(null(actual)) {
    invalid();
  }
  if(!symbolp(dummy)) {
    invalid();
  }
  insert(env, cast<Tag::Symbol>(dummy), actual);
  return push_symbols(env, cdr(dummies), cdr(actuals));
}

std::pair<Env, SExp> application(Env env_, SExp lambda, SExp args_) {
  auto [outer_env, apply_args] = eval_list(env_, args_);
  Env lambda_env = expand_env(env(lambda));
  auto lambda_args = args(lambda);
  if(symbolp(lambda_args)) {
    insert(lambda_env, cast<Tag::Symbol>(lambda_args), apply_args);
  } else {
    lambda_env = push_symbols(lambda_env, lambda_args, apply_args);
  }
  auto body_ = body(lambda);
  auto ret = nil;
  while(!null(body_)) {
    std::tie(lambda_env, ret) = eval(lambda_env, car(body_));
    body_ = cdr(body_);
  }
  return std::make_pair(outer_env, ret);
}

std::pair<Env, SExp> eval(Env env, SExp sexp) {
  if(null(sexp) || integerp(sexp)) return std::make_pair(env, sexp);
  if(symbolp(sexp)) return std::make_pair(env, lookup_symbol(env, cast<Tag::Symbol>(sexp)));
  auto car_ = car(sexp);
  auto cdr_ = cdr(sexp);
  if(!atomp(car_)) {
    std::tie(env, car_) = eval(env, car_);
  }
  if(!(symbolp(car_) || lambdap(car_))) {
    raise_with_str(InvalidApplicationException, show(car_));
  }
  if(symbolp(car_)) {
    auto const primitives = std::experimental::make_array<std::string>("cons", "car", "cdr", "atom", "eq", "fail", "inc", "dec", "sign");
    if(in<std::string>(cast<Tag::Symbol>(car_), primitives)) {
      auto l = eval_list(env, cdr_);
      // eval_list で評価は終了しているので、その後envは変化しない。
      return std::make_pair(l.first, eval_primitive(cast<Tag::Symbol>(car_), l.second));
    }
    auto const specialforms = std::experimental::make_array<std::string>("if", "define", "defmacro", "quote", "lambda");
    if(in<std::string>(cast<Tag::Symbol>(car_), specialforms)) {
      return eval_specialforms(cast<Tag::Symbol>(car_), env, cdr_);
    }
    std::tie(env, car_) = eval(env, car_);
  }
  if(macrop(car_)) {
    return eval(env, expand_macro(car_, cdr_));
  }
  if(lambdap(car_)) {
    return application(env, car_, cdr_);
  }

  raise(NeverComeException);
}

SExp eval(SExp sexp) {
  auto r = eval(default_env, sexp);
  return r.second;
}

std::pair<Env, SExp> eval(Env env, std::vector<SExp> const& sexps) {
  auto ret{nil};
  for(auto sexp: sexps) {
    std::tie(env, ret) = eval(env, sexp);
  }
  return std::make_pair(env, ret);
}

SExp eval(std::vector<SExp> const& sexps) {
  auto r = eval(default_env, sexps);
  return r.second;
}

[[noreturn]] void repl(std::istream& is) {
  auto env = default_env;
  while(true) {
    auto sexp = parse_SExpr(is);
    std::tie(env, sexp) = eval(env, sexp);
    std::cout << "#=> " << show(sexp) << std::endl;
    skip_spaces(is);
  }
}
