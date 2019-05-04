#pragma once

#include <string>
#include <cassert>

#include "allocator.hpp"

class SExp;
class Env_;
class Env {
public:
  using element_type = Env_;
  using _inner_type = std::pair<bool, Env_*>*;
  Env();
  Env(Env const&);
  Env& operator=(Env const&);
  Env(_inner_type);
  Env_ const * operator->() const {
    assert(e != nullptr);
    return e->second;
  }
  Env_* operator->() {
    assert(e != nullptr);
    return e->second;
  }
  operator bool() const {
    return e != nullptr;
  }
  Env expand();
private:
  _inner_type e;
};

Env expand_env(Env env);
Env empty_env();
SExp lookup_symbol(Env env, std::string const& sym);
void insert(Env env, std::string sym, SExp sexp);
