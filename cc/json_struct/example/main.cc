#include <iomanip>
#include <iostream>
#include <tuple>

#include "cbor.h"
#include "json.h"
#include "typing.h"

using namespace json;

class Person {
public:
  String name_;
  UniquePtr<UInt64> age_;
  SharedPtr<String> city_;
  Optional<String> email_;
};

template <> struct json::STRUCT<Person> {
  static constexpr auto properties = std::make_tuple(prop(&Person::name_, "name"),
   prop(&Person::age_, "age"),
   prop(&Person::city_, "city"),
   prop(&Person::email_, "email"));
};

void encode_decode_struct() {
  try {
    // Create a Person with mixed types
    Person p1;
    p1.name_ = "John Doe";                      // direct string
    p1.age_ = MAKE_UNIQUE(UInt64, 30);          // unique_ptr
    p1.city_ = MAKE_SHARED(String, "New York"); // shared_ptr
    p1.email_ = "john@example.com";             // optional

    // Marshal to JSON
    boost::json::value json_value = json::Marshal(p1);
    std::cout << "Encoded JSON: " << json_value << std::endl;

    // Marshal to JSON string
    String json_string = json::MarshalToString(p1);
    std::cout << "Encoded JSON string: " << json_string << std::endl;

    // Unmarshal back to Person from string
    Person p2;
    json::UnmarshalFromString(json_string, p2);

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
    UInt64 original_value = 1234567890123456789ULL;
    UInt64 decoded_value = 0;

    // Marshal to JSON
    boost::json::value json_value = json::Marshal(original_value);
    std::cout << "Encoded uint64 JSON: " << json_value << std::endl;

    // Marshal to JSON string
    String json_string = json::MarshalToString(original_value);
    std::cout << "Encoded uint64 JSON string: " << json_string << std::endl;

    // Unmarshal back to uint64_t from string
    json::UnmarshalFromString(json_string, decoded_value);

    // Display decoded results
    std::cout << "Decoded uint64_t:" << std::endl;
    std::cout << "  Original: " << original_value << std::endl;
    std::cout << "  Decoded:  " << decoded_value << std::endl;
    std::cout << "  Match: " << (original_value == decoded_value ? "Yes" : "No") << std::endl;

  } catch (const std::exception& e) {
    std::cout << "Error in encode_decode_uint64: " << e.what() << std::endl;
  }
}

void encode_decode_string() {
  try {
    // Create string values
    String original_value = "Hello, JSON World!";
    String decoded_value;

    // Marshal to JSON
    boost::json::value json_value = json::Marshal(original_value);
    std::cout << "Encoded string JSON: " << json_value << std::endl;

    // Marshal to JSON string
    String json_string = json::MarshalToString(original_value);
    std::cout << "Encoded string JSON string: " << json_string << std::endl;

    // Unmarshal back to string from string
    json::UnmarshalFromString(json_string, decoded_value);

    // Display decoded results
    std::cout << "Decoded string:" << std::endl;
    std::cout << "  Original: \"" << original_value << "\"" << std::endl;
    std::cout << "  Decoded:  \"" << decoded_value << "\"" << std::endl;
    std::cout << "  Match: " << (original_value == decoded_value ? "Yes" : "No") << std::endl;

  } catch (const std::exception& e) {
    std::cout << "Error in encode_decode_string: " << e.what() << std::endl;
  }
}

void encode_decode_double() {
  try {
    // Create double values
    Double original_value = 3.14159265358979323846;
    Double decoded_value = 0.0;

    // Marshal to JSON
    boost::json::value json_value = json::Marshal(original_value);
    std::cout << "Encoded double JSON: " << json_value << std::endl;

    // Marshal to JSON string
    String json_string = json::MarshalToString(original_value);
    std::cout << "Encoded double JSON string: " << json_string << std::endl;

    // Unmarshal back to double from string
    json::UnmarshalFromString(json_string, decoded_value);

    // Display decoded results
    std::cout << "Decoded double:" << std::endl;
    std::cout << "  Original: " << original_value << std::endl;
    std::cout << "  Decoded:  " << decoded_value << std::endl;
    std::cout << "  Match: " << (original_value == decoded_value ? "Yes" : "No") << std::endl;

  } catch (const std::exception& e) {
    std::cout << "Error in encode_decode_double: " << e.what() << std::endl;
  }
}

