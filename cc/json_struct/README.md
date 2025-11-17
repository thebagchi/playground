# C++ JSON Struct Library

A modern, header-only C++ JSON serialization/deserialization library using Boost.JSON with compile-time reflection.

## Features

- **Compile-time Reflection**: Uses C++17 template metaprogramming for zero-runtime reflection overhead
- **Multiple Nullable Types**: Supports `std::unique_ptr`, `std::shared_ptr`, and `std::optional` for nullable fields
- **Type Safety**: Strong typing with compile-time validation
- **Boost.JSON Integration**: Built on top of Boost.JSON for efficient parsing and serialization
- **Container Support**: Works with `std::vector` and other STL containers
- **Error Handling**: Comprehensive error messages for serialization/deserialization failures

## Requirements

- **C++17** or later
- **CMake** 3.14+
- **Boost** 1.89.0 (automatically downloaded and built)

## Building

### Automatic Build (Recommended)

The project includes CMake scripts that automatically download and build Boost:

```bash
# Clone the repository
cd cc/json_struct

# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
make

# Run tests
./test_json

# Run demo
./main.bin
```

### Manual Build

If you have Boost installed system-wide:

```bash
# Set Boost paths in CMakeLists.txt
# Comment out the automatic download sections

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

```cpp
class PersonList {
public:
  std::unique_ptr<std::vector<Person>> persons_;

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

The library includes comprehensive unit tests covering:

- Basic serialization/deserialization
- Nullable types (`unique_ptr`, `shared_ptr`, `optional`)
- Container serialization
- Error handling
- Mixed type scenarios

Run tests with:
```bash
./test_json
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
├── test_json.cc        # Unit tests
├── CMakeLists.txt      # Build configuration
├── cmake/              # CMake utilities
├── build.sh            # Build script
├── thirdparty/         # Downloaded Boost (auto-generated)
└── libs/              # Installed Boost (auto-generated)
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
