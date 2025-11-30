#include <chrono>
#include <iostream>
#include <thread>

#include "tsc.hpp"
#include "value.hpp"

// Utility function to convert bool to string
const char* bool_to_string(bool value) {
  return value ? "true" : "false";
}

void test_tsc() {
  uint64_t tsc_value = read_tsc();
  std::cout << "TSC Value: " << tsc_value << std::endl;
}

void test_tsc_difference() {
  uint64_t stsc = read_tsc();
  std::this_thread::sleep_for(std::chrono::seconds(1));
  uint64_t etsc = read_tsc();
  uint64_t difference = etsc - stsc;
  std::cout << "TSC difference over 1 second: " << difference << " cycles" << std::endl;
}

void test_value_bool() {
  // Test Bool constructor
  json::Value bool_value_1(true);
  json::Value bool_value_2(false);

  // Test type() function
  std::cout << "bool_value_1 type: " << bool_value_1.type_name() << std::endl;
  std::cout << "bool_value_2 type: " << bool_value_2.type_name() << std::endl;
}

void test_value_null() {
  // Test Null constructor
  json::Value null_value(nullptr);
  std::cout << "null_value type: " << null_value.type_name() << std::endl;
}

void test_value_string() {
  // Test String constructor with std::string
  json::Value string_value_1(std::string("Hello, World!"));
  std::cout << "string_value_1 type: " << string_value_1.type_name() << std::endl;

  // Test String constructor with const char*
  json::Value string_value_2("JSON Library");
  std::cout << "string_value_2 type: " << string_value_2.type_name() << std::endl;

  // Test String constructor with StringView
  json::Value string_value_3(std::string_view("StringView Test"));
  std::cout << "string_value_3 type: " << string_value_3.type_name() << std::endl;
}

void test_value_list() {
  // Test List constructor using MakePtr
  {
    json::List data = {
      json::MakePtr(42),      // Number
      json::MakePtr("hello"), // String
      json::MakePtr(true),    // Bool
      json::MakePtr(nullptr)  // Null
    };

    json::Value value(data);
    std::cout << "value type: " << value.type_name() << std::endl;
    std::cout << "list size: " << data.size() << std::endl;
  }

  // Test List move constructor
  {
    json::List data = {
      json::MakePtr(99),      // Number
      json::MakePtr("world"), // String
      json::MakePtr(false),   // Bool
      json::MakePtr(nullptr)  // Null
    };

    json::Value value(std::move(data));
    std::cout << "move value type: " << value.type_name() << std::endl;
    std::cout << "moved list size: " << data.size() << std::endl;
  }

  // Test inline List initialization
  {
    json::Value value(json::List{
        json::MakePtr(123),      // Number
        json::MakePtr("inline"), // String
        json::MakePtr(true)      // Bool
    });
    std::cout << "inline value type: " << value.type_name() << std::endl;
  }

  // Test as_list and for_each methods
  {
    json::List test_data = { json::MakePtr(100), json::MakePtr("test"), json::MakePtr(false) };

    json::Value list_value(test_data);

    // Test as_list method
    const auto& list_ref = list_value.as_list();
    std::cout << "as_list size: " << list_ref.size() << std::endl;

    // Test for_each method
    std::cout << "for_each iteration:" << std::endl;
    list_value.for_each([](const json::ValuePtr& item) {
      std::cout << "  " << item->DebugString() << std::endl;
    });
  }
}

