#pragma once

#include <string>

struct NeverComeException {};
struct UnexpectedEoFException {};
struct UnexpectedCharException {
  char c;
  UnexpectedCharException(char c_) : c{c_} {}
};
struct InvalidApplicationException {
  std::string str;
  InvalidApplicationException(std::string str_) : str{str_} {}
};
struct ConsInvalidApplicationException : public InvalidApplicationException {};
struct IfInvalidApplicationException : public InvalidApplicationException {};
struct DefineInvalidApplicationException : public InvalidApplicationException {};

struct UnboundVariableException {
  std::string str;
  UnboundVariableException(std::string str_) : str{str_} {}
};