void encode_decode_bool() {
  try {
    // Create bool values
    Bool original_value = true;
    Bool decoded_value = false;

    // Marshal to JSON
    boost::json::value json_value = json::Marshal(original_value);
    std::cout << "Encoded bool JSON: " << json_value << std::endl;

    // Marshal to JSON string
    String json_string = json::MarshalToString(original_value);
    std::cout << "Encoded bool JSON string: " << json_string << std::endl;

    // Unmarshal back to bool from string
    json::UnmarshalFromString(json_string, decoded_value);

    // Display decoded results
    std::cout << "Decoded bool:" << std::endl;
    std::cout << "  Original: " << (original_value ? "true" : "false") << std::endl;
    std::cout << "  Decoded:  " << (decoded_value ? "true" : "false") << std::endl;
    std::cout << "  Match: " << (original_value == decoded_value ? "Yes" : "No") << std::endl;

  } catch (const std::exception& e) {
    std::cout << "Error in encode_decode_bool: " << e.what() << std::endl;
  }
}

void encode_decode_int64() {
  try {
    // Create int64_t values (negative number)
    Int64 original_value = -9223372036854775807LL; // Large negative number
    Int64 decoded_value = 0;

    // Marshal to JSON
    boost::json::value json_value = json::Marshal(original_value);
    std::cout << "Encoded int64 JSON: " << json_value << std::endl;

    // Marshal to JSON string
    String json_string = json::MarshalToString(original_value);
    std::cout << "Encoded int64 JSON string: " << json_string << std::endl;

    // Unmarshal back to int64_t from string
    json::UnmarshalFromString(json_string, decoded_value);

    // Display decoded results
    std::cout << "Decoded int64_t:" << std::endl;
    std::cout << "  Original: " << original_value << std::endl;
    std::cout << "  Decoded:  " << decoded_value << std::endl;
    std::cout << "  Match: " << (original_value == decoded_value ? "Yes" : "No") << std::endl;

  } catch (const std::exception& e) {
    std::cout << "Error in encode_decode_int64: " << e.what() << std::endl;
  }
}

void encode_decode_vector() {
  try {
    // Create vector of strings
    Vector<String> original_vector = { "apple", "banana", "cherry", "date" };
    Vector<String> decoded_vector;

    // Marshal to JSON
    boost::json::value json_value = json::Marshal(original_vector);
    std::cout << "Encoded vector JSON: " << json_value << std::endl;

    // Marshal to JSON string
    String json_string = json::MarshalToString(original_vector);
    std::cout << "Encoded vector JSON string: " << json_string << std::endl;

    // Unmarshal back to vector from string
    json::UnmarshalFromString(json_string, decoded_vector);

    // Display decoded results
    std::cout << "Decoded vector:" << std::endl;
    std::cout << "  Original: [";
    for (size_t i = 0; i < original_vector.size(); ++i) {
      std::cout << "\"" << original_vector[i] << "\"";
      if (i < original_vector.size() - 1) {
        std::cout << ", ";
      }
    }
    std::cout << "]" << std::endl;

    std::cout << "  Decoded:  [";
    for (size_t i = 0; i < decoded_vector.size(); ++i) {
      std::cout << "\"" << decoded_vector[i] << "\"";
      if (i < decoded_vector.size() - 1) {
        std::cout << ", ";
      }
    }
    std::cout << "]" << std::endl;

    std::cout << "  Match: " << (original_vector == decoded_vector ? "Yes" : "No") << std::endl;

  } catch (const std::exception& e) {
    std::cout << "Error in encode_decode_vector: " << e.what() << std::endl;
  }
}

