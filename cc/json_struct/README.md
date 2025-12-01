# C++ JSON Struct Library

A high-performance, header-only C++ JSON serialization library using Boost.JSON with compile-time reflection.

## Features

- **Zero-Copy Architecture**: Output parameter design eliminates unnecessary copies
- **Compile-time Reflection**: C++17 metaprogramming with zero runtime overhead
- **High Performance**: ~5 μs for simple objects, 367k ops/sec deserialization
- **Smart Pointers**: Full support for `std::unique_ptr`, `std::shared_ptr`, `std::optional`
- **STL Containers**: Native support for `std::vector`, `std::map`, `std::unordered_map`
- **Binary Data**: Automatic Base64 encoding/decoding for `std::vector<std::uint8_t>`
- **Custom Allocators**: Optional `boost::json::static_resource` for stack allocation
- **Type Safety**: Strong compile-time type checking and validation

## Performance Benchmarks

| Operation                         | Throughput   | Latency |
|-----------------------------------|--------------|---------|
| Simple object serialization       | 206k ops/sec | 4.8 μs  |
| Simple object deserialization     | 367k ops/sec | 2.7 μs  |
| Complex object serialization      | 117k ops/sec | 8.6 μs  |
| Vector (100 items) serialization  | 2.7k ops/sec | 371 μs  |
| Large struct (1000 items)         | 162 ops/sec  | 6.2 ms  |

*Benchmarks run on standard hardware. Custom allocators provide additional 10-20% performance improvement.*

## Requirements

- **C++17** or later
- **CMake** 3.14+
- **Boost** 1.89.0 (automatically downloaded)

## Quick Start

```bash
# Build (auto-downloads Boost)
./build.sh

# Run tests
./build/tests/test_json

# Run benchmarks
./build/tests/test_benchmark
```

## Usage

### Basic Example

```cpp
#include "json.h"

struct Person {
  std::string name;
  std::unique_ptr<uint64_t> age;
  std::optional<std::string> email;

  constexpr static auto properties = std::make_tuple(
    prop(&Person::name, "name"),
    prop(&Person::age, "age"),
    prop(&Person::email, "email")
  );
};

int main() {
  // Serialize
  Person p{"John Doe", std::make_unique<uint64_t>(30), "john@example.com"};
  std::string json = MarshalToString(p);
  // {"name":"John Doe","age":30,"email":"john@example.com"}

  // Deserialize
  Person p2;
  UnmarshalFromString(json, p2);

  return 0;
}
```

### Binary Data (Base64)

```cpp
#include "json.h"

struct FileData {
  std::string filename;
  std::vector<std::uint8_t> content;  // Automatically encoded/decoded as Base64
  
  constexpr static auto properties = std::make_tuple(
    prop(&FileData::filename, "filename"),
    prop(&FileData::content, "content")
  );
};

int main() {
  // Binary data automatically encoded to Base64
  std::vector<std::uint8_t> binary_data = {0x00, 0x01, 0x02, 0xFF, 0xFE, 0xFD};
  FileData file{"test.bin", binary_data};
  
  std::string json = MarshalToString(file);
  // {"filename":"test.bin","content":"AAEC/+79"}
  
  // Binary data automatically decoded from Base64
  FileData file2;
  UnmarshalFromString(json, file2);
  // file2.content == {0x00, 0x01, 0x02, 0xFF, 0xFE, 0xFD}
  
  return 0;
}
```

### With Custom Allocator

```cpp
#include "json.h"
#include <boost/json/static_resource.hpp>

int main() {
  // Stack-allocated buffer (no heap allocations)
  unsigned char buffer[4096];
  boost::json::static_resource sr(buffer, sizeof(buffer));
  
  Person p{"Jane Doe", std::make_unique<uint64_t>(25), std::nullopt};
  
  // Use custom allocator for 10-20% performance boost
  boost::json::value json = Marshal(p, &sr);
  std::string json_str = MarshalToString(p, &sr);

  return 0;
}
```

### Supported Types

| Type                               | Nullable |
|------------------------------------|----------|
| std::string                        | No       |
| std::int64_t                       | No       |
| std::uint64_t                      | No       |
| double                             | No       |
| bool                               | No       |
| boost::json::value                 | No       |
| std::unique_ptr<T>                 | Yes      |
| std::shared_ptr<T>                 | Yes      |
| std::optional<T>                   | Yes      |
| std::vector<T>                     | No       |
| std::map<std::string, T>           | No       |
| std::unordered_map<std::string, T> | No       |
| std::vector<std::uint8_t>          | No       |

## API Reference

### Functions

```cpp
// Serialization
[[nodiscard]] boost::json::value Marshal(const T& obj, boost::json::storage_ptr sp = {});
[[nodiscard]] std::string MarshalToString(const T& obj, boost::json::storage_ptr sp = {});

// Deserialization
void Unmarshal(const boost::json::value& json, T& obj);
void UnmarshalFromString(const std::string& json_str, T& obj, boost::json::storage_ptr sp = {});

// Property declaration
prop(&Class::member, "json_key")  // Define struct member mapping
```

## Design

### Key Optimizations

1. **Zero-Copy**: Output parameters eliminate return value copies
2. **Compile-time Reflection**: All type information resolved at compile time
3. **Branch Prediction**: `[[likely]]`/`[[unlikely]]` hints for hot paths
4. **Memory Pre-allocation**: Container `reserve()` calls avoid reallocations
5. **String Views**: `boost::json::string_view` avoids temporary strings
6. **Emplace Operations**: Direct construction in containers (no copies)
7. **Custom Allocators**: Optional stack-based allocation for critical paths
8. **Base64 Zero-Copy**: Direct encoding into `boost::json::string` (no temp strings)

### Architecture

```
Write<T>  → Serialization with output parameter (const T*, boost::json::value*)
Read<T>   → Deserialization with output parameter (const value&, T*)
Marshal   → Public API wrapper for serialization
Unmarshal → Public API wrapper for deserialization
```

## Testing

37 comprehensive test cases covering all type combinations. Run with:

```bash
./build/tests/test_json        # Unit tests
./build/tests/test_benchmark   # Performance benchmarks
```

## License

MIT License. See LICENSE file for details.

---

**Dependencies**: Boost.JSON 1.89.0 (auto-downloaded during build)
