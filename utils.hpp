#pragma once

#include <algorithm>
#include <experimental/array>

template<typename T, size_t N>
bool in(T const& c, std::array<T, N> const& chars) {
  return std::any_of(begin(chars), end(chars), [&](T t) { return c == t; });
}