void encode_decode_array() {
  try {
    // Create array of strings
    Array<String, 4> original_array = { "apple", "banana", "cherry", "date" };
    Array<String, 4> decoded_array;

    // Marshal to JSON
    boost::json::value json_value = json::Marshal(original_array);
    std::cout << "Encoded array JSON: " << json_value << std::endl;

    // Marshal to JSON string
    String json_string = json::MarshalToString(original_array);
    std::cout << "Encoded array JSON string: " << json_string << std::endl;

    // Unmarshal back to array from string
    json::UnmarshalFromString(json_string, decoded_array);

    // Display decoded results
    std::cout << "Decoded array:" << std::endl;
    std::cout << "  Original: [";
    for (size_t i = 0; i < original_array.size(); ++i) {
      std::cout << original_array[i];
      if (i < original_array.size() - 1) {
        std::cout << ", ";
      }
    }
    std::cout << "]" << std::endl;

    std::cout << "  Decoded:  [";
    for (size_t i = 0; i < decoded_array.size(); ++i) {
      std::cout << decoded_array[i];
      if (i < decoded_array.size() - 1) {
        std::cout << ", ";
      }
    }
    std::cout << "]" << std::endl;

    std::cout << "  Match: " << (original_array == decoded_array ? "Yes" : "No") << std::endl;

  } catch (const std::exception& e) {
    std::cout << "Error in encode_decode_array: " << e.what() << std::endl;
  }
}

void encode_decode_list() {
  try {
    // Create list of strings
    List<String> original_list = { "apple", "banana", "cherry", "date" };
    List<String> decoded_list;

    // Marshal to JSON
    boost::json::value json_value = json::Marshal(original_list);
    std::cout << "Encoded list JSON: " << json_value << std::endl;

    // Marshal to JSON string
    String json_string = json::MarshalToString(original_list);
    std::cout << "Encoded list JSON string: " << json_string << std::endl;

    // Unmarshal back to list from string
    json::UnmarshalFromString(json_string, decoded_list);

    // Display decoded results
    std::cout << "Decoded list:" << std::endl;
    std::cout << "  Original: [";
    auto it_orig = original_list.begin();
    for (size_t i = 0; i < original_list.size(); ++i, ++it_orig) {
      std::cout << "\"" << *it_orig << "\"";
      if (i < original_list.size() - 1) {
        std::cout << ", ";
      }
    }
    std::cout << "]" << std::endl;

    std::cout << "  Decoded:  [";
    auto it_dec = decoded_list.begin();
    for (size_t i = 0; i < decoded_list.size(); ++i, ++it_dec) {
      std::cout << "\"" << *it_dec << "\"";
      if (i < decoded_list.size() - 1) {
        std::cout << ", ";
      }
    }
    std::cout << "]" << std::endl;

    std::cout << "  Match: " << (original_list == decoded_list ? "Yes" : "No") << std::endl;

  } catch (const std::exception& e) {
    std::cout << "Error in encode_decode_list: " << e.what() << std::endl;
  }
}

void encode_decode_map() {
  try {
    // Create map of string to string
    Map<String> original_map = { { "name", "Alice" }, { "city", "Boston" }, { "country", "USA" } };
    Map<String> decoded_map;

    // Marshal to JSON
    boost::json::value json_value = json::Marshal(original_map);
    std::cout << "Encoded map JSON: " << json_value << std::endl;

    // Marshal to JSON string
    String json_string = json::MarshalToString(original_map);
    std::cout << "Encoded map JSON string: " << json_string << std::endl;

    // Unmarshal back to map from string
    json::UnmarshalFromString(json_string, decoded_map);

    // Display decoded results
    std::cout << "Decoded map:" << std::endl;
    std::cout << "  Original: {";
    for (auto it = original_map.begin(); it != original_map.end(); ++it) {
      std::cout << "\"" << it->first << "\": \"" << it->second << "\"";
      if (std::next(it) != original_map.end()) {
        std::cout << ", ";
      }
    }
    std::cout << "}" << std::endl;

    std::cout << "  Decoded:  {";
    for (auto it = decoded_map.begin(); it != decoded_map.end(); ++it) {
      std::cout << "\"" << it->first << "\": \"" << it->second << "\"";
      if (std::next(it) != decoded_map.end()) {
        std::cout << ", ";
      }
    }
    std::cout << "}" << std::endl;

    std::cout << "  Match: " << (original_map == decoded_map ? "Yes" : "No") << std::endl;

  } catch (const std::exception& e) {
    std::cout << "Error in encode_decode_map: " << e.what() << std::endl;
  }
}

