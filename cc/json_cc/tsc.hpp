#ifndef TSC_HPP_INCLUDED
#define TSC_HPP_INCLUDED

#include <cstdint>

__always_inline uint64_t read_tsc() {
  union {
    uint64_t tsc_;
    struct {
      uint32_t lo_;
      uint32_t hi_;
    };
  } tsc{};
  __asm__ __volatile__("rdtsc" : "=a"(tsc.lo_), "=d"(tsc.hi_));
  return tsc.tsc_;
}

#endif  // TSC_HPP_INCLUDED