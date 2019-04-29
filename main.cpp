#include <iostream>

#include "sexp.hpp"
#include "parse.hpp"

int main(int, char**) {
  auto sexps = parse(std::cin);
  std::cout << show(sexps);
}
