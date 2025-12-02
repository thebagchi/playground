#ifndef TYPING_H_INCLUDED
#define TYPING_H_INCLUDED

#include <string_view>

template <typename T> constexpr std::string_view type_name() {
#if defined(__clang__)
  constexpr auto prefix = "[T = ";
  constexpr auto suffix = "]";
  constexpr auto function = __PRETTY_FUNCTION__;
#elif defined(__GNUC__)
  constexpr auto prefix = "with T = ";
  constexpr auto suffix = "]";
  constexpr auto function = __PRETTY_FUNCTION__;
#else
  static_assert(false, "Only GCC/Clang supported");
#endif

  std::string_view name{ function };
  name.remove_prefix(name.find(prefix) + sizeof(prefix) - 1);
  name.remove_suffix(name.size() - name.rfind(suffix));

  // strip struct/class/enum
  if (auto pos = name.find_first_not_of(" "); pos != 0) {
    name.remove_prefix(pos);
  }
  for (auto p : { "struct ", "class ", "enum class ", "enum ", "union " }) {
    if (name.starts_with(p)) {
      name.remove_prefix(strlen(p));
      break;
    }
  }

  while (!name.empty() && name.front() == ' ') {
    name.remove_prefix(1);
  }
  while (!name.empty() && name.back() == ' ') {
    name.remove_suffix(1);
  }

  return name;
}

#endif // TYPING_H_INCLUDED