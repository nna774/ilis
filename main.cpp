#include <iostream>

#include "sexp.hpp"
#include "parse.hpp"
#include "eval.hpp"

int main(int, char**) {
  auto sexps = parse(std::cin);
  std::cout << show(sexps);
  std::cout << "--------------------------------" << std::endl;
  auto sexp = eval(sexps);
  std::cout << show(sexp) << std::endl;
}
