#define BOOST_TEST_MODULE JSON Benchmark Tests
#include <boost/json.hpp>
#include <boost/test/unit_test.hpp>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <random>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include "json.h"

// =============================================================================
// Benchmark Utilities
// =============================================================================

class BenchmarkTimer {
public:
  using Clock = std::chrono::high_resolution_clock;
  using TimePoint = Clock::time_point;
  using Duration = std::chrono::nanoseconds;

  BenchmarkTimer() : start_(Clock::now()) {
  }

  void reset() {
    start_ = Clock::now();
  }

  Duration elapsed() const {
    return std::chrono::duration_cast<Duration>(Clock::now() - start_);
  }

  double elapsed_ms() const {
    return std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(elapsed()).count();
  }

  double elapsed_us() const {
    return std::chrono::duration_cast<std::chrono::duration<double, std::micro>>(elapsed()).count();
  }
private:
  TimePoint start_;
};

struct BenchmarkResult {
  std::string name;
  size_t iterations;
  double total_ms;
  double avg_us;
  double min_us;
  double max_us;

  void print() const {
    std::cout << "\n=== " << name << " ===\n";
    std::cout << "  Iterations: " << iterations << "\n";
    std::cout << "  Total time: " << total_ms << " ms\n";
    std::cout << "  Average:    " << avg_us << " μs/op\n";
    std::cout << "  Min:        " << min_us << " μs\n";
    std::cout << "  Max:        " << max_us << " μs\n";
    std::cout << "  Throughput: " << (1000000.0 / avg_us) << " ops/sec\n";
  }
};

template <typename Func>
BenchmarkResult benchmark(const std::string& name, Func&& func, size_t iterations = 1000) {
  std::vector<double> times;
  times.reserve(iterations);

  BenchmarkTimer total_timer;

  for (size_t i = 0; i < iterations; ++i) {
    BenchmarkTimer timer;
    func();
    times.push_back(timer.elapsed_us());
  }

  double total_ms = total_timer.elapsed_ms();
  double sum = 0.0;
  double min_time = times[0];
  double max_time = times[0];

  for (double t : times) {
    sum += t;
    if (t < min_time) {
      min_time = t;
    }
    if (t > max_time) {
      max_time = t;
    }
  }

  BenchmarkResult result;
  result.name = name;
  result.iterations = iterations;
  result.total_ms = total_ms;
  result.avg_us = sum / iterations;
  result.min_us = min_time;
  result.max_us = max_time;

  return result;
}

// =============================================================================
// Test Data Structures
// =============================================================================

struct Address {
  std::unique_ptr<std::string> street_;
  std::unique_ptr<std::string> city_;
  std::unique_ptr<std::string> country_;
  std::unique_ptr<std::string> postal_code_;

  constexpr const static auto properties =
      std::make_tuple(prop(&Address::street_, "street"),
                      prop(&Address::city_, "city"),
                      prop(&Address::country_, "country"),
                      prop<true>(&Address::postal_code_, "postal_code"));
};

struct Person {
  std::unique_ptr<std::string> name_;
  std::unique_ptr<std::uint64_t> age_;
  std::unique_ptr<std::string> email_;
  std::unique_ptr<Address> address_;
  std::vector<std::string> tags_;

  constexpr const static auto properties = std::make_tuple(prop(&Person::name_, "name"),
                                                           prop(&Person::age_, "age"),
                                                           prop<true>(&Person::email_, "email"),
                                                           prop<true>(&Person::address_, "address"),
                                                           prop(&Person::tags_, "tags"));
};

struct Company {
  std::unique_ptr<std::string> name_;
  std::vector<Person> employees_;
  std::map<std::string, std::string> metadata_;

  constexpr const static auto properties = std::make_tuple(prop(&Company::name_, "name"),
                                                           prop(&Company::employees_, "employees"),
                                                           prop(&Company::metadata_, "metadata"));
};

// Memory benchmark test structures
struct WithUnique {
  std::unique_ptr<std::string> value_;
  constexpr const static auto properties = std::make_tuple(prop(&WithUnique::value_, "value"));
};

struct WithShared {
  std::shared_ptr<std::string> value_;
  constexpr const static auto properties = std::make_tuple(prop(&WithShared::value_, "value"));
};

