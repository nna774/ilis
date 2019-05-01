#include "eval.hpp"
#include "utils.hpp"
#include "exceptions.hpp"
#include "parse.hpp"
#include "prelude.hpp"

#include <iostream>
#include <tuple>

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
  Env new_env = expand_env(env);
  insert(new_env, cast<Tag::Symbol>(sym), v.second);
  return std::make_pair(new_env, sym);
}

std::pair<Env, SExp> eval_lambda(Env env, SExp sexp) {
  auto args = car(sexp);
  auto body = cdr(sexp);
  return std::make_pair(env, make_Lambda(env, args, body));
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
  raise(NeverComeException);
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
  auto a = eval_list(env_, args_);
  auto outer_env = a.first;
  auto apply_args = a.second;
  Env lambda_env = expand_env(env(lambda));
  lambda_env = push_symbols(lambda_env, args(lambda), apply_args);
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
    auto primitives = std::experimental::make_array<std::string>("cons", "car", "cdr", "atom", "eq", "fail");
    if(in<std::string>(cast<Tag::Symbol>(car_), primitives)) {
      auto l = eval_list(env, cdr_);
      // eval_list で評価は終了しているので、その後envは変化しない。
      return std::make_pair(l.first, eval_primitive(cast<Tag::Symbol>(car_), l.second));
    }
    auto specialforms = std::experimental::make_array<std::string>("if", "define", "defmacro", "quote", "lambda");
    if(in<std::string>(cast<Tag::Symbol>(car_), specialforms)) {
      return eval_specialforms(cast<Tag::Symbol>(car_), env, cdr_);
    }
    std::tie(env, car_) = eval(env, car_);
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
