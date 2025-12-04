// prefetch_benchmark.cpp
// Compile: g++ -O2 -march=native -std=c++17 prefetch_benchmark.cpp -o
// prefetch_benchmark Run: ./prefetch_benchmark

#include <cpuid.h>     // For __cpuid
#include <xmmintrin.h> // For _mm_prefetch

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <optional>
#include <random>
#include <string>
#include <tuple>
#include <vector>

// Configuration constants
namespace config {
  constexpr std::size_t DEFAULT_ARRAY_SIZE = 10 * 1024 * 1024 / sizeof(int);
  constexpr std::int32_t NUM_RUNS = 100;
  constexpr std::int32_t WARMUP_RUNS = 5;
  const std::size_t DEFAULT_PREFETCH_DISTANCE = 64;
  const std::size_t DEFAULT_PREFETCH_STRIDE = 8;
} // namespace config

// RAII timer class for automatic timing
class Timer {
  std::chrono::high_resolution_clock::time_point start_;
  double& result_;
public:
  explicit Timer(double& result) noexcept
      : start_(std::chrono::high_resolution_clock::now()), result_(result) {
  }

  ~Timer() {
    auto end = std::chrono::high_resolution_clock::now();
    result_ = std::chrono::duration<double>(end - start_).count();
  }
};

// Helper: print byte counts in a short human-readable form (e.g., 1.5M)
// Prints directly to the provided ostream and restores stream flags/precision.
static void print_bytes_short(std::ostream& out, std::uint64_t bytes) noexcept {
  auto flags = out.flags();
  auto prec = out.precision();

  if (bytes >= 1024ull * 1024ull) {
    double m = static_cast<double>(bytes) / (1024.0 * 1024.0);
    out << std::fixed << std::setprecision(1) << m << "M";
  } else if (bytes >= 1024ull) {
    out << static_cast<std::uint64_t>(bytes / 1024ull) << "K";
  } else {
    out << bytes << "B";
  }

  out.flags(flags);
  out.precision(prec);
}

template <typename T> class Allocator {
public:
  using value_type = T;

  Allocator() = default;

  template <typename U> Allocator(const Allocator<U>&) noexcept {
    // Templated constructors cannot be defaulted
  }

  T* allocate(std::size_t n) {
    // Required by std::allocator_traits: allocates n elements
    // with 64-byte alignment for cache efficiency
    if (n > std::numeric_limits<std::size_t>::max() / sizeof(T)) {
      throw std::bad_alloc();
    }
    void* ptr = aligned_alloc(64, n * sizeof(T));
    if (!ptr) {
      throw std::bad_alloc();
    }
    return static_cast<T*>(ptr);
  }

  void deallocate(T* p, std::size_t) noexcept {
    // Required by std::allocator_traits: deallocates memory allocated
    free(p);
  }

  // Comparison operators as member functions
  template <typename U> bool operator==(const Allocator<U>&) const noexcept {
    // Allocators are equal if they have the same allocation strategy
    return true;
  }

  template <typename U> bool operator!=(const Allocator<U>&) const noexcept {
    // Allocators are never unequal for this stateless implementation
    return false;
  }
};

std::size_t ARRAY_SIZE = config::DEFAULT_ARRAY_SIZE;
std::size_t PREFETCH_DISTANCE = config::DEFAULT_PREFETCH_DISTANCE;
std::size_t PREFETCH_STRIDE = config::DEFAULT_PREFETCH_STRIDE;

// Sequential traversal without prefetch
[[nodiscard]] std::int64_t sequential_no_prefetch(
 const std::int32_t* arr, std::size_t size) noexcept {
  std::int64_t sum = 0;
  for (std::size_t i = 0; i < size; ++i) {
    sum = sum + *(arr + i);
  }
  return sum;
}

// Sequential traversal WITH prefetch
[[nodiscard]] std::int64_t sequential_with_prefetch(const std::int32_t* arr,
 std::size_t size,
 std::size_t distance = config::DEFAULT_PREFETCH_DISTANCE,
 std::size_t stride = config::DEFAULT_PREFETCH_STRIDE) noexcept {
  std::int64_t sum = 0;
  for (std::size_t i = 0; i < size; ++i) {
    // Prefetch far enough ahead
    if ((i % stride) == 0 && i + distance < size) {
      _mm_prefetch(reinterpret_cast<const char*>(arr + i + distance), _MM_HINT_T0);
    }
    sum = sum + *(arr + i);
  }
  return sum;
}

