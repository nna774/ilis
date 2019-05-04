#include "env.hpp"
#include "exceptions.hpp"
#include "sexp.hpp"

#include <map>

class Env_ {
  std::map<std::string, SExp> map;
  Env const parent;
public:
  Env_() : parent{} {
    map["#t"] = TRUE;
    map["#f"] = FALSE;
  }
  Env_(Env const p) : map{}, parent{p} {}
  SExp lookup(std::string const& sym) const {
    auto it = map.find(sym);
    if(it != end(map)) {
      return it->second;
    }
    if(!!parent) {
      return parent->lookup(sym);
    }
    raise_with_str(UnboundVariableException, sym);
  }
  void insert(std::string const& sym, SExp sexp) {
    map.insert(std::make_pair(sym, sexp));
  }
};

Env::Env() {
  e = nullptr;
}
Env::Env(Env const& env) {
  e = env.e;
}
Env::Env(Env::_inner_type e_) {
  e = e_;
}
Env& Env::operator=(Env const& e_) {
  e = e_.e;
  return *this;
}

Env Env::expand() {
  return Allocator<Env>().New(*this);
}

Env empty_env() {
  return Allocator<Env>().New();
}

Env expand_env(Env env) {
  return env.expand();
}

SExp lookup_symbol(Env env, std::string const& sym) {
  return env->lookup(sym);
}

void insert(Env env, std::string sym, SExp sexp) {
  env->insert(sym, sexp);
}
