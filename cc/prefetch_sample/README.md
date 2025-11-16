# Prefetch Benchmark

This project demonstrates the impact of software prefetching on memory access performance in C++.

## What is Prefetching?

Prefetching is a technique used to improve memory access performance by loading data into the CPU cache before it's actually needed. This reduces cache misses and can significantly speed up sequential memory access patterns.

### How It Works

- **Hardware Prefetching**: Modern CPUs automatically detect sequential access patterns and prefetch data.
- **Software Prefetching**: Programmers can explicitly hint to the CPU to prefetch specific memory locations using intrinsics like `_mm_prefetch()`. Different hint parameters control which cache level the data is loaded into:
  - `_MM_HINT_T0`: Prefetch to L1 cache (used in this benchmark)
  - `_MM_HINT_T1`: Prefetch to L2 cache
  - `_MM_HINT_T2`: Prefetch to L3 cache
  - `_MM_HINT_NTA`: Non-temporal aligned prefetch (for streaming data)

## CPU Cache Hierarchy

Modern CPUs have multiple levels of cache to bridge the speed gap between fast CPU cores and slower main memory:

- **L1 Cache**: Smallest and fastest (typically 32-64 KB per core). Holds most recently used data.
- **L2 Cache**: Medium size and speed (typically 256 KB - 1 MB per core). Acts as a buffer between L1 and L3.
- **L3 Cache**: Largest but slowest cache level (typically 2-32 MB, shared across cores). Last stop before main memory.

### Determining Cache Sizes Programmatically

Cache sizes can be determined programmatically using several methods:

- **CPUID instruction**: Use CPUID leaf 0x04 (Deterministic Cache Parameters) to query detailed cache information. This provides cache level, type, size, line size, associativity, and other parameters for each cache level.
- **Command line**: `lscpu` or `getconf` commands can show cache information

This benchmark program automatically detects and displays the system's cache sizes at runtime.

Prefetching helps reduce cache misses by loading data into these caches before it's needed, improving overall memory access performance.

## Benchmark Overview

This benchmark compares three memory access patterns:

1. **Sequential (no prefetch)**: Standard sequential array traversal
2. **Sequential (with prefetch)**: Sequential traversal with explicit prefetch hints
3. **Random access**: Shuffled array access (poor cache locality)

## Key Findings

- Prefetching can improve performance for predictable access patterns
- However, incorrect prefetching (too early/late) can hurt performance
- Random access suffers greatly due to poor cache locality

## Building and Running

### Prerequisites
- GCC compiler with C++17 support
- Linux environment

### Build
```bash
./build.sh
```

### Run
```bash
./prefetching.bin
```