void encode_decode_multimap() {
  try {
    // Create multimap with duplicate keys
    MultiMap<String> original_multimap;
    original_multimap.insert({ "fruits", "apple" });
    original_multimap.insert({ "fruits", "banana" });
    original_multimap.insert({ "fruits", "cherry" });
    original_multimap.insert({ "colors", "red" });
    original_multimap.insert({ "colors", "blue" });
    original_multimap.insert({ "numbers", "42" });

    MultiMap<String> decoded_multimap;

    // Marshal to JSON
    boost::json::value json_value = json::Marshal(original_multimap);
    std::cout << "Encoded multimap JSON: " << json_value << std::endl;

    // Marshal to JSON string
    String json_string = json::MarshalToString(original_multimap);
    std::cout << "Encoded multimap JSON string: " << json_string << std::endl;

    // Unmarshal back to multimap from string
    json::UnmarshalFromString(json_string, decoded_multimap);

    // Display decoded results
    std::cout << "Decoded multimap:" << std::endl;
    std::cout << "  Original: {";
    for (auto it = original_multimap.begin(); it != original_multimap.end(); ++it) {
      std::cout << "\"" << it->first << "\": \"" << it->second << "\"";
      if (std::next(it) != original_multimap.end()) {
        std::cout << ", ";
      }
    }
    std::cout << "}" << std::endl;

    std::cout << "  Decoded:  {";
    for (auto it = decoded_multimap.begin(); it != decoded_multimap.end(); ++it) {
      std::cout << "\"" << it->first << "\": \"" << it->second << "\"";
      if (std::next(it) != decoded_multimap.end()) {
        std::cout << ", ";
      }
    }
    std::cout << "}" << std::endl;

    std::cout << "  Match: " << (original_multimap == decoded_multimap ? "Yes" : "No") << std::endl;

  } catch (const std::exception& e) {
    std::cout << "Error in encode_decode_multimap: " << e.what() << std::endl;
  }
}

void encode_decode_pair() {
  try {
    // Create pair with string key and integer value
    Pair<Int64> original_pair = { "age", 30 };

    Pair<Int64> decoded_pair;

    // Marshal to JSON
    boost::json::value json_value = json::Marshal(original_pair);
    std::cout << "Encoded pair JSON: " << json_value << std::endl;

    // Marshal to JSON string
    String json_string = json::MarshalToString(original_pair);
    std::cout << "Encoded pair JSON string: " << json_string << std::endl;

    // Unmarshal back to pair from string
    json::UnmarshalFromString(json_string, decoded_pair);

    // Display decoded results
    std::cout << "Decoded pair:" << std::endl;
    std::cout << "  Original: {\"" << original_pair.first << "\": " << original_pair.second << "}"
              << std::endl;
    std::cout << "  Decoded:  {\"" << decoded_pair.first << "\": " << decoded_pair.second << "}"
              << std::endl;
    std::cout << "  Match: " << (original_pair == decoded_pair ? "Yes" : "No") << std::endl;

    // Test with complex type
    Pair<Vector<String>> complex_pair = { "tags", { "c++", "json", "boost" } };
    boost::json::value complex_json = json::Marshal(complex_pair);
    std::cout << "Encoded complex pair JSON: " << complex_json << std::endl;

    Pair<Vector<String>> decoded_complex_pair;
    json::UnmarshalFromString(json::MarshalToString(complex_pair), decoded_complex_pair);
    std::cout << "  Complex pair match: " << (complex_pair == decoded_complex_pair ? "Yes" : "No")
              << std::endl;

  } catch (const std::exception& e) {
    std::cout << "Error in encode_decode_pair: " << e.what() << std::endl;
  }
}

