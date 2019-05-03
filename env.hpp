#pragma once

#include <string>

class SExp;
class Env_;
class Env {
  Env_* _env;
public:
  Env();
  Env(Env_* s) : _env{s} {}
  friend Env expand_env(Env);
  Env_ const * operator->() const {
    return _env;
  }
  Env_* operator->() {
    return _env;
  }
};

Env expand_env(Env env);
Env empty_env();
SExp lookup_symbol(Env env, std::string const& sym);
void insert(Env env, std::string sym, SExp sexp);
