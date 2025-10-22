#include <cstdint>
#include <iostream>
#include <tuple>

#include "cpuid.h"

std::tuple<std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t>
get_cpuid(const std::uint32_t leaf = 0) {
  uint32_t eax, ebx, ecx, edx;
  __get_cpuid(leaf, &eax, &ebx, &ecx, &edx);
  return std::make_tuple(eax, ebx, ecx, edx);
}

std::tuple<std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t>
get_cpuid_count(const std::uint32_t leaf = 0, const std::uint32_t subleaf = 0) {
  uint32_t eax, ebx, ecx, edx;
  __get_cpuid_count(leaf, 0, &eax, &ebx, &ecx, &edx);
  return std::make_tuple(eax, ebx, ecx, edx);
}

bool has_sse3() {
  auto [eax, ebx, ecx, edx] = get_cpuid(1);
  return bit_SSE3 & ecx;
}

bool has_ssse3() {
  auto [eax, ebx, ecx, edx] = get_cpuid(1);
  return bit_SSSE3 & ecx;
}

bool has_sse41() {
  auto [eax, ebx, ecx, edx] = get_cpuid(1);
  return bit_SSE4_1 & ecx;
}

bool has_sse42() {
  auto [eax, ebx, ecx, edx] = get_cpuid(1);
  return bit_SSE4_2 & ecx;
}

bool has_avx() {
  auto [eax, ebx, ecx, edx] = get_cpuid(1);
  return bit_AVX & ecx;
}

bool has_mmx() {
  auto [eax, ebx, ecx, edx] = get_cpuid(1);
  return bit_MMX & edx;
}

bool has_sse() {
  auto [eax, ebx, ecx, edx] = get_cpuid(1);
  return bit_SSE & edx;
}

bool has_sse2() {
  auto [eax, ebx, ecx, edx] = get_cpuid(1);
  return bit_SSE2 & edx;
}

bool has_avx2() {
  auto [eax, ebx, ecx, edx] = get_cpuid_count(7);
  return bit_AVX2 & ebx;
}

bool has_avx512f() {
  auto [eax, ebx, ecx, edx] = get_cpuid_count(7);
  return bit_AVX512F & ebx;
}

bool has_avx512dq() {
  auto [eax, ebx, ecx, edx] = get_cpuid_count(7);
  return bit_AVX512DQ & ebx;
}

bool has_avx512pf() {
  auto [eax, ebx, ecx, edx] = get_cpuid_count(7);
  return bit_AVX512PF & ebx;
}

bool has_avx512er() {
  auto [eax, ebx, ecx, edx] = get_cpuid_count(7);
  return bit_AVX512ER & ebx;
}

bool has_avx512cd() {
  auto [eax, ebx, ecx, edx] = get_cpuid_count(7);
  return bit_AVX512CD & ebx;
}

bool has_avx512bw() {
  auto [eax, ebx, ecx, edx] = get_cpuid_count(7);
  return bit_AVX512BW & ebx;
}

bool has_avx512vl() {
  auto [eax, ebx, ecx, edx] = get_cpuid_count(7);
  return bit_AVX512VL & ebx;
}

bool has_avx512vbmi() {
  auto [eax, ebx, ecx, edx] = get_cpuid_count(7);
  return bit_AVX512VBMI & ecx;
}

bool has_avx512vbmi2() {
  auto [eax, ebx, ecx, edx] = get_cpuid_count(7);
  return bit_AVX512VBMI2 & ecx;
}

bool has_avx512vnni() {
  auto [eax, ebx, ecx, edx] = get_cpuid_count(7);
  return bit_AVX512VNNI & ecx;
}

bool has_avx512bitalg() {
  auto [eax, ebx, ecx, edx] = get_cpuid_count(7);
  return bit_AVX512BITALG & ecx;
}

bool has_avx512vpopcntdq() {
  auto [eax, ebx, ecx, edx] = get_cpuid_count(7);
  return bit_AVX512VPOPCNTDQ & ecx;
}

bool has_avx512vnniw() {
  auto [eax, ebx, ecx, edx] = get_cpuid_count(7);
  return bit_AVX5124VNNIW & edx;
}

bool has_avx5124fmaps() {
  auto [eax, ebx, ecx, edx] = get_cpuid_count(7);
  return bit_AVX5124FMAPS & edx;
}

bool has_avx512fp16() {
  auto [eax, ebx, ecx, edx] = get_cpuid_count(7);
  return bit_AVX512FP16 & edx;
}

