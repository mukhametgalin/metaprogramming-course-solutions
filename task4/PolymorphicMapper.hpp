#pragma once

#include <algorithm>
#include <optional>
#include <type_traits>


template <class From, auto target>
struct Mapping {
  template<typename Base>
  static constexpr std::optional<From> check_obj(const Base& object) {
    auto t =  dynamic_cast<const From*>(&object);
    if (t) {
      return *t;
    } else {
      return std::nullopt;
    }
  }

  template<typename Target>
  static constexpr void check_target() {
    static_assert(std::is_same_v<decltype(target), Target>);
  }

  static constexpr auto get_target() {
    return target;
  }
};

template <typename Base, typename Target>
static std::optional<Target> helper(const Base&) {
  return std::nullopt;
}

template <typename Base, typename Target, typename Head, typename... Tail>
static std::optional<Target> helper(const Base& object) {
  if (auto get = Head::check_obj(object)) {
    Head::template check_target<Target>();
    return Head::get_target();
  }
  return helper<Base, Target, Tail...>(object);
}

template <class Base, class Target, class... Mappings>
struct PolymorphicMapper {
  static std::optional<Target> map(const Base& object) {
    return helper<Base, Target, Mappings...>(object);
  }
};