void encode_decode_optional() {
  try {
    // Create optional string (with value)
    Optional<String> original_optional = "Hello Optional World!";
    Optional<String> decoded_optional;

    // Marshal to JSON
    boost::json::value json_value = json::Marshal(original_optional);
    std::cout << "Encoded optional JSON: " << json_value << std::endl;

    // Marshal to JSON string
    String json_string = json::MarshalToString(original_optional);
    std::cout << "Encoded optional JSON string: " << json_string << std::endl;

    // Unmarshal back to optional from string
    json::UnmarshalFromString(json_string, decoded_optional);

    // Display decoded results
    std::cout << "Decoded optional:" << std::endl;
    std::cout << "  Original: "
              << (original_optional ? "\"" + *original_optional + "\"" : "nullopt") << std::endl;
    std::cout << "  Decoded:  " << (decoded_optional ? "\"" + *decoded_optional + "\"" : "nullopt")
              << std::endl;
    std::cout << "  Match: " << (original_optional == decoded_optional ? "Yes" : "No") << std::endl;

  } catch (const std::exception& e) {
    std::cout << "Error in encode_decode_optional: " << e.what() << std::endl;
  }
}

void encode_decode_byte_array() {
  try {
    // Create byte array with some sample data
    ByteVector original_bytes = { std::byte{ 0x48 },
      std::byte{ 0x65 },
      std::byte{ 0x6C },
      std::byte{ 0x6C },
      std::byte{ 0x6F },
      std::byte{ 0x20 },
      std::byte{ 0x57 },
      std::byte{ 0x6F },
      std::byte{ 0x72 },
      std::byte{ 0x6C },
      std::byte{ 0x64 },
      std::byte{ 0x21 } };
    ByteVector decoded_bytes;

    // Marshal to JSON
    boost::json::value json_value = json::Marshal(original_bytes);
    std::cout << "Encoded byte array JSON: " << json_value << std::endl;

    // Marshal to JSON string
    String json_string = json::MarshalToString(original_bytes);
    std::cout << "Encoded byte array JSON string: " << json_string << std::endl;

    // Unmarshal back to byte array from string
    json::UnmarshalFromString(json_string, decoded_bytes);

    // Display decoded results
    std::cout << "Decoded byte array:" << std::endl;
    std::cout << "  Original: [";
    for (size_t i = 0; i < original_bytes.size(); ++i) {
      std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0')
                << static_cast<int>(original_bytes[i]);
      if (i < original_bytes.size() - 1) {
        std::cout << ", ";
      }
    }
    std::cout << "]" << std::endl;

    std::cout << "  Decoded:  [";
    for (size_t i = 0; i < decoded_bytes.size(); ++i) {
      std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0')
                << static_cast<int>(decoded_bytes[i]);
      if (i < decoded_bytes.size() - 1) {
        std::cout << ", ";
      }
    }
    std::cout << "]" << std::endl;

    std::cout << "  Match: " << (original_bytes == decoded_bytes ? "Yes" : "No") << std::endl;

  } catch (const std::exception& e) {
    std::cout << "Error in encode_decode_byte_array: " << e.what() << std::endl;
  }
}

