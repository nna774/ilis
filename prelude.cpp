#include "prelude.hpp"
#include "parse.hpp"
#include "eval.hpp"

#include <iostream>
#include <fstream>

char const* prelude_file = "prelude.lisp";

Env prelude(std::istream& is) {
  std::ifstream ifs(prelude_file);
  if(!ifs) {
    std::cerr << "could not open " << prelude_file << std::endl;
    throw;
  }
  auto sexps = parse(ifs);
  auto ret = eval(is, empty_env(), sexps);

  return ret.first;
}