void test_value_dict() {
  // Test Dict constructor using MakePtr
  {
    json::Dict data = { { "name", json::MakePtr("John") },
                        { "age", json::MakePtr(30) },
                        { "active", json::MakePtr(true) } };

    json::Value value(data);
    std::cout << "dict value type: " << value.type_name() << std::endl;
    std::cout << "dict size: " << data.size() << std::endl;
  }

  // Test inline Dict initialization
  {
    json::Value value(json::Dict{ { "key1", json::MakePtr(123) },
                                  { "key2", json::MakePtr("value") },
                                  { "key3", json::MakePtr(false) } });
    std::cout << "inline dict value type: " << value.type_name() << std::endl;
  }

  // Test as_dict and for_each methods
  {
    json::Dict test_data = { { "number", json::MakePtr(42.5) },
                             { "text", json::MakePtr("hello") },
                             { "flag", json::MakePtr(true) } };

    json::Value dict_value(test_data);

    // Test as_dict method
    const auto& dict_ref = dict_value.as_dict();
    std::cout << "as_dict size: " << dict_ref.size() << std::endl;

    // Test for_each method
    std::cout << "for_each iteration:" << std::endl;
    dict_value.for_each([](const std::string& key, const json::ValuePtr& value) {
      std::cout << "  \"" << key << "\": " << value->DebugString() << std::endl;
    });
  }
}

void test_make_ptr() {
  // Test MakePtr helper function
  auto ptr1 = json::MakePtr(42);      // Number
  auto ptr2 = json::MakePtr(true);    // Bool
  auto ptr3 = json::MakePtr(nullptr); // Null
  auto ptr4 = json::MakePtr("Hello"); // String

  std::cout << "MakePtr(42) type:        " << ptr1->type_name() << std::endl;
  std::cout << "MakePtr(true) type:      " << ptr2->type_name() << std::endl;
  std::cout << "MakePtr(nullptr) type:   " << ptr3->type_name() << std::endl;
  std::cout << "MakePtr(\"Hello\") type: " << ptr4->type_name() << std::endl;
}

void test_value_numeric() {
  // Test Number constructor with integer (no decimal)
  json::Value value_1(42);
  std::cout << "value_1 type: " << value_1.type_name() << std::endl;

  // Test Number constructor with floating point
  json::Value value_2(3.14159);
  std::cout << "value_2 type: " << value_2.type_name() << std::endl;

  // Test Number constructor with large integer (no decimal)
  json::Value value_3(123456789);
  std::cout << "value_3 type: " << value_3.type_name() << std::endl;

  auto ui8 = std::uint8_t(0);
  auto si8 = std::int8_t(1);

  // Construct JSON values from ui8 and si8
  json::Value value_ui8(ui8);
  json::Value value_si8(si8);

  std::cout << "value_ui8 type: " << value_ui8.type_name() << std::endl;
  std::cout << "value_si8 type: " << value_si8.type_name() << std::endl;

  // 16-bit types
  auto ui16 = std::uint16_t(256);
  auto si16 = std::int16_t(-512);
  json::Value value_ui16(ui16);
  json::Value value_si16(si16);
  std::cout << "value_ui16 type: " << value_ui16.type_name() << std::endl;
  std::cout << "value_si16 type: " << value_si16.type_name() << std::endl;

  // 32-bit types
  auto ui32 = std::uint32_t(65536);
  auto si32 = std::int32_t(-131072);
  json::Value value_ui32(ui32);
  json::Value value_si32(si32);
  std::cout << "value_ui32 type: " << value_ui32.type_name() << std::endl;
  std::cout << "value_si32 type: " << value_si32.type_name() << std::endl;

  // 64-bit types
  auto ui64 = std::uint64_t(4294967296ULL);
  auto si64 = std::int64_t(-8589934592LL);
  json::Value value_ui64(ui64);
  json::Value value_si64(si64);
  std::cout << "value_ui64 type: " << value_ui64.type_name() << std::endl;
  std::cout << "value_si64 type: " << value_si64.type_name() << std::endl;

  // Float types
  auto f32 = 3.14159f;
  auto f64 = 2.718281828459045;
  json::Value value_f32(f32);
  json::Value value_f64(f64);
  std::cout << "value_f32 type: " << value_f32.type_name() << std::endl;
  std::cout << "value_f64 type: " << value_f64.type_name() << std::endl;
}

