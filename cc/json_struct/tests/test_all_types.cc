#define BOOST_TEST_MODULE All Types Test
#include <boost/json.hpp>
#include <boost/test/unit_test.hpp>
#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <tuple>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <array>

#include "json.h"

// Enum for testing
enum class TestEnum {
  VALUE1,
  VALUE2,
  VALUE3
};

template <> struct json::ENUM<TestEnum> {
  static constexpr json::EnumEncoding encoding = json::EnumEncoding::STRING;
  static constexpr bool case_insensitive = false;
  static constexpr std::array<std::string_view, 3> names{ "value1", "value2", "value3" };
};

// Comprehensive struct with all supported types
class AllTypesStruct {
public:
  // Basic types
  UniquePtr<String> string_field;
  UniquePtr<Int64> int64_field;
  UniquePtr<UInt64> uint64_field;
  UniquePtr<Double> double_field;
  UniquePtr<Bool> bool_field;

  // Containers
  UniquePtr<Vector<String>> vector_string;
  UniquePtr<Set<Int64>> set_int64;
  UniquePtr<Map<String>> map_string;
  UniquePtr<Dict<String>> dict_string;
  UniquePtr<Array<String, 3>> array_string;

  // Smart pointers
  UniquePtr<UniquePtr<String>> unique_ptr_string;
  UniquePtr<SharedPtr<String>> shared_ptr_string;
  UniquePtr<Optional<String>> optional_string;

  // Binary data
  UniquePtr<ByteVector> byte_vector;
  UniquePtr<ByteBuffer> byte_buffer;
  UniquePtr<ByteArray<4>> byte_array;

  // Value pass-through
  UniquePtr<Value> json_value;

  // Enum
  UniquePtr<TestEnum> enum_field;
public:
  constexpr const static auto properties = std::make_tuple(
   // Basic types
   json::prop(&AllTypesStruct::string_field, "string_field"),
   json::prop(&AllTypesStruct::int64_field, "int64_field"),
   json::prop(&AllTypesStruct::uint64_field, "uint64_field"),
   json::prop(&AllTypesStruct::double_field, "double_field"),
   json::prop(&AllTypesStruct::bool_field, "bool_field"),

   // Containers
   json::prop(&AllTypesStruct::vector_string, "vector_string"),
   json::prop(&AllTypesStruct::set_int64, "set_int64"),
   json::prop(&AllTypesStruct::map_string, "map_string"),
   json::prop(&AllTypesStruct::dict_string, "dict_string"),
   json::prop(&AllTypesStruct::array_string, "array_string"),

   // Smart pointers
   json::prop(&AllTypesStruct::unique_ptr_string, "unique_ptr_string"),
   json::prop(&AllTypesStruct::shared_ptr_string, "shared_ptr_string"),
   json::prop(&AllTypesStruct::optional_string, "optional_string"),

   // Binary data
   json::prop(&AllTypesStruct::byte_vector, "byte_vector"),
   json::prop(&AllTypesStruct::byte_buffer, "byte_buffer"),
   json::prop(&AllTypesStruct::byte_array, "byte_array"), // Value pass-through
   json::prop(&AllTypesStruct::json_value, "json_value"),

   // Enum
   json::prop(&AllTypesStruct::enum_field, "enum_field"));
};

template <> struct json::STRUCT<AllTypesStruct> {
  static constexpr auto properties = AllTypesStruct::properties;
};