void encode_decode_byte_array_fixed() {
  try {
    // Create fixed-size byte array with some sample data (12 bytes)
    ByteArray<12> original_bytes = { std::byte{ 0x48 },
      std::byte{ 0x65 },
      std::byte{ 0x6C },
      std::byte{ 0x6C },
      std::byte{ 0x6F },
      std::byte{ 0x20 },
      std::byte{ 0x57 },
      std::byte{ 0x6F },
      std::byte{ 0x72 },
      std::byte{ 0x6C },
      std::byte{ 0x64 },
      std::byte{ 0x21 } };
    ByteArray<12> decoded_bytes{};

    // Marshal to JSON
    boost::json::value json_value = json::Marshal(original_bytes);
    std::cout << "Encoded fixed-size byte array JSON: " << json_value << std::endl;

    // Marshal to JSON string
    String json_string = json::MarshalToString(original_bytes);
    std::cout << "Encoded fixed-size byte array JSON string: " << json_string << std::endl;

    // Unmarshal back to byte array from string
    json::UnmarshalFromString(json_string, decoded_bytes);

    // Display decoded results
    std::cout << "Decoded fixed-size byte array:" << std::endl;
    std::cout << "  Original: [";
    for (size_t i = 0; i < original_bytes.size(); ++i) {
      std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0')
                << static_cast<int>(original_bytes[i]);
      if (i < original_bytes.size() - 1) {
        std::cout << ", ";
      }
    }
    std::cout << "]" << std::endl;

    std::cout << "  Decoded:  [";
    for (size_t i = 0; i < decoded_bytes.size(); ++i) {
      std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0')
                << static_cast<int>(decoded_bytes[i]);
      if (i < decoded_bytes.size() - 1) {
        std::cout << ", ";
      }
    }
    std::cout << "]" << std::endl;

    std::cout << "  Match: " << (original_bytes == decoded_bytes ? "Yes" : "No") << std::endl;

  } catch (const std::exception& e) {
    std::cout << "Error in encode_decode_byte_array_fixed: " << e.what() << std::endl;
  }
}

void encode_decode_arbitrary() {
  try {
    // Create arbitrary JSON value (complex object)
    boost::json::value original_arbitrary = { { "message", "Hello arbitrary JSON!" },
      { "number", 42 },
      { "boolean", true },
      { "array", { 1, 2, 3, 4, 5 } },
      { "nested", { { "key", "value" }, { "count", 100 } } } };
    boost::json::value decoded_arbitrary;

    // Marshal to JSON
    boost::json::value json_value = json::Marshal(original_arbitrary);
    std::cout << "Encoded arbitrary JSON: " << json_value << std::endl;

    // Marshal to JSON string
    String json_string = json::MarshalToString(original_arbitrary);
    std::cout << "Encoded arbitrary JSON string: " << json_string << std::endl;

    // Unmarshal back to arbitrary JSON from string
    json::UnmarshalFromString(json_string, decoded_arbitrary);

    // Display decoded results
    std::cout << "Decoded arbitrary JSON:" << std::endl;
    std::cout << "  Original: " << original_arbitrary << std::endl;
    std::cout << "  Decoded:  " << decoded_arbitrary << std::endl;
    std::cout << "  Match: " << (original_arbitrary == decoded_arbitrary ? "Yes" : "No")
              << std::endl;

  } catch (const std::exception& e) {
    std::cout << "Error in encode_decode_arbitrary: " << e.what() << std::endl;
  }
}

// =============================================================================
// HTTP Status Code Enum Examples
// =============================================================================

// Enum 1: HTTP Status Code as NUMBER (integer values)
enum class HttpStatusCode : int {
  OK = 200,
  Created = 201,
  BadRequest = 400,
  Unauthorized = 401,
  Forbidden = 403,
  NotFound = 404,
  InternalServerError = 500
};

// Specialization for HttpStatusCode with NUMBER encoding
template <> struct json::ENUM<HttpStatusCode> {
  static constexpr bool case_insensitive = false;
  static constexpr json::EnumEncoding encoding = json::EnumEncoding::NUMBER;
  static constexpr std::array names = std::array<std::string_view, 0>{};
};

// Enum 2: HTTP Status as STRING (human-readable names)
enum class HttpStatus {
  PENDING = 0,
  PROCESSING = 1,
  SUCCESS = 2,
  FAILED = 3
};

// Specialization for HttpStatus with STRING encoding
template <> struct json::ENUM<HttpStatus> {
  static constexpr bool case_insensitive = true;
  static constexpr json::EnumEncoding encoding = json::EnumEncoding::STRING;
  static constexpr std::array names = { "PENDING", "PROCESSING", "SUCCESS", "FAILED" };
};

// API Response struct using both enum types
struct ApiResponse {
  String message_;
  HttpStatusCode code_;
  HttpStatus status_;
};

