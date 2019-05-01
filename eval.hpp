#pragma once

#include <istream>
#include <vector>

#include "sexp.hpp"

SExp eval(SExp);
SExp eval(std::vector<SExp>);

[[noreturn]] void repl(std::istream&);
