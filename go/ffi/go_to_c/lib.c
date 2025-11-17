#include <stdint.h>

// Simple C function to add two integers
// This can be called from other languages via FFI
uint64_t Add(uint64_t a, uint64_t b) {
  // Add 2 integers
  return a + b;
}