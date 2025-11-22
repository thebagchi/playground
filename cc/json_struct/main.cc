#include <boost/json.hpp>
#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <tuple>

#include "json.h"

class Person {
 public:
  std::string name_;
  std::unique_ptr<std::uint64_t> age_;
  std::shared_ptr<std::string> city_;
  std::optional<std::string> email_;

 public:
  constexpr const static auto properties = std::make_tuple(
      prop(&Person::name_, "name"), prop(&Person::age_, "age"),
      prop(&Person::city_, "city"), prop(&Person::email_, "email"));
};

void encode_decode_struct() {
  try {
    // Create a Person with mixed types
    Person p1;
    p1.name_ = "John Doe";                                 // direct string
    p1.age_ = std::make_unique<std::uint64_t>(30);         // unique_ptr
    p1.city_ = std::make_shared<std::string>("New York");  // shared_ptr
    p1.email_ = "john@example.com";                        // optional

    // Marshal to JSON
    boost::json::value json_value = Marshal(p1);
    std::cout << "Encoded JSON: " << json_value << std::endl;

    // Marshal to JSON string
    std::string json_string = MarshalToString(p1);
    std::cout << "Encoded JSON string: " << json_string << std::endl;

    // Unmarshal back to Person from string
    Person p2;
    UnmarshalFromString(json_string, p2);

    // Display decoded results
    std::cout << "Decoded Person:" << std::endl;
    std::cout << "  Name: " << p2.name_ << std::endl;
    if (p2.age_) {
      std::cout << "  Age: " << *p2.age_ << std::endl;
    }
    if (p2.city_) {
      std::cout << "  City: " << *p2.city_ << std::endl;
    }
    if (p2.email_) {
      std::cout << "  Email: " << *p2.email_ << std::endl;
    }

  } catch (const std::exception& e) {
    std::cout << "Error in encode_decode_struct: " << e.what() << std::endl;
  }
}

void encode_decode_uint64() {
  try {
    // Create uint64_t values
    std::uint64_t original_value = 1234567890123456789ULL;
    std::uint64_t decoded_value = 0;

    // Marshal to JSON
    boost::json::value json_value = Marshal(original_value);
    std::cout << "Encoded uint64 JSON: " << json_value << std::endl;

    // Marshal to JSON string
    std::string json_string = MarshalToString(original_value);
    std::cout << "Encoded uint64 JSON string: " << json_string << std::endl;

    // Unmarshal back to uint64_t from string
    UnmarshalFromString(json_string, decoded_value);

    // Display decoded results
    std::cout << "Decoded uint64_t:" << std::endl;
    std::cout << "  Original: " << original_value << std::endl;
    std::cout << "  Decoded:  " << decoded_value << std::endl;
    std::cout << "  Match: " << (original_value == decoded_value ? "Yes" : "No")
              << std::endl;

  } catch (const std::exception& e) {
    std::cout << "Error in encode_decode_uint64: " << e.what() << std::endl;
  }
}

void encode_decode_string() {
  try {
    // Create string values
    std::string original_value = "Hello, JSON World!";
    std::string decoded_value;

    // Marshal to JSON
    boost::json::value json_value = Marshal(original_value);
    std::cout << "Encoded string JSON: " << json_value << std::endl;

    // Marshal to JSON string
    std::string json_string = MarshalToString(original_value);
    std::cout << "Encoded string JSON string: " << json_string << std::endl;

    // Unmarshal back to string from string
    UnmarshalFromString(json_string, decoded_value);

    // Display decoded results
    std::cout << "Decoded string:" << std::endl;
    std::cout << "  Original: \"" << original_value << "\"" << std::endl;
    std::cout << "  Decoded:  \"" << decoded_value << "\"" << std::endl;
    std::cout << "  Match: " << (original_value == decoded_value ? "Yes" : "No")
              << std::endl;

  } catch (const std::exception& e) {
    std::cout << "Error in encode_decode_string: " << e.what() << std::endl;
  }
}

