#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "sexp.hpp"

std::vector<SExp> parse(std::istream& is);
std::string show(SExp sexp);
std::string show(std::vector<SExp> const& sexps);
