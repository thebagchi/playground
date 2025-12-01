#ifndef JSON_H_INCLUDED
#define JSON_H_INCLUDED

#include <boost/json.hpp>
#include <boost/json/static_resource.hpp>
#include <boost/beast/core/detail/base64.hpp>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace base64 = boost::beast::detail::base64;

// =============================================================================
// Utility Macros for Smart Pointer Creation
// =============================================================================

/**
 * @brief Macro to create unique_ptr without angle brackets
 * Usage: MAKE_UNIQUE(Type, args...) instead of std::make_unique<Type>(args...)
 */
#define MAKE_UNIQUE(T, ...) std::make_unique<T>(__VA_ARGS__)

/**
 * @brief Macro to create shared_ptr without angle brackets
 * Usage: MAKE_SHARED(Type, args...) instead of std::make_shared<Type>(args...)
 */
#define MAKE_SHARED(T, ...) std::make_shared<T>(__VA_ARGS__)

// =============================================================================
// Type Traits
// =============================================================================

/**
 * @brief Trait to check if a type has a static properties member
 */
template <typename T, typename = void> struct has_properties : std::false_type {};

template <typename T>
struct has_properties<T, std::void_t<decltype(T::properties)>> : std::true_type {};

template <typename T> inline constexpr bool has_properties_v = has_properties<T>::value;

/**
 * @brief Trait to check if a type is optional (has operator bool)
 */
template <typename T> struct is_optional : std::false_type {};

template <typename T> struct is_optional<std::optional<T>> : std::true_type {};

template <typename T> struct is_optional<std::unique_ptr<T>> : std::true_type {};

template <typename T> struct is_optional<std::shared_ptr<T>> : std::true_type {};

template <typename T> inline constexpr bool is_optional_v = is_optional<T>::value;

/**
 * @brief Trait to get the value type from a optional type
 */
template <typename T, typename = void> struct opt {
  using type = T;
};

template <typename T> struct opt<std::optional<T>> {
  using type = T;
};

template <typename T> struct opt<std::unique_ptr<T>> {
  using type = T;
};

template <typename T> struct opt<std::shared_ptr<T>> {
  using type = T;
};

template <typename T> struct opt<const T> : opt<T> {};

template <typename T> using opt_t = typename opt<T>::type;

// =============================================================================
// JSON Serialization Library
// =============================================================================
// This library provides compile-time reflection-based JSON serialization
// for C++ objects using Boost.JSON.
//
// Performance optimizations applied:
// 1. Pre-allocate container sizes (reserve() calls) to avoid reallocations
// 2. Use inline functions with noexcept for better optimization
// 3. Avoid unnecessary copies through efficient memory management
// 4. Compile-time reflection minimizes runtime overhead
//
// Error Handling:
// - Type mismatches throw std::runtime_error with descriptive messages
// - Invalid JSON structures throw std::runtime_error
// - JSON parsing errors throw std::runtime_error with Boost error details
// - Missing required fields throw std::runtime_error
// - Null values for non-nullable fields throw std::runtime_error
//
// Usage:
//   struct MyStruct {
//     std::unique_ptr<std::string> name;
//     constexpr static auto properties = std::make_tuple(
//       prop(&MyStruct::name, "name")
//     );
//   };
//
//   MyStruct obj;
//   boost::json::value json = Write<MyStruct>{}(&obj);
// =============================================================================

// =============================================================================
// Property Reflection System
// =============================================================================

/**
 * @brief Property descriptor for compile-time reflection
 * @tparam C The class type containing the member
 * @tparam T The member type (wrapped in smart pointer)
 * @tparam Nullable Whether this field can be null
 * @tparam Required Whether this field is required
 */
template <typename C, typename T, bool Nullable = false, bool Required = false> struct Props {
  static_assert(!Nullable || is_optional_v<T>,
                "If Nullable is true, T must be a pointer or optional type");
  static_assert(!std::is_pointer_v<T>, "Naked pointers are not allowed as member types");

  /**
   * @brief Constructor for property descriptor
   * @param member Pointer to member variable
   * @param name JSON field name
   */
  constexpr Props(T C::*member, const char* name) : member_{ member }, name_{ name } {
    // Do Nothing ...
  }

  using Type = T;                             ///< The smart pointer type
  T C::*member_;                              ///< Pointer to member variable
  const char* name_;                          ///< JSON field name
  static constexpr bool nullable_ = Nullable; ///< Whether field can be null
  static constexpr bool required_ = Required; ///< Whether field is required
};