void encode_decode_double() {
  try {
    // Create double values
    double original_value = 3.14159265358979323846;
    double decoded_value = 0.0;

    // Marshal to JSON
    boost::json::value json_value = Marshal(original_value);
    std::cout << "Encoded double JSON: " << json_value << std::endl;

    // Marshal to JSON string
    std::string json_string = MarshalToString(original_value);
    std::cout << "Encoded double JSON string: " << json_string << std::endl;

    // Unmarshal back to double from string
    UnmarshalFromString(json_string, decoded_value);

    // Display decoded results
    std::cout << "Decoded double:" << std::endl;
    std::cout << "  Original: " << original_value << std::endl;
    std::cout << "  Decoded:  " << decoded_value << std::endl;
    std::cout << "  Match: " << (original_value == decoded_value ? "Yes" : "No")
              << std::endl;

  } catch (const std::exception& e) {
    std::cout << "Error in encode_decode_double: " << e.what() << std::endl;
  }
}

void encode_decode_bool() {
  try {
    // Create bool values
    bool original_value = true;
    bool decoded_value = false;

    // Marshal to JSON
    boost::json::value json_value = Marshal(original_value);
    std::cout << "Encoded bool JSON: " << json_value << std::endl;

    // Marshal to JSON string
    std::string json_string = MarshalToString(original_value);
    std::cout << "Encoded bool JSON string: " << json_string << std::endl;

    // Unmarshal back to bool from string
    UnmarshalFromString(json_string, decoded_value);

    // Display decoded results
    std::cout << "Decoded bool:" << std::endl;
    std::cout << "  Original: " << (original_value ? "true" : "false")
              << std::endl;
    std::cout << "  Decoded:  " << (decoded_value ? "true" : "false")
              << std::endl;
    std::cout << "  Match: " << (original_value == decoded_value ? "Yes" : "No")
              << std::endl;

  } catch (const std::exception& e) {
    std::cout << "Error in encode_decode_bool: " << e.what() << std::endl;
  }
}

void encode_decode_int64() {
  try {
    // Create int64_t values (negative number)
    std::int64_t original_value =
        -9223372036854775807LL;  // Large negative number
    std::int64_t decoded_value = 0;

    // Marshal to JSON
    boost::json::value json_value = Marshal(original_value);
    std::cout << "Encoded int64 JSON: " << json_value << std::endl;

    // Marshal to JSON string
    std::string json_string = MarshalToString(original_value);
    std::cout << "Encoded int64 JSON string: " << json_string << std::endl;

    // Unmarshal back to int64_t from string
    UnmarshalFromString(json_string, decoded_value);

    // Display decoded results
    std::cout << "Decoded int64_t:" << std::endl;
    std::cout << "  Original: " << original_value << std::endl;
    std::cout << "  Decoded:  " << decoded_value << std::endl;
    std::cout << "  Match: " << (original_value == decoded_value ? "Yes" : "No")
              << std::endl;

  } catch (const std::exception& e) {
    std::cout << "Error in encode_decode_int64: " << e.what() << std::endl;
  }
}

void encode_decode_vector() {
  try {
    // Create vector of strings
    std::vector<std::string> original_vector = {"apple", "banana", "cherry",
                                                "date"};
    std::vector<std::string> decoded_vector;

    // Marshal to JSON
    boost::json::value json_value = Marshal(original_vector);
    std::cout << "Encoded vector JSON: " << json_value << std::endl;

    // Marshal to JSON string
    std::string json_string = MarshalToString(original_vector);
    std::cout << "Encoded vector JSON string: " << json_string << std::endl;

    // Unmarshal back to vector from string
    UnmarshalFromString(json_string, decoded_vector);

    // Display decoded results
    std::cout << "Decoded vector:" << std::endl;
    std::cout << "  Original: [";
    for (size_t i = 0; i < original_vector.size(); ++i) {
      std::cout << "\"" << original_vector[i] << "\"";
      if (i < original_vector.size() - 1) std::cout << ", ";
    }
    std::cout << "]" << std::endl;

    std::cout << "  Decoded:  [";
    for (size_t i = 0; i < decoded_vector.size(); ++i) {
      std::cout << "\"" << decoded_vector[i] << "\"";
      if (i < decoded_vector.size() - 1) std::cout << ", ";
    }
    std::cout << "]" << std::endl;

    std::cout << "  Match: "
              << (original_vector == decoded_vector ? "Yes" : "No")
              << std::endl;

  } catch (const std::exception& e) {
    std::cout << "Error in encode_decode_vector: " << e.what() << std::endl;
  }
}

