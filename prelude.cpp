#include "prelude.hpp"
#include "parse.hpp"
#include "eval.hpp"

#include <iostream>
#include <fstream>

char const* prelude_file = "prelude.lisp";

Env prelude() {
  std::ifstream is(prelude_file);
  if(!is) {
    std::cerr << "could not open " << prelude_file << std::endl;
    throw;
  }
  auto sexps = parse(is);
  auto ret = eval(empty_env(), sexps);

  return ret.first;
}
