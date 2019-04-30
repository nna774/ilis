#include "eval.hpp"
#include "utils.hpp"
#include "exceptions.hpp"
#include "parse.hpp"

#include <utility>
#include <tuple>
#include <map>

class Env_ {
  std::map<std::string, SExp> map;
  Env_ const* parent;
public:
  Env_() : parent{nullptr} {
  }
  Env_(Env_ const* p) : map{}, parent{p} {}
  SExp lookup(std::string const& sym) const {
    auto it = map.find(sym);
    if(it != end(map)) {
      return it->second;
    }
    if(parent != nullptr) {
      return parent->lookup(sym);
    }
    raise_with_str(UnboundVariableException, sym);
  }
  void insert(std::string const& sym, SExp sexp) {
    map.insert(std::make_pair(sym, sexp));
  }
};

using Env = Env_*;
Env const default_env = new Env_{};

Env expand_env(Env env) {
  return new Env_{env};
}

SExp lookup_symbol(Env env, std::string const& sym) {
  return env->lookup(sym);
}

void insert(Env env, std::string sym, SExp sexp) {
  env->insert(sym, sexp);
}

std::pair<Env, SExp> eval(Env, SExp);

std::pair<Env, SExp> eval_list(Env env, SExp sexp) {
  if(null(sexp)) return std::make_pair(env, nil);
  auto e = eval(env, car(sexp));
  auto tail = eval_list(e.first, cdr(sexp));
  return std::make_pair(tail.first, cons(e.second, tail.second));
}

SExp eval_cons(SExp sexp) {
  auto car_ = car(sexp);
  auto cadr = car(cdr(sexp));
  auto cddr = cdr(cdr(sexp));
  if(!null(cddr)) {
    raise_with_str(ConsInvalidApplicationException, show(sexp));
  }
  return cons(car_, cadr);
}

SExp eval_primitive(std::string prim, SExp sexp) {
  if(prim == "cons") {
    return eval_cons(sexp);
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
  auto cond_ = eval(env, cond);
  env = cond_.first;
  if(null(cond_.second)) {
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

std::pair<Env, SExp> eval_specialforms(std::string form, Env env, SExp sexp) {
  if(form == "if") {
    return eval_if(env, sexp); 
  }
  if(form == "define") {
    return eval_define(env, sexp);
  }
  if(form == "quote") {
    return std::make_pair(env, sexp);
  }
  raise(NeverComeException);
}

std::pair<Env, SExp> eval(Env env, SExp sexp) {
  if(null(sexp) || integerp(sexp)) return std::make_pair(env, sexp);
  if(symbolp(sexp)) return std::make_pair(env, lookup_symbol(env, cast<Tag::Symbol>(sexp)));
  auto car_ = car(sexp);
  auto cdr_ = cdr(sexp);
  if(!atomp(car_)) {
    car_ = eval(car_);
  }
  if(!(symbolp(car_) || lambdap(car_))) {
    raise_with_str(InvalidApplicationException, show(car_));
  }
  auto primitives = std::experimental::make_array<std::string>("cons", "car", "cdr", "atom", "eq");
  if(symbolp(car_) && in(std::string{cast<Tag::Symbol>(car_)}, primitives)) {
    auto l = eval_list(env, cdr_);
    // eval_list で評価は終了しているので、その後envは変化しない。
    return std::make_pair(l.first, eval_primitive(cast<Tag::Symbol>(car_), l.second));
  }
  auto specialforms = std::experimental::make_array<std::string>("if", "define", "defmacro", "quote", "lambda");
  if(symbolp(car_) && in(std::string{cast<Tag::Symbol>(car_)}, specialforms)) {
    return eval_specialforms(cast<Tag::Symbol>(car_), env, cdr_);
  }

  raise(NeverComeException);
}

SExp eval(SExp sexp) {
  auto r = eval(default_env, sexp);
  return r.second;
}

std::pair<Env, SExp> eval(Env env, std::vector<SExp> sexps) {
  auto ret{nil};
  for(auto sexp: sexps) {
    std::tie(env, ret) = eval(env, sexp);
  }
  return std::make_pair(env, ret);
}

SExp eval(std::vector<SExp> sexps) {
  auto r = eval(default_env, sexps);
  return r.second;
}