template <> struct json::STRUCT<ApiResponse> {
  static constexpr auto properties = std::make_tuple(json::prop(&ApiResponse::message_, "message"),
   json::prop(&ApiResponse::code_, "code"),
   json::prop(&ApiResponse::status_, "status"));
};

void encode_decode_cbor() {
  try {
    std::cout << "\n=== CBOR Serialization Examples ===" << std::endl;

    // Test 1: Simple JSON object to CBOR
    std::cout << "\n1. Simple JSON to CBOR:" << std::endl;
    boost::json::value simple_json = { { "name", "Alice" }, { "age", 30 }, { "active", true } };
    std::vector<unsigned char> cbor_data;

    // Serialize to CBOR
    cbor::serialize_cbor_value(simple_json, cbor_data);
    std::cout << "CBOR bytes (" << cbor_data.size() << "): ";
    for (auto byte : cbor_data) {
      std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
    }
    std::cout << std::dec << std::endl;

    // Parse back from CBOR
    boost::json::value parsed_json;
    std::string_view sv(reinterpret_cast<const char*>(cbor_data.data()), cbor_data.size());
    cbor::parse_cbor_value(sv, parsed_json);
    std::cout << "Parsed JSON: " << parsed_json << std::endl;
    std::cout << "Match: " << (simple_json == parsed_json ? "Yes" : "No") << std::endl;

    // Test 2: Array of numbers
    std::cout << "\n2. Array of numbers to CBOR:" << std::endl;
    boost::json::value array_json = { 1, 2, 3, 4, 5 };
    cbor_data.clear();

    cbor::serialize_cbor_value(array_json, cbor_data);
    std::cout << "CBOR bytes (" << cbor_data.size() << "): ";
    for (auto byte : cbor_data) {
      std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
    }
    std::cout << std::dec << std::endl;

    parsed_json = nullptr;
    sv = std::string_view(reinterpret_cast<const char*>(cbor_data.data()), cbor_data.size());
    cbor::parse_cbor_value(sv, parsed_json);
    std::cout << "Parsed JSON: " << parsed_json << std::endl;
    std::cout << "Match: " << (array_json == parsed_json ? "Yes" : "No") << std::endl;

    // Test 3: Nested object
    std::cout << "\n3. Nested object to CBOR:" << std::endl;
    boost::json::value nested_json = { { "user",
                                        { { "name", "Bob" }, { "email", "bob@example.com" } } },
      { "score", 95.5 },
      { "tags", { "cpp", "json", "cbor" } } };
    cbor_data.clear();

    cbor::serialize_cbor_value(nested_json, cbor_data);
    std::cout << "CBOR bytes (" << cbor_data.size() << "): ";
    for (auto byte : cbor_data) {
      std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
    }
    std::cout << std::dec << std::endl;

    parsed_json = nullptr;
    sv = std::string_view(reinterpret_cast<const char*>(cbor_data.data()), cbor_data.size());
    cbor::parse_cbor_value(sv, parsed_json);
    std::cout << "Parsed JSON: " << parsed_json << std::endl;
    std::cout << "Match: " << (nested_json == parsed_json ? "Yes" : "No") << std::endl;

    // Test 4: Various data types
    std::cout << "\n4. Various data types to CBOR:" << std::endl;
    boost::json::value types_json = { { "null", nullptr },
      { "boolean", true },
      { "integer", 42 },
      { "negative", -100 },
      { "double", 3.14159 },
      { "string", "Hello CBOR!" },
      { "empty_array", boost::json::array() },
      { "empty_object", boost::json::object() } };
    cbor_data.clear();

    cbor::serialize_cbor_value(types_json, cbor_data);
    std::cout << "CBOR bytes (" << cbor_data.size() << "): ";
    for (auto byte : cbor_data) {
      std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
    }
    std::cout << std::dec << std::endl;

    parsed_json = nullptr;
    sv = std::string_view(reinterpret_cast<const char*>(cbor_data.data()), cbor_data.size());
    cbor::parse_cbor_value(sv, parsed_json);
    std::cout << "Parsed JSON: " << parsed_json << std::endl;
    std::cout << "Match: " << (types_json == parsed_json ? "Yes" : "No") << std::endl;

  } catch (const std::exception& e) {
    std::cout << "Error in encode_decode_cbor: " << e.what() << std::endl;
  }
}