// Random access (poor cache behavior)
[[nodiscard]] std::int64_t random_access(const std::int32_t* arr, std::size_t size) {
  std::int64_t sum = 0;
  std::vector<std::size_t> indices(size);
  std::iota(indices.begin(), indices.end(), 0);

  // Shuffle indices
  std::random_device rd;
  std::mt19937 gen(rd());
  std::shuffle(indices.begin(), indices.end(), gen);

  for (std::size_t i = 0; i < indices.size(); ++i) {
    sum = sum + *(arr + indices[i]);
  }
  return sum;
}

template <typename T> std::optional<std::tuple<std::size_t, std::size_t>> print_cache_sizes() {
  std::cout << "Cache sizes (via CPUID):" << std::endl;

  // Check if CPUID leaf 0x04 is supported
  std::uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;
  __cpuid(0, eax, ebx, ecx, edx);
  if (eax < 0x04) {
    std::cerr << "Error: CPUID leaf 0x04 not supported" << std::endl;
    return std::nullopt;
  }

  std::size_t l1_cache_size = 0;
  std::uint32_t line_size_l1 = 64; // default if unknown; updated when level==1
  std::size_t prefetch_distance = config::DEFAULT_PREFETCH_DISTANCE;
  std::size_t prefetch_stride = config::DEFAULT_PREFETCH_STRIDE;

  // Use CPUID leaf 0x04 (Deterministic Cache Parameters)
  for (int i = 0;; ++i) {
    __cpuid_count(0x04, i, eax, ebx, ecx, edx);

    // If cache type is 0, we've reached the end
    std::uint32_t cache_type = (eax >> 5) & 0x7;
    if (cache_type == 0) {
      break;
    }

    // Extract cache information
    std::uint32_t level = (eax >> 5) & 0x7;
    std::uint32_t line_size = (ebx & 0xFFF) + 1;
    std::uint32_t partitions = ((ebx >> 12) & 0x3FF) + 1;
    std::uint32_t ways = ((ebx >> 22) & 0x3FF) + 1;
    std::uint32_t sets = ecx + 1;

    // Calculate cache size in bytes
    std::uint64_t cache_size = static_cast<std::uint64_t>(line_size) * partitions * ways * sets;

    std::cout << "  L" << level << ": ";
    print_bytes_short(std::cout, cache_size);
    std::cout << std::endl;

    // Store L1 cache size for prefetch calculations
    if (level == 1) {
      l1_cache_size = cache_size;
      line_size_l1 = line_size;
    }
  }

  // Calculate prefetch parameters based on L1 cache size
  if (l1_cache_size > 0) {
    // Prefetch distance in cache lines: ~1/8 of L1 cache
    const std::size_t distance_lines = l1_cache_size / (8 * line_size_l1);

    // Convert to elements (T) so the returned distance can be used directly
    // by array indexing: elements_per_line = ceil(line_size / sizeof(T)).
    const std::size_t elements_per_line = (line_size_l1 + sizeof(T) - 1) / sizeof(T);
    prefetch_distance = std::max<std::size_t>(1, distance_lines * elements_per_line);

    // Prefetch stride: choose a reasonable iteration stride expressed in
    // elements. We'll prefetch every 4 cache lines, expressed in elements.
    prefetch_stride = std::max<std::size_t>(1, elements_per_line * 4);

    std::cout << "Optimum prefetch distance: " << distance_lines << " cache lines (~"
              << prefetch_distance << " elements)" << std::endl;
    std::cout << "Optimum prefetch stride: " << prefetch_stride << " iterations" << std::endl;
  } else {
    // Fallback values if L1 cache not found
    std::cout << "Using fallback prefetch parameters" << std::endl;
  }

  return std::make_optional(std::make_tuple(prefetch_distance, prefetch_stride));
}

