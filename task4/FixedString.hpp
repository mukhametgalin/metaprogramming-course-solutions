#pragma once

#include <string>

template<size_t max_length>
struct FixedString {
  constexpr FixedString(const char* string, size_t len): string(string), len(len) {}
  constexpr operator std::string_view() const {
    return {string, len};
  }

  const char* string;
  size_t len;
};

template <FixedString<256> a>
constexpr auto operator""_cstr() {
  return a;
}

template <typename sym, sym... str>
struct Helper {
  static constexpr sym data[] = {str ...};
  static constexpr size_t size = sizeof(data) / sizeof(sym);
};

template <typename sym, sym... str>
static constexpr FixedString<256> operator""_cstr() {
  return FixedString<256>(Helper<sym, str...>::data, Helper<sym, str...>::size);
}