void test_as_number() {
  // Test converting stored Number back to different types
  json::Value num_value(42.7);

  // Convert to various integer types
  auto i8 = num_value.as_number<std::int8_t>();
  auto i16 = num_value.as_number<std::int16_t>();
  auto i32 = num_value.as_number<std::int32_t>();
  auto i64 = num_value.as_number<std::int64_t>();

  // Convert to various unsigned integer types
  auto ui8 = num_value.as_number<std::uint8_t>();
  auto ui16 = num_value.as_number<std::uint16_t>();
  auto ui32 = num_value.as_number<std::uint32_t>();
  auto ui64 = num_value.as_number<std::uint64_t>();

  // Convert to floating point types
  auto f32 = num_value.as_number<float>();
  auto f64 = num_value.as_number<double>();

  std::cout << "Original value: 42.7" << std::endl;
  std::cout << "as int8_t:   " << static_cast<int>(i8) << std::endl;
  std::cout << "as int16_t:  " << i16 << std::endl;
  std::cout << "as int32_t:  " << i32 << std::endl;
  std::cout << "as int64_t:  " << i64 << std::endl;
  std::cout << "as uint8_t:  " << static_cast<int>(ui8) << std::endl;
  std::cout << "as uint16_t: " << ui16 << std::endl;
  std::cout << "as uint32_t: " << ui32 << std::endl;
  std::cout << "as uint64_t: " << ui64 << std::endl;
  std::cout << "as float:    " << f32 << std::endl;
  std::cout << "as double:   " << f64 << std::endl;

  // Test as_bool conversion
  {
    json::Value value_1(true);
    json::Value value_2(false);
    json::Value value_3(0.0);
    json::Value value_4(5.5);

    std::cout << "value_1.as_bool(): " << bool_to_string(value_1.as_bool()) << std::endl;
    std::cout << "value_2.as_bool(): " << bool_to_string(value_2.as_bool()) << std::endl;
    std::cout << "value_3.as_bool(): " << bool_to_string(value_3.as_bool()) << std::endl;
    std::cout << "value_4.as_bool(): " << bool_to_string(value_4.as_bool()) << std::endl;
  }

  // Test as_string conversion
  {
    json::Value value("Hello, JSON!");
    std::cout << "value.as_string(): " << value.DebugString() << std::endl;
  }
}

void test_debug_string() {
  std::cout << "\n=== DebugString Tests ===" << std::endl;

  // Test null
  {
    json::Value null_value(nullptr);
    std::cout << "null: " << null_value.DebugString() << std::endl;
  }

  // Test bool
  {
    json::Value bool_true(true);
    json::Value bool_false(false);
    std::cout << "bool true: " << bool_true.DebugString() << std::endl;
    std::cout << "bool false: " << bool_false.DebugString() << std::endl;
  }

  // Test number
  {
    json::Value number(42.5);
    std::cout << "number: " << number.DebugString() << std::endl;
  }

  // Test string
  {
    json::Value str("Hello \"World\"");
    std::cout << "string: " << str.DebugString() << std::endl;
  }

  // Test list
  {
    json::List test_list = { json::MakePtr(1), json::MakePtr("test"), json::MakePtr(true) };
    json::Value list_value(test_list);
    std::cout << "list: " << list_value.DebugString() << std::endl;
  }

  // Test dict
  {
    json::Dict test_dict = { { "key1", json::MakePtr("value1") }, { "key2", json::MakePtr(42) } };
    json::Value dict_value(test_dict);
    std::cout << "dict: " << dict_value.DebugString() << std::endl;
  }

  // Test nested structures
  {
    json::Dict nested_dict = {
      { "array", json::MakePtr(json::List{ json::MakePtr(1), json::MakePtr(2) }) },
      { "object", json::MakePtr(json::Dict{ { "nested", json::MakePtr("value") } }) }
    };
    json::Value nested_value(nested_dict);
    std::cout << "nested: " << nested_value.DebugString() << std::endl;
  }
}

