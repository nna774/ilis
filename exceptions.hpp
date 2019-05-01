#pragma once

#include <string>

struct Exception {
  std::string file;
  int line;
  Exception(std::string_view f, int l) : file{f}, line{l} {}
};
struct NeverComeException : public Exception {
  using Exception::Exception;
};
struct UnexpectedEoFException : public Exception {
  using Exception::Exception;
};
struct UnexpectedCharException : public Exception {
  char c;
  UnexpectedCharException(std::string_view file, int l, char c_) : Exception{file, l}, c{c_} {}
};
struct InvalidApplicationException : public Exception {
  std::string str;
  InvalidApplicationException(std::string_view file, int l, std::string_view str) : Exception{file, l}, str{str} {}
};
struct ConsInvalidApplicationException : public InvalidApplicationException {
  using InvalidApplicationException::InvalidApplicationException;
};
struct IfInvalidApplicationException : public InvalidApplicationException {
  using InvalidApplicationException::InvalidApplicationException;
};
struct DefineInvalidApplicationException : public InvalidApplicationException {
  using InvalidApplicationException::InvalidApplicationException;
};
struct LambdaInvalidApplicationException : public InvalidApplicationException {
  using InvalidApplicationException::InvalidApplicationException;
};

struct UnboundVariableException : public Exception {
  std::string const str;
  UnboundVariableException(std::string_view f, int l, std::string_view str_) : Exception{f, l}, str{str_} {}
};

template<typename T>
[[noreturn]] void raise_(std::string_view file, int line) {
  throw T{file, line};
}
template<typename T>
[[noreturn]] void raise_(std::string_view file, int line, char c) {
  throw T{file, line, c};
}
template<typename T>
[[noreturn]] void raise_(std::string_view file, int line, std::string_view str) {
  throw T{file, line, str};
}

#define raise(Type) raise_<Type>(__FILE__, __LINE__)
#define raise_with_char(Type, c) raise_<Type>(__FILE__, __LINE__, c)
#define raise_with_str(Type, str) raise_<Type>(__FILE__, __LINE__, str)
