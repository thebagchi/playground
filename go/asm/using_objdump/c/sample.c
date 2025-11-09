#include <stdint.h>

#define STACK_CALL __attribute__((force_align_arg_pointer))
#define C_DECL __attribute__((cdecl))

static const int64_t KCONSTANT = 42;

STACK_CALL C_DECL int64_t AddInt64(int64_t a, int64_t b) {
  // Function to add 2 int64_t and return int64_t
  return a + b;
}

STACK_CALL C_DECL int64_t AddConstInt64(int64_t a) {
  // Function to add KCONSTANT to int64_t and return int64_t
  return a + KCONSTANT;
}