/**
 * @brief Helper function to create property descriptors
 * @tparam Nullable Whether this field can be null (default: false)
 * @tparam Required Whether this field is required (default: false)
 * @tparam C Class type (deduced)
 * @tparam T Member type (deduced)
 * @param member Pointer to member variable
 * @param name JSON field name
 * @return Props descriptor
 */
template <bool Nullable = false, bool Required = false, typename C, typename T>
constexpr Props<C, T, Nullable, Required> prop(T C::*member, const char* name) {
  return Props<C, T, Nullable, Required>{ member, name };
}

// =============================================================================
// Utility Functions
// =============================================================================

/**
 * @brief Compile-time for-each over integer sequence
 * @tparam T Integer type
 * @tparam S Sequence of integers
 * @tparam F Function to call for each index
 * @param seq Integer sequence
 * @param f Function to execute
 */
template <typename T, T... S, typename F>
constexpr void for_each(std::integer_sequence<T, S...>, F&& f) {
  (static_cast<void>(f(std::integral_constant<T, S>{})), ...);
}

/**
 * @brief Helper functions to throw field-related runtime errors
 */
[[noreturn]] inline void throw_field_null_not_nullable(const char* field_name) {
  std::string msg;
  const size_t name_len = std::strlen(field_name);
  msg.reserve(30 + name_len); // Pre-allocate exact space needed
  msg.append("Field '").append(field_name, name_len).append("' is null but not nullable");
  throw std::runtime_error(std::move(msg));
}

[[noreturn]] inline void throw_field_missing(const char* field_name) {
  std::string msg;
  const size_t name_len = std::strlen(field_name);
  msg.reserve(17 + name_len); // Pre-allocate exact space needed
  msg.append("Field '").append(field_name, name_len).append("' is missing");
  throw std::runtime_error(std::move(msg));
}

[[noreturn]] inline void throw_type_mismatch(const char* expected_type,
                                             const boost::json::value& value) {
  std::string msg;
  const size_t expected_len = std::strlen(expected_type);
  msg.reserve(40 + expected_len); // Pre-allocate exact space needed
  msg.append("Type mismatch: expected ")
      .append(expected_type, expected_len)
      .append(", got ")
      .append(boost::json::to_string(value.kind()));
  throw std::runtime_error(std::move(msg));
}

[[noreturn]] inline void throw_invalid_json_object() {
  throw std::runtime_error("Invalid JSON: expected object for struct deserialization");
}

[[noreturn]] inline void throw_json_parse_error(const std::string& error_msg) {
  std::string msg;
  msg.reserve(18 + error_msg.size());
  msg.append("JSON parse error: ").append(error_msg);
  throw std::runtime_error(std::move(msg));
}

// =============================================================================
// JSON Serialization Templates
// =============================================================================

/**
 * @brief Forward declaration of primary template for JSON serialization
 */
template <typename T> struct Write;

// =============================================================================
// Specializations for Basic Types
// =============================================================================

/**
 * @brief Specialization for std::string
 */
template <> struct Write<std::string> {
  inline void operator()(const std::string* object, boost::json::value* out) const noexcept {
    if (object) [[likely]] {
      *out = boost::json::string_view(*object);
    } else {
      *out = nullptr;
    }
  }
};

/**
 * @brief Specialization for const std::string
 */
template <> struct Write<const std::string> {
  inline void operator()(const std::string* object, boost::json::value* out) const noexcept {
    if (object) [[likely]] {
      *out = boost::json::string_view(*object);
    } else {
      *out = nullptr;
    }
  }
};

/**
 * @brief Specialization for int64_t
 */
template <> struct Write<std::int64_t> {
  inline void operator()(const std::int64_t* object, boost::json::value* out) const noexcept {
    if (object) [[likely]] {
      *out = *object;
    } else {
      *out = nullptr;
    }
  }
};

/**
 * @brief Specialization for const int64_t
 */
