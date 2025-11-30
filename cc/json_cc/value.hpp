
#ifndef JSON_VALUE_V1_HPP_INCLUDED
#define JSON_VALUE_V1_HPP_INCLUDED

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>

namespace json {
// Forward declaration
class Value;
template <class T>
using Ptr = std::shared_ptr<T>;

template <class T>
struct is_ptr : std::false_type {};
template <class T>
struct is_ptr<Ptr<T>> : std::true_type {};

template <class T>
using Vector = std::vector<T>;

template <class K, class V>
using Map = std::map<K, V>;

using Void = std::void_t<>;
using VoidPtr = Ptr<Void>;
using Null = std::nullptr_t;
using NullPtr = Ptr<Null>;
using Bool = bool;
using BoolPtr = Ptr<Bool>;
using Number = double;
using NumberPtr = Ptr<Number>;
using String = std::string;
using StringView = std::string_view;
using StringPtr = Ptr<String>;
using ValuePtr = Ptr<Value>;
using List = std::vector<ValuePtr>;
using ListPtr = Ptr<List>;
using Dict = std::map<std::string, ValuePtr>;
using DictPtr = Ptr<Dict>;
using Variant =
    std::variant<NullPtr, BoolPtr, NumberPtr, StringPtr, ListPtr, DictPtr>;

// Type aliases for convenience
using SI8 = std::int8_t;
using SI16 = std::int16_t;
using SI32 = std::int32_t;
using SI64 = std::int64_t;
using UI8 = std::uint8_t;
using UI16 = std::uint16_t;
using UI32 = std::uint32_t;
using UI64 = std::uint64_t;
using F32 = float;
using F64 = double;
using ListForEachFunc = std::function<void(const ValuePtr&)>;
using DictForEachFunc =
    std::function<void(const std::string&, const ValuePtr&)>;

enum class Type { E_NULL, E_BOOL, E_NUMBER, E_STRING, E_LIST, E_DICT };

// Convert Type enum to string for printing
inline const char* type_to_string(Type type) {
  switch (type) {
    case Type::E_NULL:
      return "E_NULL";
    case Type::E_BOOL:
      return "E_BOOL";
    case Type::E_NUMBER:
      return "E_NUMBER";
    case Type::E_STRING:
      return "E_STRING";
    case Type::E_LIST:
      return "E_LIST";
    case Type::E_DICT:
      return "E_DICT";
    default:
      return "UNKNOWN";
  }
}

class Value final {
 public:
  // Rule of 5 constructors
  Value() = default;
  Value(const Value&) = default;
  Value(Value&&) = default;
  Value& operator=(const Value&) = default;
  Value& operator=(Value&&) = default;
  ~Value() = default;

  // Constructor from Bool
  template <typename T, typename = std::enable_if_t<std::is_same_v<T, Bool>>>
  explicit Value(T value) : data_(std::make_shared<Bool>(value)) {
    // Constructor from Bool
  }

  // Constructor from Null
  explicit Value(std::nullptr_t value) : data_(std::make_shared<Null>(value)) {
    // Constructor from Null
  }

  // Constructor from Number
  explicit Value(Number value) : data_(std::make_shared<Number>(value)) {
    // Constructor from Number
  }

  // Constructor from String
  explicit Value(const String& value) : data_(std::make_shared<String>(value)) {
    // Constructor from String
  }

  // Constructor from C-string
  explicit Value(const char* value) : data_(std::make_shared<String>(value)) {
    // Constructor from C-string
  }

  // Constructor from StringView
  explicit Value(StringView value) : data_(std::make_shared<String>(value)) {
    // Constructor from StringView
  }

  // Constructor from List
  explicit Value(const List& value) : data_(std::make_shared<List>(value)) {
    // Constructor from List
  }

  // Constructor from List (move)
  explicit Value(List&& value)
      : data_(std::make_shared<List>(std::move(value))) {
    // Constructor from List (move)
  }

