#pragma once

#include <list>
#include <algorithm>

template<typename T>
class Allocator_ {
  using element_type = typename T::element_type;
  std::list<std::pair<bool, element_type*>> list;
public:
  Allocator_(): list{} {}

  template<class... Args>
  T New(Args... args) {
    element_type* t = new element_type{std::forward<Args>(args)...};
    list.emplace_back(false, t);
    return T{&list.back()};
  }
  void collect() {
    auto result = std::remove_if(begin(list), end(list), [](auto p) { return !p.first; });
    std::for_each(result, end(list), [](auto p) { delete p.second; });
    list.erase(result, end(list));
    std::for_each(begin(list), end(list), [](auto& p) { p.first = false; });
  }
};

template<typename T>
Allocator_<T>& Allocator() {
  static Allocator_<T> allocator{};
  return allocator;
}