struct WithOptional {
  std::optional<std::string> value_;
  constexpr const static auto properties =
      std::make_tuple(prop<true>(&WithOptional::value_, "value"));
};

struct WithPointer {
  std::unique_ptr<std::string> value_;
  constexpr const static auto properties =
      std::make_tuple(prop<true>(&WithPointer::value_, "value"));
};

// =============================================================================
// Test Data Generators
// =============================================================================

std::string random_string(size_t length) {
  static const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  static std::mt19937 rng(std::random_device{}());
  static std::uniform_int_distribution<size_t> dist(0, sizeof(charset) - 2);

  std::string result;
  result.reserve(length);
  for (size_t i = 0; i < length; ++i) {
    result += charset[dist(rng)];
  }
  return result;
}

Person generate_person(bool with_optional = true) {
  static std::mt19937 rng(std::random_device{}());
  static std::uniform_int_distribution<uint64_t> age_dist(18, 80);
  static std::uniform_int_distribution<size_t> tag_count(0, 5);

  Person p;
  p.name_ = MAKE_UNIQUE(std::string, random_string(10));
  p.age_ = MAKE_UNIQUE(std::uint64_t, age_dist(rng));

  if (with_optional && rng() % 2 == 0) {
    p.email_ = MAKE_UNIQUE(std::string, random_string(15) + "@example.com");
  }

  if (with_optional && rng() % 3 == 0) {
    Address addr;
    addr.street_ = MAKE_UNIQUE(std::string, random_string(20));
    addr.city_ = MAKE_UNIQUE(std::string, random_string(10));
    addr.country_ = MAKE_UNIQUE(std::string, random_string(8));
    if (rng() % 2 == 0) {
      addr.postal_code_ = MAKE_UNIQUE(std::string, random_string(6));
    }
    p.address_ = MAKE_UNIQUE(Address, std::move(addr));
  }

  size_t num_tags = tag_count(rng);
  for (size_t i = 0; i < num_tags; ++i) {
    p.tags_.push_back(random_string(8));
  }

  return p;
}

Company generate_company(size_t num_employees) {
  Company c;
  c.name_ = MAKE_UNIQUE(std::string, random_string(15) + " Inc.");

  for (size_t i = 0; i < num_employees; ++i) {
    c.employees_.push_back(generate_person());
  }

  for (size_t i = 0; i < 5; ++i) {
    c.metadata_[random_string(8)] = random_string(12);
  }

  return c;
}

// =============================================================================
// Benchmark Test Cases
// =============================================================================

BOOST_AUTO_TEST_SUITE(serialization_benchmarks)

BOOST_AUTO_TEST_CASE(benchmark_simple_person_serialization) {
  Person person = generate_person(false);

  auto result = benchmark("Simple Person Serialization", [&]() {
    boost::json::value json = Marshal(person);
    boost::json::serialize(json); // Force materialization
  });

  result.print();
  BOOST_TEST_MESSAGE("Benchmark completed successfully");
}

BOOST_AUTO_TEST_CASE(benchmark_complex_person_serialization) {
  Person person = generate_person(true);

  auto result = benchmark("Complex Person Serialization", [&]() {
    boost::json::value json = Marshal(person);
    boost::json::serialize(json);
  });

  result.print();
  BOOST_TEST_MESSAGE("Benchmark completed successfully");
}

BOOST_AUTO_TEST_CASE(benchmark_small_company_serialization) {
  Company company = generate_company(10);

  auto result = benchmark(
      "Small Company (10 employees) Serialization",
      [&]() {
        MarshalToString(company);
      },
      500);

  result.print();
  BOOST_TEST_MESSAGE("Benchmark completed successfully");
}

BOOST_AUTO_TEST_CASE(benchmark_medium_company_serialization) {
  Company company = generate_company(100);

  auto result = benchmark(
      "Medium Company (100 employees) Serialization",
      [&]() {
        MarshalToString(company);
      },
      100);

  result.print();
  BOOST_TEST_MESSAGE("Benchmark completed successfully");
}