  // Constructor from Dict
  explicit Value(const Dict& value) : data_(std::make_shared<Dict>(value)) {
    // Constructor from Dict
  }

  // Constructor from Dict (move)
  explicit Value(Dict&& value)
      : data_(std::make_shared<Dict>(std::move(value))) {
    // Constructor from Dict (move)
  }

  Type type() const {
    // Get the type of the stored value
    switch (data_.index()) {
      case 0:
        return Type::E_NULL;
      case 1:
        return Type::E_BOOL;
      case 2:
        return Type::E_NUMBER;
      case 3:
        return Type::E_STRING;
      case 4:
        return Type::E_LIST;
      case 5:
        return Type::E_DICT;
      default:
        return Type::E_NULL;  // Default to null for invalid states
    }
  }

  const char* type_name() const {
    // Get string representation of the type
    return type_to_string(type());
  }

  // Templated function to get Number value as different types
  template <typename T>
  T as_number() const {
    if (type() != Type::E_NUMBER) {
      throw std::runtime_error("Value is not a number");
    }
    return static_cast<T>(*std::get<NumberPtr>(data_));
  }

  // Helper functions for specific numeric types
  SI8 as_si8() const {
    // Convert to signed 8-bit integer
    return as_number<SI8>();
  }
  SI16 as_si16() const {
    // Convert to signed 16-bit integer
    return as_number<SI16>();
  }
  SI32 as_si32() const {
    // Convert to signed 32-bit integer
    return as_number<SI32>();
  }
  SI64 as_si64() const {
    // Convert to signed 64-bit integer
    return as_number<SI64>();
  }
  UI8 as_ui8() const {
    // Convert to unsigned 8-bit integer
    return as_number<UI8>();
  }
  UI16 as_ui16() const {
    // Convert to unsigned 16-bit integer
    return as_number<UI16>();
  }
  UI32 as_ui32() const {
    // Convert to unsigned 32-bit integer
    return as_number<UI32>();
  }
  UI64 as_ui64() const {
    // Convert to unsigned 64-bit integer
    return as_number<UI64>();
  }
  F32 as_f32() const {
    // Convert to float
    return as_number<F32>();
  }
  F64 as_f64() const {
    // Convert to double
    return as_number<F64>();
  }

  // Function to get value as bool (works for E_BOOL and E_NUMBER)
  bool as_bool() const {
    if (type() == Type::E_BOOL) {
      return *std::get<BoolPtr>(data_);
    } else if (type() == Type::E_NUMBER) {
      return *std::get<NumberPtr>(data_) != 0.0;
    } else {
      throw std::runtime_error("Value cannot be converted to bool");
    }
  }

  // Function to get value as string (works for E_STRING)
  const String& as_string() const {
    if (type() != Type::E_STRING) {
      throw std::runtime_error("Value is not a string");
    }
    return *std::get<StringPtr>(data_);
  }

  // Function to get value as list (works for E_LIST)
  const List& as_list() const {
    if (type() != Type::E_LIST) {
      throw std::runtime_error("Value is not a list");
    }
    return *std::get<ListPtr>(data_);
  }

  // Helper method to iterate through list elements with a lambda
  void for_each(ListForEachFunc func) const {
    if (type() != Type::E_LIST) {
      throw std::runtime_error("Value is not a list");
    }
    const auto& list = *std::get<ListPtr>(data_);
    for (const auto& item : list) {
      func(item);
    }
  }

  // Function to get value as dict (works for E_DICT)
  const Dict& as_dict() const {
    if (type() != Type::E_DICT) {
      throw std::runtime_error("Value is not a dict");
    }
    return *std::get<DictPtr>(data_);
  }

  // Mutable accessor methods
  String& mutable_string() {
    if (type() != Type::E_STRING) {
      throw std::runtime_error("Value is not a string");
    }
    return *std::get<StringPtr>(data_);
  }

  List& mutable_list() {
    if (type() != Type::E_LIST) {
      throw std::runtime_error("Value is not a list");
    }
    return *std::get<ListPtr>(data_);
  }