bool has_avxvnni() {
  auto [eax, ebx, ecx, edx] = get_cpuid_count(7, 1);
  return bit_AVXVNNI & eax;
}

bool has_avx512bf16() {
  auto [eax, ebx, ecx, edx] = get_cpuid_count(7, 1);
  return bit_AVX512BF16 & eax;
}

bool has_avxifma() {
  auto [eax, ebx, ecx, edx] = get_cpuid_count(7, 1);
  return bit_AVXIFMA & eax;
}

bool has_avxvnniint8() {
  auto [eax, ebx, ecx, edx] = get_cpuid_count(7, 1);
  return bit_AVXVNNIINT8 & edx;
}

bool has_avxneconvert() {
  auto [eax, ebx, ecx, edx] = get_cpuid_count(7, 1);
  return bit_AVXNECONVERT & edx;
}

bool has_sse4a() {
  auto [eax, ebx, ecx, edx] = get_cpuid(0x80000001);
  return bit_SSE4a & ecx;
}

int main(int argc, char** argv) {
  if (has_sse3()) {
    std::cout << "has sse3 instructions ..." << std::endl;
  }
  if (has_ssse3()) {
    std::cout << "has ssse3 instructions ..." << std::endl;
  }
  if (has_sse41()) {
    std::cout << "has sse41 instructions ..." << std::endl;
  }
  if (has_sse42()) {
    std::cout << "has sse42 instructions ..." << std::endl;
  }
  if (has_avx()) {
    std::cout << "has avx instructions ..." << std::endl;
  }
  if (has_mmx()) {
    std::cout << "has mmx instructions ..." << std::endl;
  }
  if (has_sse()) {
    std::cout << "has sse instructions ..." << std::endl;
  }
  if (has_sse2()) {
    std::cout << "has sse2 instructions ..." << std::endl;
  }
  if (has_avx2()) {
    std::cout << "has avx2 instructions ..." << std::endl;
  }
  if (has_avx512f()) {
    std::cout << "has avx512f instructions ..." << std::endl;
  }
  if (has_avx512dq()) {
    std::cout << "has avx512dq instructions ..." << std::endl;
  }
  if (has_avx512pf()) {
    std::cout << "has avx512pf instructions ..." << std::endl;
  }
  if (has_avx512er()) {
    std::cout << "has avx512er instructions ..." << std::endl;
  }
  if (has_avx512cd()) {
    std::cout << "has avx512cd instructions ..." << std::endl;
  }
  if (has_avx512bw()) {
    std::cout << "has avx512bw instructions ..." << std::endl;
  }
  if (has_avx512vl()) {
    std::cout << "has avx512vl instructions ..." << std::endl;
  }
  if (has_avx512vbmi()) {
    std::cout << "has avx512vbmi instructions ..." << std::endl;
  }
  if (has_avx512vbmi2()) {
    std::cout << "has avx512vbmi2 instructions ..." << std::endl;
  }
  if (has_avx512vnni()) {
    std::cout << "has avx512vnni instructions ..." << std::endl;
  }
  if (has_avx512bitalg()) {
    std::cout << "has avx512bitalg instructions ..." << std::endl;
  }
  if (has_avx512vpopcntdq()) {
    std::cout << "has avx512vpopcntdq instructions ..." << std::endl;
  }
  if (has_avx512vnniw()) {
    std::cout << "has avx512vnniw instructions ..." << std::endl;
  }
  if (has_avx5124fmaps()) {
    std::cout << "has avx5124fmaps instructions ..." << std::endl;
  }
  if (has_avx512fp16()) {
    std::cout << "has avx512fp16 instructions ..." << std::endl;
  }
  if (has_avxvnni()) {
    std::cout << "has avxvnni instructions ..." << std::endl;
  }
  if (has_avx512bf16()) {
    std::cout << "has avx512bf16 instructions ..." << std::endl;
  }
  if (has_avxifma()) {
    std::cout << "has avxifma instructions ..." << std::endl;
  }
  if (has_avxvnniint8()) {
    std::cout << "has avxvnniint8 instructions ..." << std::endl;
  }
  if (has_avxneconvert()) {
    std::cout << "has avxneconvert instructions ..." << std::endl;
  }
  if (has_sse4a()) {
    std::cout << "has sse4a instructions ..." << std::endl;
  }
  return 0;
}