int main() {
  std::cout << "Array size: " << ARRAY_SIZE << " integers (~" << std::fixed << std::setprecision(1)
            << ARRAY_SIZE * sizeof(int) / (1024.0 * 1024.0) << " MB)" << std::endl;

  auto cache_params = print_cache_sizes<int>();
  if (cache_params.has_value()) {
    auto [calculated_distance, calculated_stride] = cache_params.value();
    PREFETCH_DISTANCE = calculated_distance;
    PREFETCH_STRIDE = calculated_stride;
  } else {
    std::cout << "Using default prefetch parameters" << std::endl;
  }
  std::cout << std::endl;

  // Allocate aligned memory
  std::vector<std::int32_t, Allocator<std::int32_t>> arr(ARRAY_SIZE);
  if (arr.data() == nullptr) {
    std::cerr << "vector allocation failed" << std::endl;
    return 1;
  }

  // Verify 64-byte alignment
  if (reinterpret_cast<std::uintptr_t>(arr.data()) % 64 != 0) {
    std::cerr << "Warning: Array not 64-byte aligned" << std::endl;
  }

  // Initialize array
  for (std::size_t i = 0; i < ARRAY_SIZE; ++i) {
    *(arr.data() + i) = static_cast<std::int32_t>(i);
  }

  // Warm up cache (optional, but helps consistency)
  std::cout << "Warming up..." << std::endl;
  volatile std::int64_t dummy = 0;
  for (std::int32_t r = 0; r < config::WARMUP_RUNS; ++r) {
    for (std::size_t i = 0; i < ARRAY_SIZE; ++i) {
      dummy = dummy + *(arr.data() + i);
    }
  }

  // Run benchmarks multiple times and average
  double time_without_prefetch = 0.0;
  double time_with_prefetch = 0.0;
  double time_random_access = 0.0;

  {
    Timer timer(time_without_prefetch);
    volatile std::int64_t result = 0;
    for (std::int32_t r = 0; r < config::NUM_RUNS; ++r) {
      result = sequential_no_prefetch(arr.data(), ARRAY_SIZE);
    }
    // Use result to prevent optimization
    if (result == 0) {
      std::cout << "";
    }
  }
  time_without_prefetch = time_without_prefetch / config::NUM_RUNS;

  {
    Timer timer(time_with_prefetch);
    volatile std::int64_t result = 0;
    for (std::int32_t r = 0; r < config::NUM_RUNS; ++r) {
      result = sequential_with_prefetch(arr.data(), ARRAY_SIZE, PREFETCH_DISTANCE, PREFETCH_STRIDE);
    }
    if (result == 0) {
      std::cout << "";
    }
  }
  time_with_prefetch = time_with_prefetch / config::NUM_RUNS;

  {
    Timer timer(time_random_access);
    volatile std::int64_t result = 0;
    for (std::int32_t r = 0; r < config::NUM_RUNS; ++r) {
      result = random_access(arr.data(), ARRAY_SIZE);
    }
    if (result == 0) {
      std::cout << "";
    }
  }
  time_random_access = time_random_access / config::NUM_RUNS;

  // Results
  std::cout << "\n=== Performance Results (avg of " << config::NUM_RUNS
            << " runs) ===" << std::endl;
  std::cout << "Sequential (no prefetch):    " << std::fixed << std::setprecision(4)
            << time_without_prefetch << " sec" << std::endl;
  std::cout << "Sequential (with prefetch):  " << std::fixed << std::setprecision(4)
            << time_with_prefetch << " sec  ";
  if (time_with_prefetch < time_without_prefetch) {
    std::cout << "(" << std::fixed << std::setprecision(1)
              << (time_without_prefetch - time_with_prefetch) / time_without_prefetch * 100
              << "% faster)" << std::endl;
  } else {
    std::cout << "(slower)" << std::endl;
  }

  std::cout << "Random access:               " << std::fixed << std::setprecision(4)
            << time_random_access << " sec  ";
  std::cout << "(" << std::fixed << std::setprecision(1)
            << time_random_access / time_without_prefetch << "x slower than no-prefetch)"
            << std::endl;

  std::cout << "\nBandwidth estimate (sequential no prefetch):" << std::endl;
  double gb = ARRAY_SIZE * sizeof(int) / (1024.0 * 1024.0 * 1024.0);
  std::cout << "  " << std::fixed << std::setprecision(2) << gb / time_without_prefetch << " GB/s"
            << std::endl;

  return 0;
}