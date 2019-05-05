#pragma once

#include <istream>
#include <vector>

#include "sexp.hpp"

std::pair<Env, SExp> eval(std::istream&, Env, SExp);
SExp eval(std::istream&, std::vector<SExp> const&);
std::pair<Env, SExp> eval(std::istream&, Env, std::vector<SExp> const&);

[[noreturn]] void repl(std::istream&);