// Helper function to create test data
AllTypesStruct createTestData() {
  AllTypesStruct obj;

  // Basic types
  obj.string_field = MAKE_UNIQUE(String, "Hello World");
  obj.int64_field = MAKE_UNIQUE(Int64, -12345);
  obj.uint64_field = MAKE_UNIQUE(UInt64, 67890);
  obj.double_field = MAKE_UNIQUE(Double, 3.14159);
  obj.bool_field = MAKE_UNIQUE(Bool, true);

  // Containers
  obj.vector_string = MAKE_UNIQUE(Vector<String>);
  obj.vector_string->push_back("item1");
  obj.vector_string->push_back("item2");

  obj.set_int64 = MAKE_UNIQUE(Set<Int64>);
  obj.set_int64->insert(100);
  obj.set_int64->insert(200);
  obj.set_int64->insert(300);

  obj.map_string = MAKE_UNIQUE(Map<String>);
  (*obj.map_string)["key1"] = "value1";
  (*obj.map_string)["key2"] = "value2";

  obj.dict_string = MAKE_UNIQUE(Dict<String>);
  (*obj.dict_string)["dict_key1"] = "dict_value1";
  (*obj.dict_string)["dict_key2"] = "dict_value2";

  obj.array_string = std::make_unique<Array<String, 3>>();
  (*obj.array_string)[0] = "array0";
  (*obj.array_string)[1] = "array1";
  (*obj.array_string)[2] = "array2";

  // Smart pointers
  obj.unique_ptr_string = MAKE_UNIQUE(UniquePtr<String>);
  *obj.unique_ptr_string = MAKE_UNIQUE(String, "unique_ptr_value");

  obj.shared_ptr_string = MAKE_UNIQUE(SharedPtr<String>);
  *obj.shared_ptr_string = MAKE_SHARED(String, "shared_ptr_value");

  obj.optional_string = MAKE_UNIQUE(Optional<String>);
  *obj.optional_string = "optional_value";

  // Binary data
  obj.byte_vector = MAKE_UNIQUE(ByteVector);
  obj.byte_vector->push_back(std::byte{ 0x01 });
  obj.byte_vector->push_back(std::byte{ 0x02 });
  obj.byte_vector->push_back(std::byte{ 0x03 });

  obj.byte_buffer = MAKE_UNIQUE(ByteBuffer);
  obj.byte_buffer->push_back(0x04);
  obj.byte_buffer->push_back(0x05);
  obj.byte_buffer->push_back(0x06);

  obj.byte_array = std::make_unique<ByteArray<4>>();
  (*obj.byte_array)[0] = std::byte{ 0x07 };
  (*obj.byte_array)[1] = std::byte{ 0x08 };
  (*obj.byte_array)[2] = std::byte{ 0x09 };
  (*obj.byte_array)[3] = std::byte{ 0x0A };

  // Value pass-through
  obj.json_value = MAKE_UNIQUE(Value);
  *obj.json_value = boost::json::object{ { "nested", "object" }, { "number", 42 } };

  // Enum
  obj.enum_field = MAKE_UNIQUE(TestEnum);
  *obj.enum_field = TestEnum::VALUE2;

  return obj;
}

// Helper function to compare two AllTypesStruct objects
bool compareAllTypesStruct(const AllTypesStruct& a, const AllTypesStruct& b) {
  // Basic types
  if ((a.string_field && b.string_field && *a.string_field != *b.string_field) ||
      (!a.string_field != !b.string_field)) {
    return false;
  }
  if ((a.int64_field && b.int64_field && *a.int64_field != *b.int64_field) ||
      (!a.int64_field != !b.int64_field)) {
    return false;
  }
  if ((a.uint64_field && b.uint64_field && *a.uint64_field != *b.uint64_field) ||
      (!a.uint64_field != !b.uint64_field)) {
    return false;
  }
  if ((a.double_field && b.double_field && *a.double_field != *b.double_field) ||
      (!a.double_field != !b.double_field)) {
    return false;
  }
  if ((a.bool_field && b.bool_field && *a.bool_field != *b.bool_field) ||
      (!a.bool_field != !b.bool_field)) {
    return false;
  }

  // Containers
  if ((a.vector_string && b.vector_string && *a.vector_string != *b.vector_string) ||
      (!a.vector_string != !b.vector_string)) {
    return false;
  }
  if ((a.set_int64 && b.set_int64 && *a.set_int64 != *b.set_int64) ||
      (!a.set_int64 != !b.set_int64)) {
    return false;
  }
  if ((a.map_string && b.map_string && *a.map_string != *b.map_string) ||
      (!a.map_string != !b.map_string)) {
    return false;
  }
  if ((a.dict_string && b.dict_string && *a.dict_string != *b.dict_string) ||
      (!a.dict_string != !b.dict_string)) {
    return false;
  }
  if ((a.array_string && b.array_string && *a.array_string != *b.array_string) ||
      (!a.array_string != !b.array_string)) {
    return false;
  }

  // Smart pointers
  if ((a.unique_ptr_string && b.unique_ptr_string &&
       ((!*a.unique_ptr_string && !*b.unique_ptr_string) ||
        (*a.unique_ptr_string && *b.unique_ptr_string &&
         **a.unique_ptr_string != **b.unique_ptr_string))) ||
      (!a.unique_ptr_string != !b.unique_ptr_string)) {
    return false;
  }
  if ((a.shared_ptr_string && b.shared_ptr_string &&
       ((!*a.shared_ptr_string && !*b.shared_ptr_string) ||
        (*a.shared_ptr_string && *b.shared_ptr_string &&
         **a.shared_ptr_string != **b.shared_ptr_string))) ||
      (!a.shared_ptr_string != !b.shared_ptr_string)) {
    return false;
  }
  if ((a.optional_string && b.optional_string && *a.optional_string != *b.optional_string) ||
      (!a.optional_string != !b.optional_string)) {
    return false;
  }

  // Binary data - compare sizes and contents manually
  if ((a.byte_vector && b.byte_vector &&
       (a.byte_vector->size() != b.byte_vector->size() ||
        !std::equal(a.byte_vector->begin(), a.byte_vector->end(), b.byte_vector->begin()))) ||
      (!a.byte_vector != !b.byte_vector)) {
    return false;
  }
  if ((a.byte_buffer && b.byte_buffer &&
       (a.byte_buffer->size() != b.byte_buffer->size() ||
        !std::equal(a.byte_buffer->begin(), a.byte_buffer->end(), b.byte_buffer->begin()))) ||
      (!a.byte_buffer != !b.byte_buffer)) {
    return false;
  }
  if ((a.byte_array && b.byte_array &&
       !std::equal(a.byte_array->begin(), a.byte_array->end(), b.byte_array->begin())) ||
      (!a.byte_array != !b.byte_array)) {
    return false;
  }

  // Value pass-through
  if ((a.json_value && b.json_value && *a.json_value != *b.json_value) ||
      (!a.json_value != !b.json_value)) {
    return false;
  }

  // Enum
  if ((a.enum_field && b.enum_field && *a.enum_field != *b.enum_field) ||
      (!a.enum_field != !b.enum_field)) {
    return false;
  }

  return true;
}

