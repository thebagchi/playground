#if __cplusplus < 201103L
#error "This code requires C++11 or later"
#endif

#include <iostream>

template <typename... T> constexpr void count_params(T... args) {
  static_assert(sizeof...(args) == 10, "please provide exactly 10 arguments to the function");
}

#if __cplusplus == 201103L
// C++11 constexpr functions have restrictions:
// - Must contain only a single return statement
// - Cannot have loops, local variables, or multiple statements
// - Must use recursion and ternary operators for any logic
// Hence we use macros and recursion to calculate string length
#define PTR_DATA(PTR) !(!(*PTR))
#define NOT_NULL(PTR) ((PTR) ? PTR_DATA(PTR) : 0)
#define INCR_PTR(PTR) PTR + 1
constexpr std::size_t string_len(const char* ptr) {
  return NOT_NULL(ptr) ? PTR_DATA(ptr) + string_len(INCR_PTR(ptr)) : 0;
}
#undef INCR_PTR
#undef NOT_NULL
#undef PTR_DATA
#elif __cplusplus >= 201402L
constexpr std::size_t string_len(const char* ptr) {
  std::size_t len = 0;
  while (ptr && *ptr) {
    ++len;
    ++ptr;
  }
  return len;
}
#endif

int main() {
  count_params(0, 1, 2, 3, 4, 5, 6, 7, 8, 9);
#if SIM_ERROR
  // unblock to get an error
  count_params(1, 2, 3, 4, 5, 6, 7, 8, 9);
#endif
  static_assert(string_len("hello world") == 11);
#if SIM_ERROR
  // unblock to get an error
  static_assert(string_len("hello world") == 12);
#endif
  std::cout << string_len("hello world") << std::endl;

  std::cout << "hello world!!!" << std::endl;
  return 0;
}
