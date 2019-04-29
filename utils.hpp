#pragma once

#include <algorithm>
#include <experimental/array>

template<typename T, size_t N>
bool in(T c, std::array<T, N> chars) {
  return std::any_of(begin(chars), end(chars), [c](T t) { return c == t; });
}
