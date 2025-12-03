#ifndef TYPING_H_INCLUDED
#define TYPING_H_INCLUDED

#include <string_view>
#include <array>
#include <cstddef>

// Compiler detection for __PRETTY_FUNCTION__ support
#if defined(__clang__)
// Clang supports __PRETTY_FUNCTION__ similar to GCC
#define HAS_PRETTY_FUNCTION 1
#elif defined(__GNUC__) || defined(__GNUG__)
// GCC supports __PRETTY_FUNCTION__
#define HAS_PRETTY_FUNCTION 1
#else
// Other compilers may not support __PRETTY_FUNCTION__
#define HAS_PRETTY_FUNCTION 0
#warning "Compiler may not support __PRETTY_FUNCTION__ for type name extraction"
#endif

// Common prefixes to remove from type names
constexpr std::array<std::string_view, 5> prefixes{
  "struct ", "class ", "enum class ", "enum ", "union "
};

// =============================================================================
// Type Name Extraction Macros
// =============================================================================

/**
 * @brief Macro to get full type name without angle brackets
 * Usage: FNAME(Type) instead of fname<Type>()
 */
#define FNAME(T) fname<T>()

/**
 * @brief Macro to get short type name without angle brackets
 * Usage: SNAME(Type) instead of sname<Type>()
 */
#define SNAME(T) sname<T>()

// Compile-time string views for prefixes
constexpr std::string_view prefix("Name");

// Empty template struct used as a marker for type extraction
template <typename T> struct Name {
  // Empty
};

// Extracts the raw __PRETTY_FUNCTION__ string at compile time
// Compatible with GCC and Clang compilers
template <typename T> constexpr auto RAW() {
  static_assert(HAS_PRETTY_FUNCTION,
                "Compiler must support __PRETTY_FUNCTION__ for type name extraction");

  std::array<char, sizeof(__PRETTY_FUNCTION__)> name{};
  for (std::size_t i = 0; i < name.size(); ++i) {
    name[i] = __PRETTY_FUNCTION__[i];
  }
  return name;
}

// Compile-time constant containing the __PRETTY_FUNCTION__ for each type
// specifically the constructor name Name<T>
template <typename T> constexpr decltype(RAW<Name<T>>()) name = RAW<Name<T>>();

// Extracts the full type name from __PRETTY_FUNCTION__
// Removes template wrapper and prefixes like "struct"/"class"
// Compatible with GCC and Clang
template <typename T> constexpr std::string_view fname() {
  // Get the raw function name string
  std::string_view raw_name(name<T>.data(), name<T>.size());

  // Find the position after "Name" prefix
  auto pos = raw_name.find(prefix);
  if (pos == std::string_view::npos) {
    return {}; // Error case
  }
  raw_name.remove_prefix(pos + prefix.size());

  // Find the balanced template brackets to extract the type
  int bracket_count = 0;
  std::size_t end_pos = 0;
  for (std::size_t i = 0; i < raw_name.size(); ++i) {
    char c = raw_name[i];
    if (c == '<') {
      ++bracket_count;
    } else if (c == '>') {
      --bracket_count;
      if (bracket_count == 0) {
        end_pos = i;
        break;
      }
    }
  }

  // Extract the type part between brackets
  if (end_pos > 0) {
    raw_name = raw_name.substr(1, end_pos - 1); // Remove outer <>
  }

  // Remove common prefixes like "struct ", "class ", etc.
  for (const auto& prefix : prefixes) {
    if (raw_name.substr(0, prefix.size()) == prefix) {
      raw_name.remove_prefix(prefix.size());
      break;
    }
  }

  // Trim leading and trailing spaces
  while (!raw_name.empty() && raw_name.front() == ' ') {
    raw_name.remove_prefix(1);
  }
  while (!raw_name.empty() && raw_name.back() == ' ') {
    raw_name.remove_suffix(1);
  }

  return raw_name;
}

// Convenience function for lvalue references
template <typename T> constexpr std::string_view fnameOf(T&) {
  return fname<T>();
}

// Extracts the short type name (everything after the last ::)
// Compatible with GCC and Clang
template <typename T> constexpr std::string_view sname() {
  auto name = fname<T>();

  // Find the last :: outside of template brackets
  int bracket_count = 0;
  std::size_t last_colon_pos = std::string_view::npos;

  for (std::size_t i = 0; i < name.size(); ++i) {
    char c = name[i];
    if (c == '<') {
      ++bracket_count;
    } else if (c == '>') {
      --bracket_count;
    } else if (c == ':' && bracket_count == 0 && i + 1 < name.size() && name[i + 1] == ':') {
      last_colon_pos = i;
      ++i; // Skip the second :
    }
  }

  // Remove everything before the last ::
  if (last_colon_pos != std::string_view::npos) {
    name.remove_prefix(last_colon_pos + 2);
  }

  return name;
}

// Convenience function for lvalue references
template <typename T> constexpr std::string_view snameOf(T&) {
  return sname<T>();
}

#endif // TYPING_H_INCLUDED