template <> struct Write<const std::int64_t> {
  inline void operator()(const std::int64_t* object, boost::json::value* out) const noexcept {
    if (object) [[likely]] {
      *out = *object;
    } else {
      *out = nullptr;
    }
  }
};

/**
 * @brief Specialization for uint64_t
 */
template <> struct Write<std::uint64_t> {
  inline void operator()(const std::uint64_t* object, boost::json::value* out) const noexcept {
    if (object) [[likely]] {
      *out = *object;
    } else {
      *out = nullptr;
    }
  }
};

/**
 * @brief Specialization for const uint64_t
 */
template <> struct Write<const std::uint64_t> {
  inline void operator()(const std::uint64_t* object, boost::json::value* out) const noexcept {
    if (object) [[likely]] {
      *out = *object;
    } else {
      *out = nullptr;
    }
  }
};

/**
 * @brief Specialization for double
 */
template <> struct Write<double> {
  inline void operator()(const double* object, boost::json::value* out) const noexcept {
    if (object) [[likely]] {
      *out = *object;
    } else {
      *out = nullptr;
    }
  }
};

/**
 * @brief Specialization for const double
 */
template <> struct Write<const double> {
  inline void operator()(const double* object, boost::json::value* out) const noexcept {
    if (object) [[likely]] {
      *out = *object;
    } else {
      *out = nullptr;
    }
  }
};

/**
 * @brief Specialization for bool
 */
template <> struct Write<bool> {
  inline void operator()(const bool* object, boost::json::value* out) const noexcept {
    if (object) [[likely]] {
      *out = *object;
    } else {
      *out = nullptr;
    }
  }
};

/**
 * @brief Specialization for const bool
 */
template <> struct Write<const bool> {
  inline void operator()(const bool* object, boost::json::value* out) const noexcept {
    if (object) [[likely]] {
      *out = *object;
    } else {
      *out = nullptr;
    }
  }
};

// =============================================================================
// Specializations for boost::json::value
// =============================================================================

/**
 * @brief Specialization for boost::json::value (pass-through)
 */
template <> struct Write<boost::json::value> {
  inline void operator()(const boost::json::value* object, boost::json::value* out) const noexcept {
    *out = *object; // Pass through as-is
  }
};

/**
 * @brief Specialization for const boost::json::value (pass-through)
 */
template <> struct Write<const boost::json::value> {
  inline void operator()(const boost::json::value* object, boost::json::value* out) const noexcept {
    *out = *object; // Pass through as-is
  }
};

// =============================================================================
// Primary Template Definition
// =============================================================================

/**
 * @brief Primary template for JSON serialization
 * @tparam T Type to serialize (must have static properties member)
 */
template <typename T> struct Write {
  /**
   * @brief Serialize object to JSON value
   * @param object Pointer to object to serialize
   * @param out Pointer to output JSON value
   */
  inline void operator()(const T* object, boost::json::value* out) const {
    static_assert(has_properties_v<T>, "T must have a static properties member for serialization");
    boost::json::object obj(out->storage());
    static constexpr auto props = std::tuple_size_v<decltype(T::properties)>;
    obj.reserve(props); // Pre-allocate space for better performance

    for_each(std::make_index_sequence<props>{}, [&](auto i) {
      constexpr auto property = std::get<i>(T::properties);
      using Type = typename decltype(property)::Type;
      const auto& ptr = object->*(property.member_);

      // Extract the underlying value type from optional containers (unique_ptr,
      // shared_ptr, optional) by removing const/reference qualifiers and
      // getting the contained type
      using M = std::remove_reference_t<decltype(ptr)>;
      using B = std::remove_const_t<M>;
      using P = opt_t<B>;
      if constexpr (is_optional_v<B>) {
        if (ptr) [[likely]] {
          boost::json::value temp(out->storage());
          Write<P>{}(&*ptr, &temp);
          obj.emplace(property.name_, std::move(temp));
        } else if (property.nullable_) [[likely]] {
          obj.emplace(property.name_, nullptr);
        } else {
          throw_field_null_not_nullable(property.name_);
        }
      } else {
        boost::json::value temp(out->storage());
        Write<P>{}(&ptr, &temp);
        obj.emplace(property.name_, std::move(temp));
      }
    });

    *out = std::move(obj);
  }
};