void test_setters() {
  std::cout << "\n=== Setter Tests ===" << std::endl;

  // Test set_bool
  {
    json::Value value;
    value.set_bool(true);
    std::cout << "set_bool(true): " << value.DebugString() << std::endl;
    value.set_bool(false);
    std::cout << "set_bool(false): " << value.DebugString() << std::endl;
  }

  // Test set_number
  {
    json::Value value;
    value.set_number(42.5);
    std::cout << "set_number(42.5): " << value.DebugString() << std::endl;
  }

  // Test set_string
  {
    json::Value value;
    value.set_string("Hello World");
    std::cout << "set_string(\"Hello World\"): " << value.DebugString() << std::endl;
    value.set_string(std::string("Moved String"));
    std::cout << "set_string(move): " << value.DebugString() << std::endl;
  }

  // Test set_list
  {
    json::Value value;
    json::List test_list = { json::MakePtr(1), json::MakePtr("test") };
    value.set_list(test_list);
    std::cout << "set_list: " << value.DebugString() << std::endl;
    value.set_list(json::List{ json::MakePtr(3), json::MakePtr(4) });
    std::cout << "set_list(move): " << value.DebugString() << std::endl;
  }

  // Test set_dict
  {
    json::Value value;
    json::Dict test_dict = { { "key", json::MakePtr("value") } };
    value.set_dict(test_dict);
    std::cout << "set_dict: " << value.DebugString() << std::endl;
    value.set_dict(json::Dict{ { "a", json::MakePtr(1) }, { "b", json::MakePtr(2) } });
    std::cout << "set_dict(move): " << value.DebugString() << std::endl;
  }

  // Test set_null
  {
    json::Value value;
    value.set_null();
    std::cout << "set_null(): " << value.DebugString() << std::endl;
  }

  // Test changing types
  {
    json::Value value;
    value.set_string("initial");
    std::cout << "initial string: " << value.DebugString() << std::endl;
    value.set_number(123);
    std::cout << "changed to number: " << value.DebugString() << std::endl;
    value.set_bool(true);
    std::cout << "changed to bool: " << value.DebugString() << std::endl;
  }
}

void test_mutable_accessors() {
  std::cout << "\n=== Mutable Accessor Tests ===" << std::endl;

  // Test mutable_string
  {
    json::Value value("initial");
    std::cout << "before: " << value.DebugString() << std::endl;

    json::String& str = value.mutable_string();
    str = "modified";
    std::cout << "after modification: " << value.DebugString() << std::endl;

    str += " and extended";
    std::cout << "after extension: " << value.DebugString() << std::endl;
  }

  // Test mutable_list
  {
    json::Value value(json::List{ json::MakePtr(1), json::MakePtr(2) });
    std::cout << "before: " << value.DebugString() << std::endl;

    json::List& list = value.mutable_list();
    list.push_back(json::MakePtr(3));
    list[0] = json::MakePtr(999);
    std::cout << "after modification: " << value.DebugString() << std::endl;
  }

  // Test mutable_dict
  {
    json::Value value(json::Dict{ { "key1", json::MakePtr("value1") } });
    std::cout << "before: " << value.DebugString() << std::endl;

    json::Dict& dict = value.mutable_dict();
    dict["key2"] = json::MakePtr(42);
    dict["key1"] = json::MakePtr("modified");
    std::cout << "after modification: " << value.DebugString() << std::endl;
  }

  // Test error cases
  {
    json::Value value(42);
    try {
      value.mutable_string();
      std::cout << "ERROR: Should have thrown exception" << std::endl;
    } catch (const std::runtime_error& e) {
      std::cout << "Correctly caught exception for wrong type: " << e.what() << std::endl;
    }
  }
}

int main() {
  // Simple test to ensure the header compiles
  json::Value value;
  std::cout << "default value type: " << value.type_name() << std::endl;

  // Test Bool constructor
  test_value_bool();

  // Test Null constructor
  test_value_null();

  // Test String constructor
  test_value_string();

  // Test List constructor
  test_value_list();

  // Test Dict constructor
  test_value_dict();

  // Test MakePtr helper function
  test_make_ptr();

  // Test numeric constructors
  test_value_numeric();

  // Test as_number conversions
  test_as_number();

  // Test DebugString functionality
  test_debug_string();

  // Test setter methods
  test_setters();

  // Test mutable accessor methods
  test_mutable_accessors();

  // Test TSC functionality
  test_tsc();

  // Test TSC difference over time
  test_tsc_difference();

  return 0;
}