void encode_decode_enum() {
  try {
    std::cout << "\n=== Enum Serialization Examples ===" << std::endl;

    // Example 1: HttpStatusCode (NUMBER encoding)
    std::cout << "\n1. HTTP Status Code (NUMBER):" << std::endl;
    ApiResponse response1;
    response1.message_ = "User created successfully";
    response1.code_ = HttpStatusCode::Created;
    response1.status_ = HttpStatus::SUCCESS;

    // Marshal to JSON
    String json_string1 = json::MarshalToString(response1);
    std::cout << "Encoded JSON: " << json_string1 << std::endl;

    // Unmarshal back
    ApiResponse response1_decoded;
    json::UnmarshalFromString(json_string1, response1_decoded);
    std::cout << "Decoded - Message: " << response1_decoded.message_ << ", Code: " << std::dec
              << static_cast<int>(response1_decoded.code_)
              << ", Status: " << static_cast<int>(response1_decoded.status_) << std::endl;

    // Example 2: Different status code (NOT FOUND)
    std::cout << "\n2. HTTP Status Code (NOT FOUND):" << std::endl;
    ApiResponse response2;
    response2.message_ = "Resource not found";
    response2.code_ = HttpStatusCode::NotFound;
    response2.status_ = HttpStatus::FAILED;

    String json_string2 = json::MarshalToString(response2);
    std::cout << "Encoded JSON: " << json_string2 << std::endl;

    ApiResponse response2_decoded;
    json::UnmarshalFromString(json_string2, response2_decoded);
    std::cout << "Decoded - Message: " << response2_decoded.message_ << ", Code: " << std::dec
              << static_cast<int>(response2_decoded.code_)
              << ", Status: " << static_cast<int>(response2_decoded.status_) << std::endl;

    // Example 3: Error response
    std::cout << "\n3. HTTP Status Code (INTERNAL SERVER ERROR):" << std::endl;
    ApiResponse response3;
    response3.message_ = "Internal server error occurred";
    response3.code_ = HttpStatusCode::InternalServerError;
    response3.status_ = HttpStatus::FAILED;

    String json_string3 = json::MarshalToString(response3);
    std::cout << "Encoded JSON: " << json_string3 << std::endl;

    ApiResponse response3_decoded;
    json::UnmarshalFromString(json_string3, response3_decoded);
    std::cout << "Decoded - Message: " << response3_decoded.message_ << ", Code: " << std::dec
              << static_cast<int>(response3_decoded.code_)
              << ", Status: " << static_cast<int>(response3_decoded.status_) << std::endl;

  } catch (const std::exception& e) {
    std::cout << "Error in encode_decode_enum: " << e.what() << std::endl;
  }
}

int main(int argc, char* argv[]) {
  std::cout << "hello world" << std::endl;

  struct Foo {};

  std::cout << "type_name<int>() == " << FNAME(int) << std::endl;
  std::cout << "type_name<std::vector<double>>() == " << FNAME(std::vector<double>) << std::endl;
  std::cout << "short_type_name<std::vector<double>>() == " << SNAME(std::vector<double>)
            << std::endl;
  std::cout << "type_name<Foo>() == " << FNAME(Foo) << std::endl;

  try {
    encode_decode_struct();
    encode_decode_uint64();
    encode_decode_int64();
    encode_decode_string();
    encode_decode_double();
    encode_decode_bool();
    encode_decode_vector();
    encode_decode_array();
    encode_decode_list();
    encode_decode_map();
    encode_decode_multimap();
    encode_decode_pair();
    encode_decode_optional();
    encode_decode_byte_array();
    encode_decode_byte_array_fixed();
    encode_decode_arbitrary();
    encode_decode_enum();
    encode_decode_cbor();
  } catch (const std::exception& e) {
    std::cerr << "Error in main: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
