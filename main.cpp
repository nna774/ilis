#include <iostream>

#include "sexp.hpp"
#include "parse.hpp"
#include "eval.hpp"

int main(int argc, char**) {
  if(argc > 1) {
    repl(std::cin);
  }

  auto sexps = parse(std::cin);
  std::cout << show(sexps);
  std::cout << "--------------------------------" << std::endl;
  auto sexp = eval(std::cin, sexps);
  std::cout << show(sexp) << std::endl;

  return 0;
}