BOOST_AUTO_TEST_CASE(TEST_ALL_TYPES_ENCODE_DECODE) {
  try {
    // Create original object with test data
    AllTypesStruct original = createTestData();

    // Marshal to JSON
    boost::json::value json_value = json::Marshal(original);
    std::cout << "==> TEST_ALL_TYPES_ENCODE_DECODE" << std::endl;
    std::cout << json_value << std::endl;
    std::cout << std::endl;

    // Unmarshal back to new object
    AllTypesStruct decoded;

    // Try to unmarshal with better error reporting
    try {
      json::Unmarshal(json_value, decoded);
    } catch (const std::exception& inner_e) {
      std::cout << "Unmarshal failed with: " << inner_e.what() << std::endl;
      std::cout << "JSON was: " << boost::json::serialize(json_value) << std::endl;
      throw;
    }

    // Compare original and decoded
    BOOST_CHECK(compareAllTypesStruct(original, decoded));

  } catch (const std::exception& e) {
    std::cout << "Error in 'TEST_ALL_TYPES_ENCODE_DECODE': " << e.what() << std::endl;
    BOOST_FAIL("Exception thrown: " + std::string(e.what()));
  }
}

BOOST_AUTO_TEST_CASE(TEST_RAW_C_ARRAY_ENCODE_DECODE) {
  try {
    std::cout << "\n==> TEST_RAW_C_ARRAY_ENCODE_DECODE" << std::endl;

    // Test Int64 array
    {
      Int64 int64_array[5] = { -100, -1, 0, 1, 100 };
      boost::json::value json_value = json::Marshal(int64_array);
      std::cout << "Int64[5]: " << json_value << std::endl;

      Int64 decoded_int64[5];
      json::Unmarshal(json_value, decoded_int64);

      for (size_t i = 0; i < 5; ++i) {
        BOOST_CHECK_EQUAL(int64_array[i], decoded_int64[i]);
      }
    }

    // Test UInt64 array
    {
      UInt64 uint64_array[3] = { 0, 42, 18446744073709551615ULL };
      boost::json::value json_value = json::Marshal(uint64_array);
      std::cout << "UInt64[3]: " << json_value << std::endl;

      UInt64 decoded_uint64[3];
      json::Unmarshal(json_value, decoded_uint64);

      for (size_t i = 0; i < 3; ++i) {
        BOOST_CHECK_EQUAL(uint64_array[i], decoded_uint64[i]);
      }
    }

    // Test Double array
    {
      Double double_array[4] = { -1.5, 0.0, 3.14159, 2.71828 };
      boost::json::value json_value = json::Marshal(double_array);
      std::cout << "Double[4]: " << json_value << std::endl;

      Double decoded_double[4];
      json::Unmarshal(json_value, decoded_double);

      for (size_t i = 0; i < 4; ++i) {
        BOOST_CHECK_CLOSE(double_array[i], decoded_double[i], 0.0001);
      }
    }

    // Test Bool array
    {
      Bool bool_array[2] = { true, false };
      boost::json::value json_value = json::Marshal(bool_array);
      std::cout << "Bool[2]: " << json_value << std::endl;

      Bool decoded_bool[2];
      json::Unmarshal(json_value, decoded_bool);

      for (size_t i = 0; i < 2; ++i) {
        BOOST_CHECK_EQUAL(bool_array[i], decoded_bool[i]);
      }
    }

    std::cout << "All Raw C array tests passed!" << std::endl;

  } catch (const std::exception& e) {
    std::cout << "Error in 'TEST_RAW_C_ARRAY_ENCODE_DECODE': " << e.what() << std::endl;
    BOOST_FAIL("Exception thrown: " + std::string(e.what()));
  }
}