// =============================================================================
// Base64 Support for Binary Data
// =============================================================================

/**
 * @brief Specialization for std::vector<std::uint8_t> - serialize as Base64 string
 */
template <> struct Write<std::vector<std::uint8_t>> {
  inline void operator()(const std::vector<std::uint8_t>* data,
                         boost::json::value* out) const noexcept {
    if (!data || data->empty()) [[unlikely]] {
      *out = nullptr;
      return;
    }

    const std::size_t encoded_len = base64::encoded_size(data->size());

    // Construct json::string directly with correct storage and size
    auto& str = out->emplace_string();
    str.reserve(encoded_len); // Critical: pre-allocate exact size
    str.resize(encoded_len);  // Now safe to write into
    base64::encode(const_cast<char*>(str.data()), data->data(), data->size());
  }
};

// =============================================================================
// Specializations for Container Types
// =============================================================================

/**
 * @brief Specialization for std::vector<T>
 * @tparam T Element type (can be basic or complex)
 */
template <typename T> struct Write<std::vector<T>> {
  static_assert(!std::is_pointer_v<T>,
                "Raw pointers are not supported in std::vector for serialization. "
                "Use std::unique_ptr, std::shared_ptr, or std::optional instead.");

  inline void operator()(const std::vector<T>* object, boost::json::value* out) const {
    if (!object) [[unlikely]] {
      *out = nullptr;
      return;
    }

    boost::json::array arr(out->storage());
    arr.reserve(object->size());

    for (const auto& item : *object) {
      boost::json::value temp(out->storage());
      Write<T>{}(&item, &temp);
      arr.emplace_back(std::move(temp));
    }

    *out = std::move(arr);
  }
};

/**
 * @brief Specialization for std::map<std::string, T>
 * @tparam T Value type (can be basic or complex)
 */
template <typename T> struct Write<std::map<std::string, T>> {
  inline void operator()(const std::map<std::string, T>* object, boost::json::value* out) const {
    if (!object) [[unlikely]] {
      *out = nullptr;
      return;
    }

    boost::json::object obj(out->storage());
    obj.reserve(object->size()); // Pre-allocate space for better performance

    for (const auto& item : *object) {
      boost::json::value temp(out->storage());
      Write<T>{}(&item.second, &temp);
      obj.emplace(item.first, std::move(temp));
    }

    *out = std::move(obj);
  }
};

/**
 * @brief Specialization for std::unordered_map<std::string, T>
 * @tparam T Value type (can be basic or complex)
 */
template <typename T> struct Write<std::unordered_map<std::string, T>> {
  inline void operator()(const std::unordered_map<std::string, T>* object,
                         boost::json::value* out) const {
    if (!object) [[unlikely]] {
      *out = nullptr;
      return;
    }

    boost::json::object obj(out->storage());
    obj.reserve(object->size()); // Pre-allocate space for better performance

    for (const auto& item : *object) {
      boost::json::value temp(out->storage());
      Write<T>{}(&item.second, &temp);
      obj.emplace(item.first, std::move(temp));
    }

    *out = std::move(obj);
  }
};

// =============================================================================
// Specializations for Nullable Types
// =============================================================================

/**
 * @brief Specialization for std::unique_ptr<T>
 * @tparam T Pointed-to type
 */
template <typename T> struct Write<std::unique_ptr<T>> {
  inline void operator()(const std::unique_ptr<T>* object, boost::json::value* out) const noexcept {
    if (!object || !*object) [[unlikely]] {
      *out = nullptr;
    } else {
      Write<T>{}(object->get(), out);
    }
  }
};

/**
 * @brief Specialization for std::shared_ptr<T>
 * @tparam T Pointed-to type
 */
template <typename T> struct Write<std::shared_ptr<T>> {
  inline void operator()(const std::shared_ptr<T>* object, boost::json::value* out) const noexcept {
    if (!object || !*object) [[unlikely]] {
      *out = nullptr;
    } else {
      Write<T>{}(object->get(), out);
    }
  }
};

/**
 * @brief Specialization for std::optional<T>
 * @tparam T Contained type
 */
template <typename T> struct Write<std::optional<T>> {
  inline void operator()(const std::optional<T>* object, boost::json::value* out) const noexcept {
    if (!object || !*object) [[unlikely]] {
      *out = nullptr;
    } else {
      Write<T>{}(&**object, out);
    }
  }
};

