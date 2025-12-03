#define BOOST_TEST_MODULE JSON Benchmark Tests
#include <boost/json.hpp>
#include <boost/test/unit_test.hpp>
#include <array>
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

// using namespace json;

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
  Vector<double> times;
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
  UniquePtr<String> street_;
  UniquePtr<String> city_;
  UniquePtr<String> country_;
  UniquePtr<String> postal_code_;

  constexpr const static auto properties =
      std::make_tuple(json::prop(&Address::street_, "street"),
                      json::prop(&Address::city_, "city"),
                      json::prop(&Address::country_, "country"),
                      json::prop(&Address::postal_code_, "postal_code", json::NULLABLE));
};

struct Person {
  UniquePtr<String> name_;
  UniquePtr<UInt64> age_;
  UniquePtr<String> email_;
  UniquePtr<Address> address_;
  Vector<String> tags_;

  constexpr const static auto properties =
      std::make_tuple(json::prop(&Person::name_, "name"),
                      json::prop(&Person::age_, "age"),
                      json::prop(&Person::email_, "email", json::NULLABLE),
                      json::prop(&Person::address_, "address", json::NULLABLE),
                      json::prop(&Person::tags_, "tags"));
};

struct Company {
  UniquePtr<String> name_;
  Vector<Person> employees_;
  Map<String> metadata_;

  constexpr const static auto properties =
      std::make_tuple(json::prop(&Company::name_, "name"),
                      json::prop(&Company::employees_, "employees"),
                      json::prop(&Company::metadata_, "metadata"));
};

// Memory benchmark test structures
struct WithUnique {
  UniquePtr<String> value_;
  constexpr const static auto properties =
      std::make_tuple(json::prop(&WithUnique::value_, "value"));
};

struct WithShared {
  SharedPtr<String> value_;
  constexpr const static auto properties =
      std::make_tuple(json::prop(&WithShared::value_, "value"));
};

struct WithOptional {
  Optional<String> value_;
  constexpr const static auto properties =
      std::make_tuple(json::prop(&WithOptional::value_, "value", json::NULLABLE));
};

struct WithPointer {
  UniquePtr<String> value_;
  constexpr const static auto properties =
      std::make_tuple(json::prop(&WithPointer::value_, "value", json::NULLABLE));
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
    boost::json::value json = json::Marshal(person);
    boost::json::serialize(json); // Force materialization
  });

  result.print();
  BOOST_TEST_MESSAGE("Benchmark completed successfully");
}

BOOST_AUTO_TEST_CASE(benchmark_complex_person_serialization) {
  Person person = generate_person(true);

  auto result = benchmark("Complex Person Serialization", [&]() {
    boost::json::value json = json::Marshal(person);
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
        (void)json::MarshalToString(company);
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
        (void)json::MarshalToString(company);
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
        (void)json::MarshalToString(company);
      },
      10);

  result.print();
  BOOST_TEST_MESSAGE("Benchmark completed successfully");
}

BOOST_AUTO_TEST_CASE(benchmark_vector_serialization) {
  Vector<Person> people;
  people.reserve(100);
  for (size_t i = 0; i < 100; ++i) {
    people.push_back(generate_person(true));
  }

  auto result = benchmark(
      "Vector<Person> (100) Serialization",
      [&]() {
        boost::json::value json = json::Marshal(people);
        boost::json::serialize(json);
      },
      200);

  result.print();
  BOOST_TEST_MESSAGE("Benchmark completed successfully");
}