BOOST_AUTO_TEST_CASE(benchmark_large_company_serialization) {
  Company company = generate_company(1000);

  auto result = benchmark(
      "Large Company (1000 employees) Serialization",
      [&]() {
        MarshalToString(company);
      },
      10);

  result.print();
  BOOST_TEST_MESSAGE("Benchmark completed successfully");
}

BOOST_AUTO_TEST_CASE(benchmark_vector_serialization) {
  std::vector<Person> people;
  people.reserve(100);
  for (size_t i = 0; i < 100; ++i) {
    people.push_back(generate_person(true));
  }

  auto result = benchmark(
      "Vector<Person> (100) Serialization",
      [&]() {
        boost::json::value json = Marshal(people);
        boost::json::serialize(json);
      },
      200);

  result.print();
  BOOST_TEST_MESSAGE("Benchmark completed successfully");
}

BOOST_AUTO_TEST_CASE(benchmark_map_serialization) {
  std::map<std::string, Person> people_map;
  for (size_t i = 0; i < 100; ++i) {
    people_map[random_string(10)] = generate_person(true);
  }

  auto result = benchmark(
      "Map<string, Person> (100) Serialization",
      [&]() {
        boost::json::value json = Marshal(people_map);
        boost::json::serialize(json);
      },
      200);

  result.print();
  BOOST_TEST_MESSAGE("Benchmark completed successfully");
}

BOOST_AUTO_TEST_SUITE_END()

// =============================================================================
// Deserialization Benchmarks
// =============================================================================

BOOST_AUTO_TEST_SUITE(deserialization_benchmarks)

BOOST_AUTO_TEST_CASE(benchmark_simple_person_deserialization) {
  Person person = generate_person(false);
  std::string json_str = MarshalToString(person);

  auto result = benchmark("Simple Person Deserialization", [&]() {
    Person p;
    UnmarshalFromString(json_str, p);
  });

  result.print();
  BOOST_TEST_MESSAGE("Benchmark completed successfully");
}

BOOST_AUTO_TEST_CASE(benchmark_complex_person_deserialization) {
  Person person = generate_person(true);
  std::string json_str = MarshalToString(person);

  auto result = benchmark("Complex Person Deserialization", [&]() {
    Person p;
    UnmarshalFromString(json_str, p);
  });

  result.print();
  BOOST_TEST_MESSAGE("Benchmark completed successfully");
}

BOOST_AUTO_TEST_CASE(benchmark_small_company_deserialization) {
  Company company = generate_company(10);
  std::string json_str = MarshalToString(company);

  auto result = benchmark(
      "Small Company (10 employees) Deserialization",
      [&]() {
        Company c;
        UnmarshalFromString(json_str, c);
      },
      500);

  result.print();
  BOOST_TEST_MESSAGE("Benchmark completed successfully");
}

BOOST_AUTO_TEST_CASE(benchmark_medium_company_deserialization) {
  Company company = generate_company(100);
  std::string json_str = MarshalToString(company);

  auto result = benchmark(
      "Medium Company (100 employees) Deserialization",
      [&]() {
        Company c;
        UnmarshalFromString(json_str, c);
      },
      100);

  result.print();
  BOOST_TEST_MESSAGE("Benchmark completed successfully");
}

BOOST_AUTO_TEST_CASE(benchmark_large_company_deserialization) {
  Company company = generate_company(1000);
  std::string json_str = MarshalToString(company);

  auto result = benchmark(
      "Large Company (1000 employees) Deserialization",
      [&]() {
        Company c;
        UnmarshalFromString(json_str, c);
      },
      10);

  result.print();
  BOOST_TEST_MESSAGE("Benchmark completed successfully");
}

BOOST_AUTO_TEST_CASE(benchmark_vector_deserialization) {
  std::vector<Person> people;
  people.reserve(100);
  for (size_t i = 0; i < 100; ++i) {
    people.push_back(generate_person(true));
  }
  std::string json_str = MarshalToString(people);

  auto result = benchmark(
      "Vector<Person> (100) Deserialization",
      [&]() {
        std::vector<Person> p;
        UnmarshalFromString(json_str, p);
      },
      200);

  result.print();
  BOOST_TEST_MESSAGE("Benchmark completed successfully");
}