// =============================================================================
// Marshal Function (Wrapper for Write)
// =============================================================================

/**
 * @brief Convenience function to marshal (serialize) an object to JSON with optional custom allocator
 * @tparam T Type to serialize
 * @param object Reference to the object to serialize
 * @param sp Storage pointer for custom memory allocation (optional)
 * @return JSON value representation
 */
template <typename T>
[[nodiscard]] boost::json::value Marshal(const T& object, boost::json::storage_ptr sp = {}) {
  boost::json::value result(sp.get() ? sp : boost::json::storage_ptr{});
  Write<T>{}(&object, &result);
  return result;
}

/**
 * @brief Convenience function to marshal (serialize) an object to JSON string
 * @tparam T Type to serialize
 * @param object Reference to the object to serialize
 * @param sp Storage pointer for custom memory allocation (optional)
 * @return JSON string representation
 */
template <typename T>
[[nodiscard]] std::string MarshalToString(const T& object, boost::json::storage_ptr sp = {}) {
  return boost::json::serialize(Marshal(object, sp));
}

// =============================================================================
// JSON Deserialization Templates
// =============================================================================

/**
 * @brief Primary template for JSON deserialization
 * @tparam T Type to deserialize (must have static properties member)
 */
template <typename T> struct Read {
  /**
   * @brief Deserialize JSON value to object
   * @param value JSON value to deserialize from
   * @param object Pointer to object to populate
   */
  inline void operator()(const boost::json::value& value, T* object) const {
    if (!value.is_object()) [[unlikely]] {
      throw_invalid_json_object();
    }

    const boost::json::object& obj = value.as_object();
    static constexpr auto props = std::tuple_size_v<decltype(T::properties)>;

    for_each(std::make_index_sequence<props>{}, [&](auto i) {
      constexpr auto property = std::get<i>(T::properties);
      using Type = typename decltype(property)::Type;

      // Check if JSON object contains this field
      auto it = obj.find(property.name_);
      if (it != obj.end()) [[likely]] {
        const auto& val = it->value();
        auto& ptr = object->*(property.member_);
        using M = std::remove_reference_t<decltype(ptr)>;
        using B = std::remove_const_t<M>;
        using P = opt_t<B>;

        if constexpr (is_optional_v<B>) {
          if (val.is_null()) [[unlikely]] {
            if (property.nullable_) [[likely]] {
              if constexpr (std::is_same_v<std::decay_t<decltype(ptr)>, std::optional<P>>) {
                ptr = std::nullopt;
              } else {
                ptr = nullptr;
              }
            } else [[unlikely]] {
              throw_field_null_not_nullable(property.name_);
            }
          } else [[likely]] {
            if constexpr (std::is_same_v<std::decay_t<decltype(ptr)>, std::optional<P>>) {
              P temp;
              Read<P>{}(val, &temp);
              ptr = std::move(temp);
            } else {
              ptr = std::make_unique<P>();
              Read<P>{}(val, ptr.get());
            }
          }
        } else {
          if (val.is_null()) [[unlikely]] {
            throw_field_null_not_nullable(property.name_);
          } else [[likely]] {
            Read<P>{}(val, &ptr);
          }
        }
      } else [[unlikely]] {
        // Field missing
        if (property.nullable_) [[likely]] {
          auto& ptr = object->*(property.member_);
          using M = std::remove_reference_t<decltype(ptr)>;
          using B = std::remove_const_t<M>;
          if constexpr (is_optional_v<B>) {
            using P = opt_t<B>;
            if constexpr (std::is_same_v<std::decay_t<decltype(ptr)>, std::optional<P>>) {
              ptr = std::nullopt;
            } else {
              ptr = nullptr;
            }
          }
          // No else: static_assert ensures nullable implies optional
        } else {
          throw_field_missing(property.name_);
        }
      }
    });
  }
};

// =============================================================================
// Specializations for Nullable Types (Read)
// =============================================================================

/**
 * @brief Specialization for std::unique_ptr<T>
 * @tparam T Pointed-to type
 */