BOOST_AUTO_TEST_CASE(benchmark_map_serialization) {
  Map<Person> people_map;
  for (size_t i = 0; i < 100; ++i) {
    people_map[random_string(10)] = generate_person(true);
  }

  auto result = benchmark(
      "Map<string, Person> (100) Serialization",
      [&]() {
        boost::json::value json = json::Marshal(people_map);
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
  std::string json_str = json::MarshalToString(person);

  auto result = benchmark("Simple Person Deserialization", [&]() {
    Person p;
    json::UnmarshalFromString(json_str, p);
  });

  result.print();
  BOOST_TEST_MESSAGE("Benchmark completed successfully");
}

BOOST_AUTO_TEST_CASE(benchmark_complex_person_deserialization) {
  Person person = generate_person(true);
  std::string json_str = json::MarshalToString(person);

  auto result = benchmark("Complex Person Deserialization", [&]() {
    Person p;
    json::UnmarshalFromString(json_str, p);
  });

  result.print();
  BOOST_TEST_MESSAGE("Benchmark completed successfully");
}

BOOST_AUTO_TEST_CASE(benchmark_small_company_deserialization) {
  Company company = generate_company(10);
  std::string json_str = json::MarshalToString(company);

  auto result = benchmark(
      "Small Company (10 employees) Deserialization",
      [&]() {
        Company c;
        json::UnmarshalFromString(json_str, c);
      },
      500);

  result.print();
  BOOST_TEST_MESSAGE("Benchmark completed successfully");
}

BOOST_AUTO_TEST_CASE(benchmark_medium_company_deserialization) {
  Company company = generate_company(100);
  std::string json_str = json::MarshalToString(company);

  auto result = benchmark(
      "Medium Company (100 employees) Deserialization",
      [&]() {
        Company c;
        json::UnmarshalFromString(json_str, c);
      },
      100);

  result.print();
  BOOST_TEST_MESSAGE("Benchmark completed successfully");
}

BOOST_AUTO_TEST_CASE(benchmark_large_company_deserialization) {
  Company company = generate_company(1000);
  std::string json_str = json::MarshalToString(company);

  auto result = benchmark(
      "Large Company (1000 employees) Deserialization",
      [&]() {
        Company c;
        json::UnmarshalFromString(json_str, c);
      },
      10);

  result.print();
  BOOST_TEST_MESSAGE("Benchmark completed successfully");
}

BOOST_AUTO_TEST_CASE(benchmark_vector_deserialization) {
  Vector<Person> people;
  people.reserve(100);
  for (size_t i = 0; i < 100; ++i) {
    people.push_back(generate_person(true));
  }
  std::string json_str = json::MarshalToString(people);

  auto result = benchmark(
      "Vector<Person> (100) Deserialization",
      [&]() {
        Vector<Person> p;
        json::UnmarshalFromString(json_str, p);
      },
      200);

  result.print();
  BOOST_TEST_MESSAGE("Benchmark completed successfully");
}

BOOST_AUTO_TEST_CASE(benchmark_map_deserialization) {
  Map<Person> people_map;
  for (size_t i = 0; i < 100; ++i) {
    people_map[random_string(10)] = generate_person(true);
  }
  std::string json_str = json::MarshalToString(people_map);

  auto result = benchmark(
      "Map<string, Person> (100) Deserialization",
      [&]() {
        Map<Person> m;
        json::UnmarshalFromString(json_str, m);
      },
      200);

  result.print();
  BOOST_TEST_MESSAGE("Benchmark completed successfully");
}

BOOST_AUTO_TEST_CASE(benchmark_array_serialization) {
  Array<Person, 10> people;
  for (size_t i = 0; i < 10; ++i) {
    people[i] = generate_person(true);
  }

  auto result = benchmark(
      "Array<Person, 10> Serialization",
      [&]() {
        boost::json::value json = json::Marshal(people);
        boost::json::serialize(json);
      },
      200);

  result.print();
  BOOST_TEST_MESSAGE("Benchmark completed successfully");
}

BOOST_AUTO_TEST_CASE(benchmark_array_deserialization) {
  Array<Person, 10> people;
  for (size_t i = 0; i < 10; ++i) {
    people[i] = generate_person(true);
  }
  std::string json_str = json::MarshalToString(people);

  auto result = benchmark(
      "Array<Person, 10> Deserialization",
      [&]() {
        Array<Person, 10> a;
        json::UnmarshalFromString(json_str, a);
      },
      200);

  result.print();
  BOOST_TEST_MESSAGE("Benchmark completed successfully");
}

BOOST_AUTO_TEST_CASE(benchmark_array_vs_vector_serialization) {
  // Compare array vs vector with same size
  Array<Person, 50> people_array;
  Vector<Person> people_vector;

  for (size_t i = 0; i < 50; ++i) {
    people_array[i] = generate_person(true);
    people_vector.push_back(generate_person(true));
  }

  auto array_result = benchmark(
      "Array<Person, 50> Serialization",
      [&]() {
        boost::json::value json = json::Marshal(people_array);
        boost::json::serialize(json);
      },
      100);

  auto vector_result = benchmark(
      "Vector<Person, 50> Serialization",
      [&]() {
        boost::json::value json = json::Marshal(people_vector);
        boost::json::serialize(json);
      },
      100);

  array_result.print();
  vector_result.print();

  std::cout << "\nPerformance comparison:\n";
  std::cout << "  Array is " << (vector_result.avg_us / array_result.avg_us) << "x "
            << (array_result.avg_us < vector_result.avg_us ? "faster" : "slower")
            << " than Vector\n";

  BOOST_TEST_MESSAGE("Benchmark comparison completed successfully");
}

BOOST_AUTO_TEST_CASE(benchmark_array_vs_vector_deserialization) {
  // Compare array vs vector with same size
  Array<Person, 50> people_array;
  Vector<Person> people_vector;

  for (size_t i = 0; i < 50; ++i) {
    people_array[i] = generate_person(true);
    people_vector.push_back(generate_person(true));
  }

  std::string array_json = json::MarshalToString(people_array);
  std::string vector_json = json::MarshalToString(people_vector);

  auto array_result = benchmark(
      "Array<Person, 50> Deserialization",
      [&]() {
        Array<Person, 50> a;
        json::UnmarshalFromString(array_json, a);
      },
      100);

  auto vector_result = benchmark(
      "Vector<Person, 50> Deserialization",
      [&]() {
        Vector<Person> v;
        json::UnmarshalFromString(vector_json, v);
      },
      100);

  array_result.print();
  vector_result.print();

  std::cout << "\nPerformance comparison:\n";
  std::cout << "  Array is " << (vector_result.avg_us / array_result.avg_us) << "x "
            << (array_result.avg_us < vector_result.avg_us ? "faster" : "slower")
            << " than Vector\n";

  BOOST_TEST_MESSAGE("Benchmark comparison completed successfully");
}

BOOST_AUTO_TEST_CASE(benchmark_array_vs_vector_roundtrip) {
  // Compare array vs vector roundtrip with same size
  Array<Person, 25> people_array;
  Vector<Person> people_vector;

  for (size_t i = 0; i < 25; ++i) {
    people_array[i] = generate_person(true);
    people_vector.push_back(generate_person(true));
  }

  auto array_result = benchmark(
      "Array<Person, 25> Round-trip",
      [&]() {
        std::string json_str = json::MarshalToString(people_array);
        Array<Person, 25> a;
        json::UnmarshalFromString(json_str, a);
      },
      100);

  auto vector_result = benchmark(
      "Vector<Person, 25> Round-trip",
      [&]() {
        std::string json_str = json::MarshalToString(people_vector);
        Vector<Person> v;
        json::UnmarshalFromString(json_str, v);
      },
      100);

  array_result.print();
  vector_result.print();

  std::cout << "\nPerformance comparison:\n";
  std::cout << "  Array is " << (vector_result.avg_us / array_result.avg_us) << "x "
            << (array_result.avg_us < vector_result.avg_us ? "faster" : "slower")
            << " than Vector\n";

  BOOST_TEST_MESSAGE("Benchmark comparison completed successfully");
}

BOOST_AUTO_TEST_CASE(benchmark_list_serialization) {
  List<Person> people;
  for (size_t i = 0; i < 100; ++i) {
    people.push_back(generate_person(true));
  }

  auto result = benchmark(
      "List<Person> (100) Serialization",
      [&]() {
        boost::json::value json = json::Marshal(people);
        boost::json::serialize(json);
      },
      200);

  result.print();
  BOOST_TEST_MESSAGE("Benchmark completed successfully");
}

BOOST_AUTO_TEST_CASE(benchmark_list_deserialization) {
  List<Person> people;
  for (size_t i = 0; i < 100; ++i) {
    people.push_back(generate_person(true));
  }
  std::string json_str = json::MarshalToString(people);

  auto result = benchmark(
      "List<Person> (100) Deserialization",
      [&]() {
        List<Person> l;
        json::UnmarshalFromString(json_str, l);
      },
      200);

  result.print();
  BOOST_TEST_MESSAGE("Benchmark completed successfully");
}

BOOST_AUTO_TEST_CASE(benchmark_list_vs_vector_serialization) {
  // Compare list vs vector with same size
  List<Person> people_list;
  Vector<Person> people_vector;

  for (size_t i = 0; i < 50; ++i) {
    people_list.push_back(generate_person(true));
    people_vector.push_back(generate_person(true));
  }

  auto list_result = benchmark(
      "List<Person, 50> Serialization",
      [&]() {
        boost::json::value json = json::Marshal(people_list);
        boost::json::serialize(json);
      },
      100);

  auto vector_result = benchmark(
      "Vector<Person, 50> Serialization",
      [&]() {
        boost::json::value json = json::Marshal(people_vector);
        boost::json::serialize(json);
      },
      100);

  list_result.print();
  vector_result.print();

  std::cout << "\nPerformance comparison:\n";
  std::cout << "  List is " << (vector_result.avg_us / list_result.avg_us) << "x "
            << (list_result.avg_us < vector_result.avg_us ? "faster" : "slower")
            << " than Vector\n";

  BOOST_TEST_MESSAGE("Benchmark comparison completed successfully");
}

BOOST_AUTO_TEST_CASE(benchmark_list_vs_vector_deserialization) {
  // Compare list vs vector with same size
  List<Person> people_list;
  Vector<Person> people_vector;

  for (size_t i = 0; i < 50; ++i) {
    people_list.push_back(generate_person(true));
    people_vector.push_back(generate_person(true));
  }

  std::string list_json = json::MarshalToString(people_list);
  std::string vector_json = json::MarshalToString(people_vector);

  auto list_result = benchmark(
      "List<Person, 50> Deserialization",
      [&]() {
        List<Person> l;
        json::UnmarshalFromString(list_json, l);
      },
      100);

  auto vector_result = benchmark(
      "Vector<Person, 50> Deserialization",
      [&]() {
        Vector<Person> v;
        json::UnmarshalFromString(vector_json, v);
      },
      100);

  list_result.print();
  vector_result.print();

  std::cout << "\nPerformance comparison:\n";
  std::cout << "  List is " << (vector_result.avg_us / list_result.avg_us) << "x "
            << (list_result.avg_us < vector_result.avg_us ? "faster" : "slower")
            << " than Vector\n";

  BOOST_TEST_MESSAGE("Benchmark comparison completed successfully");
}

BOOST_AUTO_TEST_CASE(benchmark_list_vs_vector_roundtrip) {
  // Compare list vs vector roundtrip with same size
  List<Person> people_list;
  Vector<Person> people_vector;

  for (size_t i = 0; i < 25; ++i) {
    people_list.push_back(generate_person(true));
    people_vector.push_back(generate_person(true));
  }

  auto list_result = benchmark(
      "List<Person, 25> Round-trip",
      [&]() {
        std::string json_str = json::MarshalToString(people_list);
        List<Person> l;
        json::UnmarshalFromString(json_str, l);
      },
      100);

  auto vector_result = benchmark(
      "Vector<Person, 25> Round-trip",
      [&]() {
        std::string json_str = json::MarshalToString(people_vector);
        Vector<Person> v;
        json::UnmarshalFromString(json_str, v);
      },
      100);

  list_result.print();
  vector_result.print();

  std::cout << "\nPerformance comparison:\n";
  std::cout << "  List is " << (vector_result.avg_us / list_result.avg_us) << "x "
            << (list_result.avg_us < vector_result.avg_us ? "faster" : "slower")
            << " than Vector\n";

  BOOST_TEST_MESSAGE("Benchmark comparison completed successfully");
}

BOOST_AUTO_TEST_CASE(benchmark_unordered_map_serialization) {
  Dict<Person> people_map;
  for (size_t i = 0; i < 100; ++i) {
    people_map[random_string(10)] = generate_person(true);
  }

  auto result = benchmark(
      "UnorderedMap<string, Person> (100) Serialization",
      [&]() {
        std::string json_str = json::MarshalToString(people_map);
      },
      200);

  result.print();
  BOOST_TEST_MESSAGE("Benchmark completed successfully");
}

BOOST_AUTO_TEST_CASE(benchmark_unordered_map_deserialization) {
  Dict<Person> people_map;
  for (size_t i = 0; i < 100; ++i) {
    people_map[random_string(10)] = generate_person(true);
  }
  std::string json_str = json::MarshalToString(people_map);

  auto result = benchmark(
      "UnorderedMap<string, Person> (100) Deserialization",
      [&]() {
        Dict<Person> m;
        json::UnmarshalFromString(json_str, m);
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
    std::string json_str = json::MarshalToString(person);
    Person p;
    json::UnmarshalFromString(json_str, p);
  });

  result.print();
  BOOST_TEST_MESSAGE("Benchmark completed successfully");
}

BOOST_AUTO_TEST_CASE(benchmark_company_roundtrip) {
  Company company = generate_company(50);

  auto result = benchmark(
      "Company (50 employees) Round-trip",
      [&]() {
        std::string json_str = json::MarshalToString(company);
        Company c;
        json::UnmarshalFromString(json_str, c);
      },
      100);

  result.print();
  BOOST_TEST_MESSAGE("Benchmark completed successfully");
}

BOOST_AUTO_TEST_CASE(benchmark_vector_roundtrip) {
  Vector<Person> people;
  people.reserve(50);
  for (size_t i = 0; i < 50; ++i) {
    people.push_back(generate_person(true));
  }

  auto result = benchmark(
      "Vector<Person> (50) Round-trip",
      [&]() {
        std::string json_str = json::MarshalToString(people);
        Vector<Person> p;
        json::UnmarshalFromString(json_str, p);
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
        (void)json::MarshalToString(u);
      },
      1000);

  WithShared s;
  s.value_ = MAKE_SHARED(std::string, random_string(50));
  auto shared_result = benchmark(
      "shared_ptr Serialization",
      [&]() {
        (void)json::MarshalToString(s);
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
        (void)json::MarshalToString(opt);
      },
      1000);

  WithPointer ptr;
  ptr.value_ = MAKE_UNIQUE(std::string, random_string(50));
  auto ptr_result = benchmark(
      "unique_ptr Serialization",
      [&]() {
        (void)json::MarshalToString(ptr);
      },
      1000);

  opt_result.print();
  ptr_result.print();

  std::cout << "\nComparison: unique_ptr is "
            << (ptr_result.avg_us / opt_result.avg_us * 100.0 - 100.0) << "% "
            << (ptr_result.avg_us > opt_result.avg_us ? "slower" : "faster") << " than optional\n";
}

BOOST_AUTO_TEST_SUITE_END()