void encode_decode_map() {
  try {
    // Create map of string to string
    std::map<std::string, std::string> original_map = {
        {"name", "Alice"}, {"city", "Boston"}, {"country", "USA"}};
    std::map<std::string, std::string> decoded_map;

    // Marshal to JSON
    boost::json::value json_value = Marshal(original_map);
    std::cout << "Encoded map JSON: " << json_value << std::endl;

    // Marshal to JSON string
    std::string json_string = MarshalToString(original_map);
    std::cout << "Encoded map JSON string: " << json_string << std::endl;

    // Unmarshal back to map from string
    UnmarshalFromString(json_string, decoded_map);

    // Display decoded results
    std::cout << "Decoded map:" << std::endl;
    std::cout << "  Original: {";
    for (auto it = original_map.begin(); it != original_map.end(); ++it) {
      std::cout << "\"" << it->first << "\": \"" << it->second << "\"";
      if (std::next(it) != original_map.end()) std::cout << ", ";
    }
    std::cout << "}" << std::endl;

    std::cout << "  Decoded:  {";
    for (auto it = decoded_map.begin(); it != decoded_map.end(); ++it) {
      std::cout << "\"" << it->first << "\": \"" << it->second << "\"";
      if (std::next(it) != decoded_map.end()) std::cout << ", ";
    }
    std::cout << "}" << std::endl;

    std::cout << "  Match: " << (original_map == decoded_map ? "Yes" : "No")
              << std::endl;

  } catch (const std::exception& e) {
    std::cout << "Error in encode_decode_map: " << e.what() << std::endl;
  }
}

void encode_decode_optional() {
  try {
    // Create optional string (with value)
    std::optional<std::string> original_optional = "Hello Optional World!";
    std::optional<std::string> decoded_optional;

    // Marshal to JSON
    boost::json::value json_value = Marshal(original_optional);
    std::cout << "Encoded optional JSON: " << json_value << std::endl;

    // Marshal to JSON string
    std::string json_string = MarshalToString(original_optional);
    std::cout << "Encoded optional JSON string: " << json_string << std::endl;

    // Unmarshal back to optional from string
    UnmarshalFromString(json_string, decoded_optional);

    // Display decoded results
    std::cout << "Decoded optional:" << std::endl;
    std::cout << "  Original: "
              << (original_optional ? "\"" + *original_optional + "\""
                                    : "nullopt")
              << std::endl;
    std::cout << "  Decoded:  "
              << (decoded_optional ? "\"" + *decoded_optional + "\""
                                   : "nullopt")
              << std::endl;
    std::cout << "  Match: "
              << (original_optional == decoded_optional ? "Yes" : "No")
              << std::endl;

  } catch (const std::exception& e) {
    std::cout << "Error in encode_decode_optional: " << e.what() << std::endl;
  }
}

void encode_decode_arbitrary() {
  try {
    // Create arbitrary JSON value (complex object)
    boost::json::value original_arbitrary = {
        {"message", "Hello arbitrary JSON!"},
        {"number", 42},
        {"boolean", true},
        {"array", {1, 2, 3, 4, 5}},
        {"nested", {{"key", "value"}, {"count", 100}}}};
    boost::json::value decoded_arbitrary;

    // Marshal to JSON
    boost::json::value json_value = Marshal(original_arbitrary);
    std::cout << "Encoded arbitrary JSON: " << json_value << std::endl;

    // Marshal to JSON string
    std::string json_string = MarshalToString(original_arbitrary);
    std::cout << "Encoded arbitrary JSON string: " << json_string << std::endl;

    // Unmarshal back to arbitrary JSON from string
    UnmarshalFromString(json_string, decoded_arbitrary);

    // Display decoded results
    std::cout << "Decoded arbitrary JSON:" << std::endl;
    std::cout << "  Original: " << original_arbitrary << std::endl;
    std::cout << "  Decoded:  " << decoded_arbitrary << std::endl;
    std::cout << "  Match: "
              << (original_arbitrary == decoded_arbitrary ? "Yes" : "No")
              << std::endl;

  } catch (const std::exception& e) {
    std::cout << "Error in encode_decode_arbitrary: " << e.what() << std::endl;
  }
}

int main(int argc, char* argv[]) {
  std::cout << "hello world" << std::endl;

  try {
    encode_decode_struct();
    encode_decode_uint64();
    encode_decode_int64();
    encode_decode_string();
    encode_decode_double();
    encode_decode_bool();
    encode_decode_vector();
    encode_decode_map();
    encode_decode_optional();
    encode_decode_arbitrary();
  } catch (const std::exception& e) {
    std::cerr << "Error in main: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