template <typename T> struct Read<std::unique_ptr<T>> {
  inline void operator()(const boost::json::value& value, std::unique_ptr<T>* object) const {
    if (value.is_null()) [[unlikely]] {
      *object = nullptr;
    } else [[likely]] {
      *object = std::make_unique<T>();
      Read<T>{}(value, object->get());
    }
  }
};

/**
 * @brief Specialization for std::shared_ptr<T>
 * @tparam T Pointed-to type
 */
template <typename T> struct Read<std::shared_ptr<T>> {
  inline void operator()(const boost::json::value& value, std::shared_ptr<T>* object) const {
    if (value.is_null()) [[unlikely]] {
      *object = nullptr;
    } else [[likely]] {
      *object = std::make_shared<T>();
      Read<T>{}(value, object->get());
    }
  }
};

/**
 * @brief Specialization for std::optional<T>
 * @tparam T Contained type
 */
template <typename T> struct Read<std::optional<T>> {
  inline void operator()(const boost::json::value& value, std::optional<T>* object) const {
    if (value.is_null()) [[unlikely]] {
      *object = std::nullopt;
    } else [[likely]] {
      T temp;
      Read<T>{}(value, &temp);
      *object = std::move(temp);
    }
  }
};

// =============================================================================
// Specializations for Basic Types (Read)
// =============================================================================

/**
 * @brief Specialization for std::string
 */
template <> struct Read<std::string> {
  inline void operator()(const boost::json::value& value, std::string* object) const {
    if (!value.is_string()) [[unlikely]] {
      throw_type_mismatch("string", value);
    }
    const auto& str = value.as_string();
    object->reserve(str.size());
    *object = str;
  }
};

/**
 * @brief Specialization for int64_t
 */
template <> struct Read<std::int64_t> {
  inline void operator()(const boost::json::value& value, std::int64_t* object) const {
    if (value.is_int64()) [[likely]] {
      *object = value.as_int64();
    } else if (value.is_uint64()) [[unlikely]] {
      auto val = value.as_uint64();
      if (val <= static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
        *object = static_cast<std::int64_t>(val);
      } else {
        throw std::runtime_error("Value too large for int64_t");
      }
    } else {
      throw_type_mismatch("integer", value);
    }
  }
};

/**
 * @brief Specialization for uint64_t
 */
template <> struct Read<std::uint64_t> {
  inline void operator()(const boost::json::value& value, std::uint64_t* object) const {
    if (value.is_uint64()) [[likely]] {
      *object = value.as_uint64();
    } else if (value.is_int64()) [[unlikely]] {
      auto val = value.as_int64();
      if (val >= 0) {
        *object = static_cast<std::uint64_t>(val);
      } else {
        throw std::runtime_error("Negative value cannot be converted to uint64_t");
      }
    } else {
      throw_type_mismatch("unsigned integer", value);
    }
  }
};

/**
 * @brief Specialization for double
 */
template <> struct Read<double> {
  inline void operator()(const boost::json::value& value, double* object) const {
    if (value.is_double()) [[likely]] {
      *object = value.as_double();
    } else if (value.is_int64()) [[unlikely]] {
      *object = static_cast<double>(value.as_int64());
    } else if (value.is_uint64()) [[unlikely]] {
      *object = static_cast<double>(value.as_uint64());
    } else {
      throw_type_mismatch("number", value);
    }
  }
};

/**
 * @brief Specialization for bool
 */
template <> struct Read<bool> {
  inline void operator()(const boost::json::value& value, bool* object) const {
    if (value.is_bool()) [[likely]] {
      *object = value.as_bool();
    } else [[unlikely]] {
      throw_type_mismatch("boolean", value);
    }
  }
};

// =============================================================================
// Specializations for boost::json::value (Read)
// =============================================================================

/**
 * @brief Specialization for boost::json::value (pass-through)
 */
template <> struct Read<boost::json::value> {
  inline void operator()(const boost::json::value& value, boost::json::value* object) const {
    *object = value; // Pass through as-is
  }
};

// =============================================================================
// Base64 Support for Binary Data (Read)
// =============================================================================

/**
 * @brief Specialization for std::vector<std::uint8_t> - deserialize from Base64 string
 */
