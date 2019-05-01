#pragma once

#include <string>

struct SExp;

class Env_;
using Env = Env_ *;

Env expand_env(Env env);
Env empty_env();
SExp lookup_symbol(Env env, std::string const& sym);
void insert(Env env, std::string sym, SExp sexp);