BOOST_AUTO_TEST_CASE(TEST_BINARY_C_ARRAY_BASE64_ENCODE_DECODE) {
  try {
    std::cout << "\n==> TEST_BINARY_C_ARRAY_BASE64_ENCODE_DECODE" << std::endl;

    // Test UChar array (unsigned char C-style array)
    {
      UChar uchar_array[5] = { 0x01, 0x02, 0x03, 0x04, 0x05 };
      boost::json::value json_value = json::Marshal(uchar_array);
      std::cout << "UChar[5]: " << json_value << std::endl;

      // Verify it's a base64 string
      BOOST_CHECK(json_value.is_string());

      UChar decoded_uchar[5];
      json::Unmarshal(json_value, decoded_uchar);

      for (size_t i = 0; i < 5; ++i) {
        BOOST_CHECK_EQUAL(uchar_array[i], decoded_uchar[i]);
      }
    }

    // Test Byte array (std::byte C-style array)
    {
      Byte byte_array[4] = {
        std::byte{ 0xAA }, std::byte{ 0xBB }, std::byte{ 0xCC }, std::byte{ 0xDD }
      };
      boost::json::value json_value = json::Marshal(byte_array);
      std::cout << "Byte[4]: " << json_value << std::endl;

      // Verify it's a base64 string
      BOOST_CHECK(json_value.is_string());

      Byte decoded_byte[4];
      json::Unmarshal(json_value, decoded_byte);

      for (size_t i = 0; i < 4; ++i) {
        BOOST_CHECK(byte_array[i] == decoded_byte[i]);
      }
    }

    // Test larger UChar array
    {
      UChar large_array[16];
      for (size_t i = 0; i < 16; ++i) {
        large_array[i] = static_cast<UChar>(i * 16);
      }

      boost::json::value json_value = json::Marshal(large_array);
      std::cout << "UChar[16]: " << json_value << std::endl;

      UChar decoded_large[16];
      json::Unmarshal(json_value, decoded_large);

      for (size_t i = 0; i < 16; ++i) {
        BOOST_CHECK_EQUAL(large_array[i], decoded_large[i]);
      }
    }

    std::cout << "All Binary C array base64 tests passed!" << std::endl;

  } catch (const std::exception& e) {
    std::cout << "Error in 'TEST_BINARY_C_ARRAY_BASE64_ENCODE_DECODE': " << e.what() << std::endl;
    BOOST_FAIL("Exception thrown: " + std::string(e.what()));
  }
}

BOOST_AUTO_TEST_CASE(TEST_SET_ENCODE_DECODE) {
  try {
    std::cout << "\n==> TEST_SET_ENCODE_DECODE" << std::endl;

    // Test Set<Int64>
    {
      Set<Int64> int_set = { 42, 10, 99, -5, 0 };
      boost::json::value json_value = json::Marshal(int_set);
      std::cout << "Set<Int64>: " << json_value << std::endl;

      // Verify it's a JSON array
      BOOST_CHECK(json_value.is_array());

      Set<Int64> decoded_set;
      json::Unmarshal(json_value, decoded_set);

      // Sets maintain sorted order and uniqueness
      BOOST_CHECK_EQUAL(int_set.size(), decoded_set.size());
      BOOST_CHECK(int_set == decoded_set);
    }

    // Test Set<String>
    {
      Set<String> string_set = { "zebra", "alpha", "beta", "gamma" };
      boost::json::value json_value = json::Marshal(string_set);
      std::cout << "Set<String>: " << json_value << std::endl;

      BOOST_CHECK(json_value.is_array());

      Set<String> decoded_set;
      json::Unmarshal(json_value, decoded_set);

      BOOST_CHECK_EQUAL(string_set.size(), decoded_set.size());
      BOOST_CHECK(string_set == decoded_set);
    }

    // Test Set with duplicates (should be deduplicated)
    {
      boost::json::array arr = { 1, 2, 2, 3, 3, 3, 4 };
      boost::json::value json_value = arr;

      Set<Int64> decoded_set;
      json::Unmarshal(json_value, decoded_set);

      // Set should contain only unique values
      BOOST_CHECK_EQUAL(decoded_set.size(), 4);
      BOOST_CHECK(decoded_set.count(1) == 1);
      BOOST_CHECK(decoded_set.count(2) == 1);
      BOOST_CHECK(decoded_set.count(3) == 1);
      BOOST_CHECK(decoded_set.count(4) == 1);
    }

    std::cout << "All Set tests passed!" << std::endl;

  } catch (const std::exception& e) {
    std::cout << "Error in 'TEST_SET_ENCODE_DECODE': " << e.what() << std::endl;
    BOOST_FAIL("Exception thrown: " + std::string(e.what()));
  }
}