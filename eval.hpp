#pragma once

#include <vector>

#include "sexp.hpp"

SExp eval(SExp);
SExp eval(std::vector<SExp>);