  Dict& mutable_dict() {
    if (type() != Type::E_DICT) {
      throw std::runtime_error("Value is not a dict");
    }
    return *std::get<DictPtr>(data_);
  }

  // Setter methods to change the value
  void set_bool(bool value) {
    // Set the value to a boolean
    data_ = std::make_shared<Bool>(value);
  }

  void set_number(Number value) {
    // Set the value to a number
    data_ = std::make_shared<Number>(value);
  }

  void set_string(const String& value) {
    // Set the value to a string (copy)
    data_ = std::make_shared<String>(value);
  }

  void set_string(String&& value) {
    // Set the value to a string (move)
    data_ = std::make_shared<String>(std::move(value));
  }

  void set_list(const List& value) {
    // Set the value to a list (copy)
    data_ = std::make_shared<List>(value);
  }

  void set_list(List&& value) {
    // Set the value to a list (move)
    data_ = std::make_shared<List>(std::move(value));
  }

  void set_dict(const Dict& value) {
    // Set the value to a dictionary (copy)
    data_ = std::make_shared<Dict>(value);
  }

  void set_dict(Dict&& value) {
    // Set the value to a dictionary (move)
    data_ = std::make_shared<Dict>(std::move(value));
  }

  void set_null() {
    // Set the value to null
    data_ = std::make_shared<Null>(nullptr);
  }

  // Helper method to iterate through dict key-value pairs with a lambda
  void for_each(DictForEachFunc func) const {
    if (type() != Type::E_DICT) {
      throw std::runtime_error("Value is not a dict");
    }
    const auto& dict = *std::get<DictPtr>(data_);
    for (const auto& pair : dict) {
      func(pair.first, pair.second);
    }
  }

  // Convert the value to a JSON string representation
  String DebugString() const {
    switch (type()) {
      case Type::E_NULL:
        return "null";
      case Type::E_BOOL:
        return *std::get<BoolPtr>(data_) ? "true" : "false";
      case Type::E_NUMBER: {
        // Convert number to string with reasonable precision
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%.17g", *std::get<NumberPtr>(data_));
        return buf;
      }
      case Type::E_STRING: {
        const auto& str = *std::get<StringPtr>(data_);
        String result = "\"";
        for (char c : str) {
          switch (c) {
            case '"':
              result += "\\\"";
              break;
            case '\\':
              result += "\\\\";
              break;
            case '\b':
              result += "\\b";
              break;
            case '\f':
              result += "\\f";
              break;
            case '\n':
              result += "\\n";
              break;
            case '\r':
              result += "\\r";
              break;
            case '\t':
              result += "\\t";
              break;
            default:
              if (c < 32) {
                char esc[7];
                std::snprintf(esc, sizeof(esc), "\\u%04x",
                              static_cast<unsigned char>(c));
                result += esc;
              } else {
                result += c;
              }
              break;
          }
        }
        result += "\"";
        return result;
      }
      case Type::E_LIST: {
        const auto& list = *std::get<ListPtr>(data_);
        if (list.empty()) return "[]";

        String result = "[";
        for (size_t i = 0; i < list.size(); ++i) {
          if (i > 0) result += ",";
          result += list[i]->DebugString();
        }
        result += "]";
        return result;
      }
      case Type::E_DICT: {
        const auto& dict = *std::get<DictPtr>(data_);
        if (dict.empty()) return "{}";

        String result = "{";
        bool first = true;
        for (const auto& pair : dict) {
          if (!first) result += ",";
          first = false;
          result += "\"" + pair.first + "\":";
          result += pair.second->DebugString();
        }
        result += "}";
        return result;
      }
      default:
        return "null";
    }
  }

 private:
  Variant data_;
};

// Helper function to create ValuePtr
template <typename T>
ValuePtr MakePtr(T v) {
  return std::make_shared<Value>(Value(v));
}

}  // namespace json

#endif  // JSON_VALUE_V1_HPP_INCLUDED