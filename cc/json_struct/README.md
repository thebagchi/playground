# C++ JSON Struct Library

A modern, header-only C++ JSON serialization/deserialization library using Boost.JSON with compile-time reflection.

## Recent Updates (November 2025)

- ✅ **Comprehensive Type Aliases**: Added pre-defined type aliases for all vector/map combinations with nullable types
- ✅ **Standardized Test Suite**: 20 test cases following `TEST_PARSE_JSON_{TYPENAME}` naming convention
- ✅ **Clean Test Output**: Removed verbose output, keeping only JSON serialization results with standardized "==> TEST_NAME" prefixes
- ✅ **Consistent Error Messages**: Updated to uniform "Error in 'TEST_NAME': " format
- ✅ **Code Cleanup**: Removed all comments from test files for improved maintainability
- ✅ **Variable Naming**: Renamed `data_optional` to `data_with_optional` for clarity

## Features

- **Compile-time Reflection**: Uses C++17 template metaprogramming for zero-runtime reflection overhead
- **Multiple Nullable Types**: Supports `std::unique_ptr`, `std::shared_ptr`, and `std::optional` for nullable fields
- **Comprehensive Type Aliases**: Pre-defined type aliases for vectors and maps with all nullable combinations
- **Type Safety**: Strong typing with compile-time validation
- **Boost.JSON Integration**: Built on top of Boost.JSON for efficient parsing and serialization
- **Container Support**: Works with `std::vector`, `std::map` and other STL containers
- **Error Handling**: Comprehensive error messages for serialization/deserialization failures
- **Standardized Testing**: 20 comprehensive test cases with clean, consistent output formatting

## Requirements

- **C++17** or later
- **CMake** 3.14+
- **Boost** 1.89.0 (automatically downloaded and built)

## Building

### Automatic Build (Recommended)

The project includes CMake scripts that automatically download and build Boost:

```bash
# Navigate to the project directory
cd cc/json_struct

# Build the project (downloads and builds Boost automatically)
./build.sh

# Run the comprehensive test suite (20 test cases)
./build/test_json

# Run the demo program
./build/main.bin
```

### Manual Build

If you have Boost installed system-wide:

```bash
# Set Boost paths in CMakeLists.txt
# Comment out the automatic download sections

mkdir build && cd build
cmake ..
make
```

## Usage

### Basic Example

```cpp
#include "json.h"
#include <memory>

class Person {
public:
  std::string name_;
  std::unique_ptr<std::uint64_t> age_;
  std::shared_ptr<std::string> city_;
  std::optional<std::string> email_;

public:
  constexpr const static auto properties = std::make_tuple(
      prop(&Person::name_, "name"),     // required field
      prop(&Person::age_, "age"),       // nullable (unique_ptr)
      prop(&Person::city_, "city"),     // nullable (shared_ptr)
      prop(&Person::email_, "email")    // nullable (optional)
  );
};

int main() {
  // Create object
  Person p;
  p.name_ = "John Doe";
  p.age_ = std::make_unique<std::uint64_t>(30);
  p.city_ = std::make_shared<std::string>("New York");
  p.email_ = "john@example.com";

  // Serialize to JSON
  boost::json::value json_value = Marshal(p);
  std::cout << json_value << std::endl;
  // Output: 
  // {"name":"John Doe","age":30,"city":"New York","email":"john@example.com"}

  // Deserialize from JSON
  Person p2;
  Unmarshal(json_value, p2);

  return 0;
}
```

### Field Types

| Type | Nullable | JSON Representation |
|------|----------|-------------------|
| `std::string` | No | `"value"` |
| `std::unique_ptr<T>` | Yes | `value` or `null` |
| `std::shared_ptr<T>` | Yes | `value` or `null` |
| `std::optional<T>` | Yes | `value` or `null` |
| `std::vector<T>` | No | `[item1, item2, ...]` |

### Container Support

The library provides comprehensive support for STL containers with nullable types. Pre-defined type aliases are available for common combinations:

```cpp
// Vector types
using PersonVector = std::vector<Person>;
using PersonVectorUniquePtr = std::vector<std::unique_ptr<Person>>;
using PersonVectorSharedPtr = std::vector<std::shared_ptr<Person>>;
using PersonVectorOptional = std::vector<std::optional<Person>>;

// Map types
using PersonMap = std::map<std::string, Person>;
using PersonMapUniquePtr = std::map<std::string, std::unique_ptr<Person>>;
using PersonMapSharedPtr = std::map<std::string, std::shared_ptr<Person>>;
using PersonMapOptional = std::map<std::string, std::optional<Person>>;

// Example usage
class PersonList {
public:
  std::unique_ptr<PersonVector> persons_;

public:
  constexpr const static auto properties = std::make_tuple(
      prop(&PersonList::persons_, "persons")
  );
};
```

## API Reference

### Core Functions

- `Marshal(obj)` - Serialize object to `boost::json::value`
- `Unmarshal(json_value, obj)` - Deserialize `boost::json::value` to object

### Property Declaration

- `prop(&Class::member, "json_key")` - Declare a required property
- `prop(&Class::member, "json_key")` - Declare a nullable property (same syntax)

### Type Traits

- `is_optional_v<T>` - Check if type is nullable
- `has_properties_v<T>` - Check if type has reflection properties

## Testing

The library includes a comprehensive unit test suite with 20 test cases covering all serialization scenarios:

- Basic object serialization/deserialization
- All nullable types (`unique_ptr`, `shared_ptr`, `optional`)
- Container serialization (vectors and maps)
- Mixed nullable/required field combinations
- Error handling and validation

### Test Output Format

Tests use a clean, standardized output format:

```
==> TEST_PARSE_JSON_PERSON
{"name":"John Doe","age":30,"city":"New York","email":null}

==> TEST_PARSE_JSON_PERSON_VECTOR_OPTIONAL
[{"name":"Alice Cooper","age":28,"city":"Boston","email":"alice@example.com"},null,{"name":"David Wilson","age":42,"city":"Seattle","email":null}]
```

### Running Tests

```bash
# Build and run all tests
./build.sh
./build/test_json

# Expected output: "*** No errors detected" with 20 test cases passing
```

## Architecture

### Compile-time Reflection

The library uses C++17 template metaprogramming to generate serialization code at compile time:

1. **Property Tuples**: Classes define a `properties` member containing field metadata
2. **Type Traits**: Compile-time type checking for nullable vs required fields
3. **Template Specialization**: Different serialization logic for different types
4. **Zero Runtime Overhead**: All reflection happens at compile time

### Error Handling

- Throws `std::runtime_error` for serialization failures
- Detailed error messages indicating which field failed
- Null pointer checks for required fields

## Project Structure

```
json_struct/
├── json.h              # Main library header
├── main.cc             # Demo program
├── test_json.cc        # Unit tests (20 comprehensive test cases)
├── test_data.h         # Test data definitions
├── CMakeLists.txt      # Build configuration
├── cmake/              # CMake utilities
├── build.sh            # Build script
├── thirdparty/         # Downloaded Boost (auto-generated)
└── libs/               # Installed Boost (auto-generated)
```

## Contributing

1. Fork the repository
2. Create a feature branch
3. Add tests for new functionality
4. Ensure all tests pass
5. Submit a pull request

## License

This project is open source. See LICENSE file for details.

## Dependencies

- **Boost.JSON**: JSON parsing/serialization
- **Boost.Test**: Unit testing framework
- **CMake**: Build system

The build system automatically downloads and builds Boost, so no manual dependency installation is required.
