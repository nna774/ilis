#include "env.hpp"
#include "exceptions.hpp"
#include "sexp.hpp"

#include <map>

class Env_ {
  std::map<std::string, SExp> map;
  Env_ const* parent;
public:
  Env_() : parent{nullptr} {
    map["#t"] = TRUE;
    map["#f"] = FALSE;
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


#include <list>
template<typename T>
class Allocator_ {
  std::list<std::pair<T*, bool>> list;
public:
  Allocator_() {}

  template<class... Args>
  T* New(Args... args) {
    T* t = new T{std::forward<Args>(args)...};
    return t;
  }
};
template<typename T>
Allocator_<T> Allocator = Allocator_<T>{};

Env::Env() {
  _env = Allocator<Env_>.New();
}

Env empty_env() {
  return Env{};
}

Env expand_env(Env env) {
  return Allocator<Env_>.New(env._env);
}

SExp lookup_symbol(Env env, std::string const& sym) {
  return env->lookup(sym);
}

void insert(Env env, std::string sym, SExp sexp) {
  env->insert(sym, sexp);
}
