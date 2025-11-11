#include <stdint.h>

static const int8_t CONSTANT_I8 = 42;
static const int16_t CONSTANT_I16 = 42;
static const int32_t CONSTANT_I32 = 42;
static const int64_t CONSTANT_I64 = 42;

static const uint8_t CONSTANT_U8 = 42;
static const uint16_t CONSTANT_U16 = 42;
static const uint32_t CONSTANT_U32 = 42;
static const uint64_t CONSTANT_U64 = 42;

int8_t AddInt8(int8_t a, int8_t b) {
  // Function to add 2 int8_t and return int8_t
  return a + b;
}

int8_t AddConstInt8(int8_t a) {
  // Function to add CONSTANT_I8 to int8_t and return int8_t
  return a + CONSTANT_I8;
}

uint8_t AddUint8(uint8_t a, uint8_t b) {
  // Function to add 2 uint8_t and return uint8_t
  return a + b;
}

uint8_t AddConstUint8(uint8_t a) {
  // Function to add CONSTANT_U8 to uint8_t and return uint8_t
  return a + CONSTANT_U8;
}

int16_t AddInt16(int16_t a, int16_t b) {
  // Function to add 2 int16_t and return int16_t
  return a + b;
}

int16_t AddConstInt16(int16_t a) {
  // Function to add CONSTANT_I16 to int16_t and return int16_t
  return a + CONSTANT_I16;
}

uint16_t AddUint16(uint16_t a, uint16_t b) {
  // Function to add 2 uint16_t and return uint16_t
  return a + b;
}

uint16_t AddConstUint16(uint16_t a) {
  // Function to add CONSTANT_U16 to uint16_t and return uint16_t
  return a + CONSTANT_U16;
}

int32_t AddInt32(int32_t a, int32_t b) {
  // Function to add 2 int32_t and return int32_t
  return a + b;
}

int32_t AddConstInt32(int32_t a) {
  // Function to add CONSTANT_I32 to int32_t and return int32_t
  return a + CONSTANT_I32;
}

uint32_t AddUint32(uint32_t a, uint32_t b) {
  // Function to add 2 uint32_t and return uint32_t
  return a + b;
}

uint32_t AddConstUint32(uint32_t a) {
  // Function to add CONSTANT_U32 to uint32_t and return uint32_t
  return a + CONSTANT_U32;
}

int64_t AddInt64(int64_t a, int64_t b) {
  // Function to add 2 int64_t and return int64_t
  return a + b;
}

int64_t AddConstInt64(int64_t a) {
  // Function to add KCONSTANT to int64_t and return int64_t
  return a + CONSTANT_I64;
}

uint64_t AddUint64(uint64_t a, uint64_t b) {
  // Function to add 2 uint64_t and return uint64_t
  return a + b;
}

uint64_t AddConstUint64(uint64_t a) {
  // Function to add CONSTANT_U64 to uint64_t and return uint64_t
  return a + CONSTANT_U64;
}