BOOST_AUTO_TEST_CASE(benchmark_map_deserialization) {
  std::map<std::string, Person> people_map;
  for (size_t i = 0; i < 100; ++i) {
    people_map[random_string(10)] = generate_person(true);
  }
  std::string json_str = MarshalToString(people_map);

  auto result = benchmark(
      "Map<string, Person> (100) Deserialization",
      [&]() {
        std::map<std::string, Person> m;
        UnmarshalFromString(json_str, m);
      },
      200);

  result.print();
  BOOST_TEST_MESSAGE("Benchmark completed successfully");
}

BOOST_AUTO_TEST_SUITE_END()

// =============================================================================
// Round-trip Benchmarks
// =============================================================================

BOOST_AUTO_TEST_SUITE(roundtrip_benchmarks)

BOOST_AUTO_TEST_CASE(benchmark_person_roundtrip) {
  Person person = generate_person(true);

  auto result = benchmark("Person Round-trip (Serialize + Deserialize)", [&]() {
    std::string json_str = MarshalToString(person);
    Person p;
    UnmarshalFromString(json_str, p);
  });

  result.print();
  BOOST_TEST_MESSAGE("Benchmark completed successfully");
}

BOOST_AUTO_TEST_CASE(benchmark_company_roundtrip) {
  Company company = generate_company(50);

  auto result = benchmark(
      "Company (50 employees) Round-trip",
      [&]() {
        std::string json_str = MarshalToString(company);
        Company c;
        UnmarshalFromString(json_str, c);
      },
      100);

  result.print();
  BOOST_TEST_MESSAGE("Benchmark completed successfully");
}

BOOST_AUTO_TEST_CASE(benchmark_vector_roundtrip) {
  std::vector<Person> people;
  people.reserve(50);
  for (size_t i = 0; i < 50; ++i) {
    people.push_back(generate_person(true));
  }

  auto result = benchmark(
      "Vector<Person> (50) Round-trip",
      [&]() {
        std::string json_str = MarshalToString(people);
        std::vector<Person> p;
        UnmarshalFromString(json_str, p);
      },
      200);

  result.print();
  BOOST_TEST_MESSAGE("Benchmark completed successfully");
}

BOOST_AUTO_TEST_SUITE_END()

// =============================================================================
// Memory Allocation Benchmarks
// =============================================================================

BOOST_AUTO_TEST_SUITE(memory_benchmarks)

BOOST_AUTO_TEST_CASE(benchmark_shared_ptr_vs_unique_ptr) {
  WithUnique u;
  u.value_ = MAKE_UNIQUE(std::string, random_string(50));
  auto unique_result = benchmark(
      "unique_ptr Serialization",
      [&]() {
        MarshalToString(u);
      },
      1000);

  WithShared s;
  s.value_ = MAKE_SHARED(std::string, random_string(50));
  auto shared_result = benchmark(
      "shared_ptr Serialization",
      [&]() {
        MarshalToString(s);
      },
      1000);

  unique_result.print();
  shared_result.print();

  std::cout << "\nComparison: shared_ptr is "
            << (shared_result.avg_us / unique_result.avg_us * 100.0 - 100.0) << "% "
            << (shared_result.avg_us > unique_result.avg_us ? "slower" : "faster")
            << " than unique_ptr\n";
}

BOOST_AUTO_TEST_CASE(benchmark_optional_vs_pointer) {
  WithOptional opt;
  opt.value_ = random_string(50);
  auto opt_result = benchmark(
      "optional Serialization",
      [&]() {
        MarshalToString(opt);
      },
      1000);

  WithPointer ptr;
  ptr.value_ = MAKE_UNIQUE(std::string, random_string(50));
  auto ptr_result = benchmark(
      "unique_ptr Serialization",
      [&]() {
        MarshalToString(ptr);
      },
      1000);

  opt_result.print();
  ptr_result.print();

  std::cout << "\nComparison: unique_ptr is "
            << (ptr_result.avg_us / opt_result.avg_us * 100.0 - 100.0) << "% "
            << (ptr_result.avg_us > opt_result.avg_us ? "slower" : "faster") << " than optional\n";
}

BOOST_AUTO_TEST_SUITE_END()
