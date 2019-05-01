#pragma once

#include <istream>
#include <vector>

#include "sexp.hpp"

SExp eval(SExp);
std::pair<Env, SExp> eval(Env, SExp);
SExp eval(std::vector<SExp> const&);
std::pair<Env, SExp> eval(Env, std::vector<SExp> const&);

[[noreturn]] void repl(std::istream&);
