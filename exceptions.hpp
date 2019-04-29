#pragma once

struct NeverComeException {};
struct UnexpectedEoFException {};
struct UnexpectedCharException {
  char c;
  UnexpectedCharException(char _c) : c{_c} {}
};