template <> struct Read<std::vector<std::uint8_t>> {
  inline void operator()(const boost::json::value& value, std::vector<std::uint8_t>* out) const {
    if (value.is_null()) {
      out->clear();
      return;
    }
    if (!value.is_string()) {
      throw_type_mismatch("base64 string", value);
    }
    const auto& str = value.as_string();
    const std::size_t max_decoded = base64::decoded_size(str.size());
    out->resize(max_decoded);
    auto [ptr, len] = base64::decode(out->data(), str.data(), str.size());
    if (len == 0 && !str.empty()) {
      throw std::runtime_error("Invalid base64");
    }
    out->resize(len);
  }
};

// =============================================================================
// Specializations for Container Types (Read)
// =============================================================================

/**
 * @brief Specialization for std::vector<T>
 * @tparam T Element type
 */
template <typename T> struct Read<std::vector<T>> {
  static_assert(!std::is_pointer_v<T>,
                "Raw pointers are not supported in std::vector for deserialization. "
                "Use std::unique_ptr, std::shared_ptr, or std::optional instead.");

  inline void operator()(const boost::json::value& value, std::vector<T>* object) const {
    if (value.is_null()) [[unlikely]] {
      object->clear();
    } else if (value.is_array()) [[likely]] {
      const boost::json::array& arr = value.as_array();
      object->clear();
      object->reserve(arr.size());

      for (const auto& item : arr) {
        object->emplace_back(); // Construct in-place for better performance
        Read<T>{}(item, &object->back());
      }
    } else {
      throw_type_mismatch("array", value);
    }
  }
};

/**
 * @brief Specialization for std::map<std::string, T>
 * @tparam T Value type
 */
template <typename T> struct Read<std::map<std::string, T>> {
  inline void operator()(const boost::json::value& value, std::map<std::string, T>* object) const {
    if (value.is_null()) [[unlikely]] {
      object->clear();
    } else if (value.is_object()) [[likely]] {
      const boost::json::object& obj = value.as_object();
      object->clear();
      // Note: std::map doesn't support reserve() as it's tree-based, not
      // contiguous

      for (const auto& item : obj) {
        T temp;
        Read<T>{}(item.value(), &temp);
        object->emplace(boost::json::string_view(item.key()), std::move(temp));
      }
    } else {
      throw_type_mismatch("object", value);
    }
  }
};

/**
 * @brief Specialization for std::unordered_map<std::string, T>
 * @tparam T Value type
 */
template <typename T> struct Read<std::unordered_map<std::string, T>> {
  inline void operator()(const boost::json::value& value,
                         std::unordered_map<std::string, T>* object) const {
    if (value.is_null()) [[unlikely]] {
      object->clear();
    } else if (value.is_object()) [[likely]] {
      const boost::json::object& obj = value.as_object();
      object->clear();
      object->reserve(obj.size()); // Pre-allocate space for better performance

      for (const auto& item : obj) {
        T temp;
        Read<T>{}(item.value(), &temp);
        object->emplace(boost::json::string_view(item.key()), std::move(temp));
      }
    } else {
      throw_type_mismatch("object", value);
    }
  }
};

// =============================================================================
// Unmarshal Function (Wrapper for Read)
// =============================================================================

/**
 * @brief Convenience function to unmarshal (deserialize) JSON to an object
 * @tparam T Type to deserialize
 * @param value JSON value to deserialize from
 * @param object Reference to the object to populate
 */
template <typename T> void Unmarshal(const boost::json::value& value, T& object) {
  Read<T>{}(value, &object);
}

/**
 * @brief Convenience function to unmarshal (deserialize) JSON string to an object
 * @tparam T Type to deserialize
 * @param json_string JSON string to deserialize from
 * @param object Reference to the object to populate
 * @param sp Storage pointer for custom memory allocation during parsing (optional)
 * @throws std::runtime_error if JSON parsing fails or type mismatches occur
 */
template <typename T>
void UnmarshalFromString(const std::string& json_string,
                         T& object,
                         boost::json::storage_ptr sp = {}) {
  boost::system::error_code ec;
  boost::json::value value =
      sp.get() ? boost::json::parse(json_string, ec, sp) : boost::json::parse(json_string, ec);
  if (ec) {
    throw_json_parse_error(ec.message());
  }
  Read<T>{}(value, &object);
}

#endif // JSON_H_INCLUDED