#ifndef JSON_H_INCLUDED
#define JSON_H_INCLUDED

#include <boost/json.hpp>
#include <boost/json/static_resource.hpp>
#include <boost/beast/core/detail/base64.hpp>
#include <array>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include "utils.h"

namespace base64 = boost::beast::detail::base64;

namespace json {

  // =============================================================================
  // JSON Serialization Library
  // =============================================================================
  // This library provides compile-time reflection-based JSON serialization
  // for C++ objects using Boost.JSON.
  //
  // Supported Types:
  // - Basic types: String, Int64, UInt64, Double, Bool
  // - Containers: Vector, List, Map, MultiMap, Dict, MultiDict, Array, Pair
  // - Smart pointers: unique_ptr, shared_ptr, optional
  // - Binary data: ByteVector, ByteArray (Base64 encoded)
  // - Custom structs with properties
  // - Enums (with user-provided Write/Read specializations)
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
  //     UniquePtr<String> name;
  //     UniquePtr<String> email;
  //   };
  //
  //   template<> struct STRUCT(MyStruct) {
  //     static constexpr auto properties = std::make_tuple(
  //       prop(&MyStruct::name, "name", REQUIRED),
  //       prop(&MyStruct::email, "email", NULLABLE)
  //     );
  //   };
  //
  //   MyStruct obj;
  //   Value json = Marshal(obj);
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
    constexpr Props(T C::*member, std::string_view name) : member_(member), name_(name) {
      // Do Nothing ...
    }

    using Type = T;                             ///< The smart pointer type
    T C::*member_;                              ///< Pointer to member variable
    std::string_view name_;                     ///< JSON field name
    static constexpr bool nullable_ = Nullable; ///< Whether field can be null
    static constexpr bool required_ = Required; ///< Whether field is required
  };

  // Convenience tag types for property options
  struct nullable_tag {};
  struct required_tag {};

  static constexpr nullable_tag NULLABLE{};
  static constexpr required_tag REQUIRED{};

  /**
 * @brief Helper function to create property descriptors
 * @tparam C Class type (deduced)
 * @tparam T Member type (deduced)
 * @param member Pointer to member variable
 * @param name JSON field name
 * @return Props descriptor (optional field)
 */
  template <typename C, typename T>
  constexpr Props<C, T, false, false> prop(T C::*member, std::string_view name) {
    return Props<C, T, false, false>{ member, name };
  }

  /**
 * @brief Helper function to create nullable property descriptors
 * @tparam C Class type (deduced)
 * @tparam T Member type (deduced)
 * @param member Pointer to member variable
 * @param name JSON field name
 * @param tag Nullable tag
 * @return Props descriptor (nullable field)
 */
  template <typename C, typename T>
  constexpr Props<C, T, true, false> prop(T C::*member, std::string_view name, nullable_tag) {
    return Props<C, T, true, false>{ member, name };
  }

  /**
 * @brief Helper function to create required property descriptors
 * @tparam C Class type (deduced)
 * @tparam T Member type (deduced)
 * @param member Pointer to member variable
 * @param name JSON field name
 * @param tag Required tag
 * @return Props descriptor (required field)
 */
  template <typename C, typename T>
  constexpr Props<C, T, false, true> prop(T C::*member, std::string_view name, required_tag) {
    return Props<C, T, false, true>{ member, name };
  }

  /**
 * @brief Helper function to create nullable and required property descriptors
 * @tparam C Class type (deduced)
 * @tparam T Member type (deduced)
 * @param member Pointer to member variable
 * @param name JSON field name
 * @param nullable_tag Nullable tag
 * @param required_tag Required tag
 * @return Props descriptor (nullable and required field)
 */
  template <typename C, typename T>
  constexpr Props<C, T, true, true>
  prop(T C::*member, std::string_view name, nullable_tag, required_tag) {
    return Props<C, T, true, true>{ member, name };
  }

  /**
 * @brief STRUCT helper for defining JSON properties for structs/classes
 * Provides a cleaner syntax for defining struct properties using a builder pattern
 *
 * Example usage:
 *   struct MyStruct {
 *     UniquePtr<String> name;
 *     UniquePtr<int> age;
 *   };
 *
 *   template<> struct STRUCT(MyStruct) {
 *     static constexpr auto properties = std::make_tuple(
 *       prop(&MyStruct::name, "name", REQUIRED),
 *       prop(&MyStruct::email, "email", NULLABLE)
 *     );
 *   };
 */

  /**
 * @brief Base template for STRUCT property definitions
 * @tparam T The struct/class type
 */
  template <typename T> struct STRUCT {
    // Default empty properties tuple - should be specialized
    static constexpr auto properties = std::make_tuple();
  };

  /**
 * @brief Trait to check if STRUCT<T> has properties
 */
  template <typename T, typename = void> struct has_struct_properties : std::false_type {};

  template <typename T>
  struct has_struct_properties<T, std::void_t<decltype(STRUCT<T>::properties)>> : std::true_type {};

  template <typename T>
  inline constexpr bool has_struct_properties_v = has_struct_properties<T>::value;

  /**
 * @brief Get properties tuple from STRUCT<T>::properties
 */
  template <typename T> constexpr auto get_properties_tuple() {
    return STRUCT<T>::properties;
  }

  // =============================================================================
  // Enum Support
  // =============================================================================

  enum class EnumEncoding {
    STRING,
    NUMBER
  };

  /**
 * @brief Metadata descriptor for enum types
 * @tparam T The enum type
 */
  template <typename T> struct ENUM {
    static constexpr EnumEncoding encoding = EnumEncoding::NUMBER;
    static constexpr bool case_insensitive = false;
    static constexpr std::array<std::string_view, 0> names{};
  };

  // -----------------------------------------------------------------------------
  // Validation — fires at compile time if mismatch
  // -----------------------------------------------------------------------------
  template <typename Enum> struct enum_validator {
    static constexpr std::size_t name_count = std::size(ENUM<Enum>::names);
    static constexpr bool value = true;

    static_assert((ENUM<Enum>::encoding == EnumEncoding::STRING && name_count > 0) ||
                      (ENUM<Enum>::encoding == EnumEncoding::NUMBER && name_count == 0),
                  "ENUM<T> validation failed: for STRING encoding, names array must not be empty; "
                  "for NUMBER encoding, names array must be empty");
  };

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
  [[noreturn]] inline void throw_field_null_not_nullable(std::string_view field_name) {
    String msg;
    msg.reserve(30 + field_name.size()); // Pre-allocate exact space needed
    msg.append("Field '").append(field_name).append("' is null but not nullable");
    throw std::runtime_error(std::move(msg));
  }

  [[noreturn]] inline void throw_field_missing(std::string_view field_name) {
    String msg;
    msg.reserve(17 + field_name.size()); // Pre-allocate exact space needed
    msg.append("Field '").append(field_name).append("' is missing");
    throw std::runtime_error(std::move(msg));
  }

  [[noreturn]] inline void throw_type_mismatch(const char* expected_type,
                                               const boost::json::value& value) {
    String msg;
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

  [[noreturn]] inline void throw_json_parse_error(const String& error_msg) {
    String msg;
    msg.reserve(18 + error_msg.size());
    msg.append("JSON parse error: ").append(error_msg);
    throw std::runtime_error(std::move(msg));
  }

  [[noreturn]] inline void throw_invalid_base64() {
    throw std::runtime_error("Invalid base64");
  }

  [[noreturn]] inline void throw_byte_array_null() {
    throw std::runtime_error("Cannot deserialize null value into ByteArray - array size is fixed");
  }

  [[noreturn]] inline void throw_byte_array_size_mismatch(std::size_t decoded_size,
                                                          std::size_t expected_size) {
    String msg;
    msg.reserve(60); // Pre-allocate enough space
    msg.append("ByteArray size mismatch: decoded ")
        .append(std::to_string(decoded_size))
        .append(" bytes, but ByteArray expects ")
        .append(std::to_string(expected_size))
        .append(" bytes");
    throw std::runtime_error(std::move(msg));
  }

  [[noreturn]] inline void throw_value_too_large_for_int64() {
    throw std::runtime_error("Value too large for int64_t");
  }

  [[noreturn]] inline void throw_negative_value_for_uint64() {
    throw std::runtime_error("Negative value cannot be converted to uint64_t");
  }

  [[noreturn]] inline void throw_array_null() {
    throw std::runtime_error("Cannot deserialize null value into Array - array size is fixed");
  }

  [[noreturn]] inline void throw_array_size_mismatch(std::size_t json_size,
                                                     std::size_t expected_size) {
    String msg;
    msg.reserve(80); // Pre-allocate enough space
    msg.append("Array size mismatch: JSON array has ")
        .append(std::to_string(json_size))
        .append(" elements, but Array expects ")
        .append(std::to_string(expected_size))
        .append(" elements");
    throw std::runtime_error(std::move(msg));
  }

  // =============================================================================
  // Detail: Enum Support Helpers
  // =============================================================================

  namespace detail {
    /**
     * @brief Bubble sort implementation for compile-time string sorting
     * @tparam N Size of array
     * @param arr Array of string_view to sort in-place
     */
    template <std::size_t N> constexpr void sort_strings(std::array<std::string_view, N>& arr) {
      for (std::size_t i = 0; i < N; ++i) {
        for (std::size_t j = i + 1; j < N; ++j) {
          if (arr[i] > arr[j]) {
            std::swap(arr[i], arr[j]);
          }
        }
      }
    }

    /**
     * @brief Create and sort a copy of enum names at compile time
     * @tparam Enum The enum type
     * @return Sorted array of enum name string views
     */
    template <typename Enum> constexpr auto sorted_names() {
      auto names = ENUM<Enum>::names;
      sort_strings(names);
      return names;
    }

    /**
     * @brief Convert character to lowercase (for case-insensitive comparison)
     */
    constexpr char to_lower(char c) noexcept {
      return (c >= 'A' && c <= 'Z') ? char(c + 32) : c;
    }

    /**
     * @brief Case-insensitive string comparison
     */
    constexpr bool case_insensitive_equal(std::string_view a, std::string_view b) noexcept {
      if (a.size() != b.size()) {
        return false;
      }
      for (std::size_t i = 0; i < a.size(); ++i) {
        if (to_lower(a[i]) != to_lower(b[i])) {
          return false;
        }
      }
      return true;
    }

    // -----------------------------------------------------------------------------
    // Build sorted array from array
    // -----------------------------------------------------------------------------
    template <typename Enum> constexpr auto make_sorted_names() {
      if constexpr (ENUM<Enum>::encoding == EnumEncoding::NUMBER) {
        return std::array<std::string_view, 0>{};
      } else {
        constexpr auto& arr = ENUM<Enum>::names;
        constexpr std::size_t N = std::size(arr);
        std::array<std::string_view, N> sorted_arr{};
        for (std::size_t i = 0; i < N; ++i) {
          sorted_arr[i] = arr[i];
        }
        // Bubble sort — N is tiny, compile-time only (manual swap to avoid std::swap)
        for (std::size_t i = 0; i < N; ++i) {
          for (std::size_t j = i + 1; j < N; ++j) {
            if (sorted_arr[i] > sorted_arr[j]) {
              auto temp = sorted_arr[i];
              sorted_arr[i] = sorted_arr[j];
              sorted_arr[j] = temp;
            }
          }
        }
        return sorted_arr;
      }
    }

    template <typename Enum> constexpr auto sorted_names_v = make_sorted_names<Enum>();

    // Case-insensitive comparison
    constexpr bool iequal(std::string_view a, std::string_view b) {
      if (a.size() != b.size()) {
        return false;
      }
      for (std::size_t i = 0; i < a.size(); ++i) {
        if (to_lower(a[i]) != to_lower(b[i])) {
          return false;
        }
      }
      return true;
    }

  } // namespace detail

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
 * @brief Specialization for String
 */
  template <> struct Write<String> {
    inline void operator()(const String* object, Value* out) const noexcept {
      if (object) [[likely]] {
        *out = boost::json::string_view(*object);
      } else {
        *out = nullptr;
      }
    }
  };

  /**
 * @brief Specialization for const String
 */
  template <> struct Write<const String> {
    inline void operator()(const String* object, Value* out) const noexcept {
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
  template <> struct Write<Int64> {
    inline void operator()(const Int64* object, Value* out) const noexcept {
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
  template <> struct Write<const Int64> {
    inline void operator()(const Int64* object, Value* out) const noexcept {
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
  template <> struct Write<UInt64> {
    inline void operator()(const UInt64* object, Value* out) const noexcept {
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
  template <> struct Write<const UInt64> {
    inline void operator()(const UInt64* object, Value* out) const noexcept {
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
  template <> struct Write<Double> {
    inline void operator()(const Double* object, Value* out) const noexcept {
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
  template <> struct Write<const Double> {
    inline void operator()(const Double* object, Value* out) const noexcept {
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
  template <> struct Write<Bool> {
    inline void operator()(const Bool* object, Value* out) const noexcept {
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
  template <> struct Write<const Bool> {
    inline void operator()(const Bool* object, Value* out) const noexcept {
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
  template <> struct Write<Value> {
    inline void operator()(const Value* object, Value* out) const noexcept {
      *out = *object; // Pass through as-is
    }
  };

  /**
 * @brief Specialization for const boost::json::value (pass-through)
 */
  template <> struct Write<const Value> {
    inline void operator()(const Value* object, Value* out) const noexcept {
      *out = *object; // Pass through as-is
    }
  };

  // =============================================================================
  // Generic Enum Handler (for any enum type)
  // =============================================================================
  // This specialization automatically handles any enum type using the ENUM<T> metadata

  namespace detail {
    template <typename T, bool IsEnum = std::is_enum_v<T>> struct WriteEnum;

    // Specialization for enums
    template <typename T> struct WriteEnum<T, true> {
      static_assert(enum_validator<T>::value, "Invalid ENUM configuration");

      inline void operator()(const T* value, boost::json::value* out) const noexcept {
        if (!value) {
          *out = nullptr;
          return;
        }

        if constexpr (ENUM<T>::encoding == EnumEncoding::NUMBER) {
          // Serialize as underlying integer value
          *out = static_cast<std::underlying_type_t<T>>(*value);
        } else {
          // Serialize as string name
          constexpr auto& names = ENUM<T>::names;
          *out = std::string(names[static_cast<std::size_t>(*value)]);
        }
      }
    };

    // Non-specialization for non-enums (should not be called)
    template <typename T> struct WriteEnum<T, false> {
      inline void operator()(const T*, boost::json::value*) const {
        static_assert(false, "WriteEnum should only be used for enum types");
      }
    };

    template <typename T, bool IsStruct = has_struct_properties_v<T>> struct WriteStruct;

    // Specialization for structs
    template <typename T> struct WriteStruct<T, true> {
      inline void operator()(const T* object, boost::json::value* out) const {
        boost::json::object obj(out->storage());
        constexpr auto props_tuple = get_properties_tuple<T>();
        static constexpr auto props = std::tuple_size_v<decltype(props_tuple)>;
        obj.reserve(props); // Pre-allocate space for better performance

        for_each(std::make_index_sequence<props>{}, [&](auto i) {
          constexpr auto property = std::get<i>(props_tuple);
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
              ::json::Write<P>{}(&*ptr, &temp);
              obj.emplace(property.name_, std::move(temp));
            } else if (property.nullable_) [[likely]] {
              obj.emplace(property.name_, nullptr);
            } else {
              throw_field_null_not_nullable(property.name_);
            }
          } else {
            boost::json::value temp(out->storage());
            ::json::Write<P>{}(&ptr, &temp);
            obj.emplace(property.name_, std::move(temp));
          }
        });

        *out = std::move(obj);
      }
    };

    // Non-specialization for non-structs
    template <typename T> struct WriteStruct<T, false> {
      inline void operator()(const T*, boost::json::value*) const {
        static_assert(has_struct_properties_v<T>,
                      "T must have STRUCT<T> specialization for serialization");
      }
    };
  } // namespace detail

  // =============================================================================
  // Primary Template Definition
  // =============================================================================

  /**
 * @brief Primary template for JSON serialization
 * @tparam T Type to serialize (must have static properties member for structs, or be an enum)
 */
  template <typename T> struct Write {
    /**
   * @brief Serialize object to JSON value
   * @param object Pointer to object to serialize
   * @param out Pointer to output JSON value
   */
    inline void operator()(const T* object, boost::json::value* out) const {
      if constexpr (std::is_enum_v<T>) {
        // Delegate to enum handler
        detail::WriteEnum<T>{}(object, out);
      } else {
        // Delegate to struct handler
        detail::WriteStruct<T>{}(object, out);
      }
    }
  };

  // =============================================================================
  // Generic Enum Handler (for any enum type)
  // =============================================================================
  // This specialization automatically handles any enum type using the ENUM<T> metadata

  // =============================================================================
  // Base64 Support for Binary Data
  // =============================================================================

  /**
 * @brief Specialization for ByteVector - serialize as Base64 string
 */
  template <> struct Write<ByteVector> {
    inline void operator()(const ByteVector* data, Value* out) const noexcept {
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

  /**
 * @brief Specialization for ByteArray<N> - serialize as Base64 string
 * @tparam N Array size (known at compile time)
 */
  template <std::size_t N> struct Write<ByteArray<N>> {
    inline void operator()(const ByteArray<N>* data, Value* out) const noexcept {
      if (!data) [[unlikely]] {
        *out = nullptr;
        return;
      }

      const std::size_t encoded_len = base64::encoded_size(N);

      // Construct json::string directly with correct storage and size
      auto& str = out->emplace_string();
      str.reserve(encoded_len); // Critical: pre-allocate exact size
      str.resize(encoded_len);  // Now safe to write into
      base64::encode(const_cast<char*>(str.data()), data->data(), N);
    }
  };

  // =============================================================================
  // Specializations for Container Types
  // =============================================================================

  /**
 * @brief Specialization for std::vector<T>
 * @tparam T Element type (can be basic or complex)
 */
  template <typename T> struct Write<Vector<T>> {
    static_assert(!std::is_pointer_v<T>,
                  "Raw pointers are not supported in std::vector for serialization. "
                  "Use std::unique_ptr, std::shared_ptr, or std::optional instead.");

    inline void operator()(const Vector<T>* object, Value* out) const {
      if (!object) [[unlikely]] {
        *out = nullptr;
        return;
      }

      boost::json::array arr(out->storage());

      if (!object->empty()) [[likely]] {
        arr.reserve(object->size());

        for (const auto& item : *object) {
          Value temp(out->storage());
          Write<T>{}(&item, &temp);
          arr.emplace_back(std::move(temp));
        }
      }

      *out = std::move(arr);
    }
  };

  /**
 * @brief Specialization for List<T>
 * @tparam T Element type (can be basic or complex)
 */
  template <typename T> struct Write<List<T>> {
    static_assert(!std::is_pointer_v<T>,
                  "Raw pointers are not supported in List for serialization. "
                  "Use std::unique_ptr, std::shared_ptr, or std::optional instead.");

    inline void operator()(const List<T>* object, Value* out) const {
      if (!object) [[unlikely]] {
        *out = nullptr;
        return;
      }

      boost::json::array arr(out->storage());

      if (!object->empty()) [[likely]] {
        arr.reserve(object->size());

        for (const auto& item : *object) {
          Value temp(out->storage());
          Write<T>{}(&item, &temp);
          arr.emplace_back(std::move(temp));
        }
      }

      *out = std::move(arr);
    }
  };

  /**
 * @brief Specialization for Map<T>
 * @tparam T Value type (can be basic or complex)
 */
  template <typename T> struct Write<Map<T>> {
    inline void operator()(const Map<T>* object, Value* out) const {
      if (!object) [[unlikely]] {
        *out = nullptr;
        return;
      }

      boost::json::object obj(out->storage());

      if (!object->empty()) [[likely]] {
        obj.reserve(object->size()); // Pre-allocate space for better performance

        for (const auto& item : *object) {
          Value temp(out->storage());
          Write<T>{}(&item.second, &temp);
          obj.emplace(item.first, std::move(temp));
        }
      }

      *out = std::move(obj);
    }
  };

  /**
 * @brief Specialization for MultiMap<T>
 * @tparam T Value type (can be basic or complex)
 * @note For duplicate keys, values are stored as an array in JSON
 */
  template <typename T> struct Write<MultiMap<T>> {
    inline void operator()(const MultiMap<T>* object, Value* out) const {
      if (!object) [[unlikely]] {
        *out = nullptr;
        return;
      }

      if (object->empty()) [[unlikely]] {
        *out = boost::json::object(out->storage());
        return;
      }

      boost::json::object obj(out->storage());

      // Group values by key - use string_view for comparison to avoid copies
      std::string_view current_key;
      boost::json::array current_array(out->storage());

      for (const auto& item : *object) {
        std::string_view item_key = item.first;

        if (current_key.empty()) [[unlikely]] {
          // First item
          current_key = item_key;
        } else if (current_key != item_key) [[unlikely]] {
          // Key changed - emit previous key's values
          if (current_array.size() == 1) {
            // Single value - store directly
            obj.emplace(current_key, std::move(current_array[0]));
          } else {
            // Multiple values - store as array
            obj.emplace(current_key, std::move(current_array));
          }
          current_array.clear();
          current_key = item_key;
        }

        // Add current value to array
        Value temp(out->storage());
        Write<T>{}(&item.second, &temp);
        current_array.emplace_back(std::move(temp));
      }

      // Emit last key's values
      if (current_array.size() == 1) {
        obj.emplace(current_key, std::move(current_array[0]));
      } else {
        obj.emplace(current_key, std::move(current_array));
      }

      *out = std::move(obj);
    }
  };

  /**
 * @brief Specialization for MultiDict<T>
 * @tparam T Value type (can be basic or complex)
 * @note For duplicate keys, values are stored as an array in JSON
 */
  template <typename T> struct Write<MultiDict<T>> {
    inline void operator()(const MultiDict<T>* object, Value* out) const {
      if (!object) [[unlikely]] {
        *out = nullptr;
        return;
      }

      if (object->empty()) [[unlikely]] {
        *out = boost::json::object(out->storage());
        return;
      }

      boost::json::object obj(out->storage());

      // Build a map of keys to their values using string_view for efficient lookups
      std::unordered_map<std::string_view, boost::json::array> key_values;

      for (const auto& item : *object) {
        std::string_view key_view = item.first;
        auto it = key_values.find(key_view);
        if (it == key_values.end()) [[unlikely]] {
          it = key_values.emplace(key_view, boost::json::array(out->storage())).first;
        }

        Value temp(out->storage());
        Write<T>{}(&item.second, &temp);
        it->second.emplace_back(std::move(temp));
      }

      // Emit all keys
      obj.reserve(key_values.size());
      for (auto& kv : key_values) {
        if (kv.second.size() == 1) {
          // Single value - store directly
          obj.emplace(kv.first, std::move(kv.second[0]));
        } else {
          // Multiple values - store as array
          obj.emplace(kv.first, std::move(kv.second));
        }
      }

      *out = std::move(obj);
    }
  };

  /**
 * @brief Specialization for Pair<T>
 * @tparam T Value type (can be basic or complex)
 * @note Serializes as JSON object with "key" and "value" fields
 */
  template <typename T> struct Write<Pair<T>> {
    inline void operator()(const Pair<T>* object, Value* out) const {
      if (!object) [[unlikely]] {
        *out = nullptr;
        return;
      }

      boost::json::object obj(out->storage());
      obj.reserve(2);

      // Serialize key
      obj.emplace("key", object->first);

      // Serialize value
      Value temp(out->storage());
      Write<T>{}(&object->second, &temp);
      obj.emplace("value", std::move(temp));

      *out = std::move(obj);
    }
  };

  /**
 * @brief Specialization for Dict<T>
 * @tparam T Value type (can be basic or complex)
 */
  template <typename T> struct Write<Dict<T>> {
    inline void operator()(const Dict<T>* object, Value* out) const {
      if (!object) [[unlikely]] {
        *out = nullptr;
        return;
      }

      boost::json::object obj(out->storage());

      if (!object->empty()) [[likely]] {
        obj.reserve(object->size()); // Pre-allocate space for better performance

        for (const auto& item : *object) {
          Value temp(out->storage());
          Write<T>{}(&item.second, &temp);
          obj.emplace(item.first, std::move(temp));
        }
      }

      *out = std::move(obj);
    }
  };

  /**
 * @brief Specialization for Array<T, N>
 * @tparam T Element type (can be basic or complex)
 * @tparam N Array size (known at compile time)
 */
  template <typename T, std::size_t N> struct Write<Array<T, N>> {
    static_assert(!std::is_pointer_v<T>,
                  "Raw pointers are not supported in Array for serialization. "
                  "Use std::unique_ptr, std::shared_ptr, or std::optional instead.");

    inline void operator()(const Array<T, N>* object, Value* out) const {
      if (!object) [[unlikely]] {
        *out = nullptr;
        return;
      }

      boost::json::array arr(out->storage());
      arr.reserve(N);

      for (const auto& item : *object) {
        Value temp(out->storage());
        Write<T>{}(&item, &temp);
        arr.emplace_back(std::move(temp));
      }

      *out = std::move(arr);
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
    inline void operator()(const std::unique_ptr<T>* object,
                           boost::json::value* out) const noexcept {
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
    inline void operator()(const std::shared_ptr<T>* object,
                           boost::json::value* out) const noexcept {
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
  template <typename T> [[nodiscard]] Value Marshal(const T& object, StoragePtr sp = {}) {
    Value result(sp.get() ? sp : StoragePtr{});
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
  template <typename T> [[nodiscard]] String MarshalToString(const T& object, StoragePtr sp = {}) {
    return boost::json::serialize(Marshal(object, sp));
  }

  // =============================================================================
  // JSON Deserialization Templates
  // =============================================================================

  template <typename T, bool IsEnum = std::is_enum_v<T>> struct ReadEnum;
  template <typename T, bool IsStruct = has_struct_properties_v<T>> struct ReadStruct;

  /**
 * @brief Primary template for JSON deserialization
 * @tparam T Type to deserialize (must have static properties member for structs, or be an enum)
 */
  template <typename T> struct Read {
    /**
   * @brief Deserialize JSON value to object
   * @param value JSON value to deserialize from
   * @param object Pointer to object to populate
   */
    inline void operator()(const boost::json::value& value, T* object) const {
      if constexpr (std::is_enum_v<T>) {
        // Delegate to enum handler
        ReadEnum<T>{}(value, object);
      } else {
        // Delegate to struct handler
        ReadStruct<T>{}(value, object);
      }
    }
  };

  // =============================================================================
  // Generic Enum Handler for Read (for any enum type)
  // =============================================================================

  // Specialization for enums
  template <typename T> struct ReadEnum<T, true> {
    static_assert(enum_validator<T>::value, "Invalid ENUM configuration");

    inline void operator()(const boost::json::value& value, T* object) const {
      if (value.is_null()) {
        *object = T{};
        return;
      }

      if constexpr (ENUM<T>::encoding == EnumEncoding::NUMBER) {
        // Deserialize from integer value
        if (value.is_int64()) {
          *object = static_cast<T>(value.as_int64());
        } else if (value.is_uint64()) {
          *object = static_cast<T>(value.as_uint64());
        } else {
          throw_type_mismatch("integer for enum", value);
        }
      } else {
        // Deserialize from string name
        if (!value.is_string()) {
          throw_type_mismatch("string for enum name", value);
        }

        std::string_view input = value.as_string();
        const auto& sorted_names = detail::sorted_names_v<T>;
        const auto& original_names = ENUM<T>::names;

        // Binary search in sorted names
        auto it = std::lower_bound(sorted_names.begin(),
                                   sorted_names.end(),
                                   input,
                                   [](std::string_view a, std::string_view b) {
                                     if (ENUM<T>::case_insensitive) {
                                       std::size_t n = std::min(a.size(), b.size());
                                       for (std::size_t i = 0; i < n; ++i) {
                                         char ca = detail::to_lower(a[i]);
                                         char cb = detail::to_lower(b[i]);
                                         if (ca != cb) {
                                           return ca < cb;
                                         }
                                       }
                                       return a.size() < b.size();
                                     }
                                     return a < b;
                                   });

        if (it == sorted_names.end() ||
            (ENUM<T>::case_insensitive ? !detail::iequal(*it, input) : *it != input)) {
          throw std::runtime_error(std::string("Invalid enum value: ") + std::string(input));
        }

        // Find the original index in the unsorted names array
        std::size_t sorted_index = std::distance(sorted_names.begin(), it);
        std::string_view found_name = sorted_names[sorted_index];

        // Find the position in original names array
        for (std::size_t i = 0; i < original_names.size(); ++i) {
          if (ENUM<T>::case_insensitive ? detail::iequal(original_names[i], found_name) :
                                          original_names[i] == found_name) {
            *object = static_cast<T>(i);
            return;
          }
        }

        // This should never happen if arrays are consistent
        throw std::runtime_error(std::string("Internal error: could not map enum name: ") +
                                 std::string(input));
      }
    }
  };

  // Non-specialization for non-enums (should not be called)
  template <typename T> struct ReadEnum<T, false> {
    inline void operator()(const boost::json::value&, T*) const {
      static_assert(false, "ReadEnum should only be used for enum types");
    }
  };

  // Specialization for structs
  template <typename T> struct ReadStruct<T, true> {
    inline void operator()(const boost::json::value& value, T* object) const {
      if (!value.is_object()) [[unlikely]] {
        throw_invalid_json_object();
      }

      const boost::json::object& obj = value.as_object();
      constexpr auto props_tuple = get_properties_tuple<T>();
      static constexpr auto props = std::tuple_size_v<decltype(props_tuple)>;

      for_each(std::make_index_sequence<props>{}, [&](auto i) {
        constexpr auto property = std::get<i>(props_tuple);
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
                ::json::Read<P>{}(val, &temp);
                ptr = std::move(temp);
              } else {
                if constexpr (std::is_same_v<B, UniquePtr<P>>) {
                  ptr = MAKE_UNIQUE(P);
                } else if constexpr (std::is_same_v<B, SharedPtr<P>>) {
                  ptr = MAKE_SHARED(P);
                }
                ::json::Read<P>{}(val, ptr.get());
              }
            }
          } else {
            if (val.is_null()) [[unlikely]] {
              throw_field_null_not_nullable(property.name_);
            } else [[likely]] {
              ::json::Read<P>{}(val, &ptr);
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

  // Non-specialization for non-structs
  template <typename T> struct ReadStruct<T, false> {
    inline void operator()(const boost::json::value&, T*) const {
      static_assert(has_struct_properties_v<T>,
                    "T must have STRUCT<T> specialization for deserialization");
    }
  };

  // =============================================================================
  // Specializations for Nullable Types (Read)
  // =============================================================================

  /**
 * @brief Specialization for std::unique_ptr<T>
 * @tparam T Pointed-to type
 */
  template <typename T> struct Read<UniquePtr<T>> {
    inline void operator()(const Value& value, UniquePtr<T>* object) const {
      if (value.is_null()) [[unlikely]] {
        *object = nullptr;
      } else [[likely]] {
        *object = MAKE_UNIQUE(T);
        Read<T>{}(value, object->get());
      }
    }
  };

  /**
 * @brief Specialization for std::shared_ptr<T>
 * @tparam T Pointed-to type
 */
  template <typename T> struct Read<SharedPtr<T>> {
    inline void operator()(const Value& value, SharedPtr<T>* object) const {
      if (value.is_null()) [[unlikely]] {
        *object = nullptr;
      } else [[likely]] {
        *object = MAKE_SHARED(T);
        Read<T>{}(value, object->get());
      }
    }
  };

  /**
 * @brief Specialization for std::optional<T>
 * @tparam T Contained type
 */
  template <typename T> struct Read<Optional<T>> {
    inline void operator()(const Value& value, Optional<T>* object) const {
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
 * @brief Specialization for String
 */
  template <> struct Read<String> {
    inline void operator()(const Value& value, String* object) const {
      if (!value.is_string()) [[unlikely]] {
        throw_type_mismatch("string", value);
      }
      *object = value.as_string();
    }
  };

  /**
 * @brief Specialization for int64_t
 */
  template <> struct Read<Int64> {
    inline void operator()(const Value& value, Int64* object) const {
      if (value.is_int64()) [[likely]] {
        *object = value.as_int64();
      } else if (value.is_uint64()) [[unlikely]] {
        auto val = value.as_uint64();
        if (val <= static_cast<std::uint64_t>(std::numeric_limits<Int64>::max())) {
          *object = static_cast<Int64>(val);
        } else {
          throw_value_too_large_for_int64();
        }
      } else {
        throw_type_mismatch("integer", value);
      }
    }
  };

  /**
 * @brief Specialization for uint64_t
 */
  template <> struct Read<UInt64> {
    inline void operator()(const Value& value, UInt64* object) const {
      if (value.is_uint64()) [[likely]] {
        *object = value.as_uint64();
      } else if (value.is_int64()) [[unlikely]] {
        auto val = value.as_int64();
        if (val >= 0) {
          *object = static_cast<UInt64>(val);
        } else {
          throw_negative_value_for_uint64();
        }
      } else {
        throw_type_mismatch("unsigned integer", value);
      }
    }
  };

  /**
 * @brief Specialization for double
 */
  template <> struct Read<Double> {
    inline void operator()(const Value& value, Double* object) const {
      if (value.is_double()) [[likely]] {
        *object = value.as_double();
      } else if (value.is_int64()) [[unlikely]] {
        *object = static_cast<Double>(value.as_int64());
      } else if (value.is_uint64()) [[unlikely]] {
        *object = static_cast<Double>(value.as_uint64());
      } else {
        throw_type_mismatch("number", value);
      }
    }
  };

  /**
 * @brief Specialization for bool
 */
  template <> struct Read<Bool> {
    inline void operator()(const Value& value, Bool* object) const {
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
  template <> struct Read<Value> {
    inline void operator()(const Value& value, Value* object) const {
      *object = value; // Pass through as-is
    }
  };

  // =============================================================================
  // Base64 Support for Binary Data (Read)
  // =============================================================================

  /**
 * @brief Specialization for ByteVector - deserialize from Base64 string
 */
  template <> struct Read<ByteVector> {
    inline void operator()(const Value& value, ByteVector* out) const {
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
      auto [decoded, consumed] = base64::decode(out->data(), str.data(), str.size());
      if (consumed != str.size() || (decoded == 0 && !str.empty())) {
        throw_invalid_base64();
      }
      out->resize(decoded);
    }
  };

  /**
 * @brief Specialization for ByteArray<N> - deserialize from Base64 string
 * @tparam N Array size (known at compile time)
 */
  template <std::size_t N> struct Read<ByteArray<N>> {
    inline void operator()(const Value& value, ByteArray<N>* out) const {
      if (value.is_null()) [[unlikely]] {
        throw_byte_array_null();
      }
      if (!value.is_string()) {
        throw_type_mismatch("base64 string", value);
      }
      const auto& str = value.as_string();

      // Decode the base64 string directly into the array
      auto [decoded, consumed] = base64::decode(out->data(), str.data(), str.size());
      if (consumed != str.size() || (decoded == 0 && !str.empty())) {
        throw_invalid_base64();
      }

      // Verify that the decoded size matches the expected array size
      if (decoded != N) {
        throw_byte_array_size_mismatch(decoded, N);
      }
    }
  };

  // =============================================================================
  // Specializations for Container Types (Read)
  // =============================================================================

  /**
 * @brief Specialization for std::vector<T>
 * @tparam T Element type
 */
  template <typename T> struct Read<Vector<T>> {
    static_assert(!std::is_pointer_v<T>,
                  "Raw pointers are not supported in std::vector for deserialization. "
                  "Use std::unique_ptr, std::shared_ptr, or std::optional instead.");

    inline void operator()(const Value& value, Vector<T>* object) const {
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
 * @brief Specialization for List<T>
 * @tparam T Element type
 */
  template <typename T> struct Read<List<T>> {
    static_assert(!std::is_pointer_v<T>,
                  "Raw pointers are not supported in List for deserialization. "
                  "Use std::unique_ptr, std::shared_ptr, or std::optional instead.");

    inline void operator()(const Value& value, List<T>* object) const {
      if (value.is_null()) [[unlikely]] {
        object->clear();
      } else if (value.is_array()) [[likely]] {
        const boost::json::array& arr = value.as_array();
        object->clear();

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
 * @brief Specialization for Map<T>
 * @tparam T Value type
 */
  template <typename T> struct Read<Map<T>> {
    inline void operator()(const Value& value, Map<T>* object) const {
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
          object->emplace(String(item.key()), std::move(temp));
        }
      } else {
        throw_type_mismatch("object", value);
      }
    }
  };

  /**
 * @brief Specialization for MultiMap<T>
 * @tparam T Value type
 * @note Handles both single values and arrays for each key
 */
  template <typename T> struct Read<MultiMap<T>> {
    inline void operator()(const Value& value, MultiMap<T>* object) const {
      if (value.is_null()) [[unlikely]] {
        object->clear();
      } else if (value.is_object()) [[likely]] {
        const boost::json::object& obj = value.as_object();
        object->clear();

        for (const auto& item : obj) {
          // Convert key once and reuse for all values
          String key_str(item.key());

          if (item.value().is_array()) {
            // Multiple values for this key
            const boost::json::array& arr = item.value().as_array();
            for (const auto& arr_item : arr) {
              T temp;
              Read<T>{}(arr_item, &temp);
              object->emplace(key_str, std::move(temp));
            }
          } else {
            // Single value for this key
            T temp;
            Read<T>{}(item.value(), &temp);
            object->emplace(key_str, std::move(temp));
          }
        }
      } else {
        throw_type_mismatch("object", value);
      }
    }
  };

  /**
 * @brief Specialization for MultiDict<T>
 * @tparam T Value type
 * @note Handles both single values and arrays for each key
 */
  template <typename T> struct Read<MultiDict<T>> {
    inline void operator()(const Value& value, MultiDict<T>* object) const {
      if (value.is_null()) [[unlikely]] {
        object->clear();
      } else if (value.is_object()) [[likely]] {
        const boost::json::object& obj = value.as_object();
        object->clear();

        for (const auto& item : obj) {
          // Convert key once and reuse for all values
          String key_str(item.key());

          if (item.value().is_array()) {
            // Multiple values for this key
            const boost::json::array& arr = item.value().as_array();
            for (const auto& arr_item : arr) {
              T temp;
              Read<T>{}(arr_item, &temp);
              object->emplace(key_str, std::move(temp));
            }
          } else {
            // Single value for this key
            T temp;
            Read<T>{}(item.value(), &temp);
            object->emplace(key_str, std::move(temp));
          }
        }
      } else {
        throw_type_mismatch("object", value);
      }
    }
  };

  /**
 * @brief Specialization for Pair<T>
 * @tparam T Value type
 * @note Expects JSON object with "key" and "value" fields
 */
  template <typename T> struct Read<Pair<T>> {
    inline void operator()(const Value& value, Pair<T>* object) const {
      if (value.is_null()) [[unlikely]] {
        object->first.clear();
        object->second = T{};
      } else if (value.is_object()) [[likely]] {
        const boost::json::object& obj = value.as_object();

        // Read key field
        auto key_it = obj.find("key");
        if (key_it == obj.end()) {
          throw_field_missing("key");
        }
        if (!key_it->value().is_string()) {
          throw_type_mismatch("string for key field", key_it->value());
        }
        object->first = key_it->value().as_string();

        // Read value field
        auto value_it = obj.find("value");
        if (value_it == obj.end()) {
          throw_field_missing("value");
        }
        Read<T>{}(value_it->value(), &object->second);
      } else {
        throw_type_mismatch("object", value);
      }
    }
  };

  /**
 * @brief Specialization for Dict<T>
 * @tparam T Value type
 */
  template <typename T> struct Read<Dict<T>> {
    inline void operator()(const Value& value, Dict<T>* object) const {
      if (value.is_null()) [[unlikely]] {
        object->clear();
      } else if (value.is_object()) [[likely]] {
        const boost::json::object& obj = value.as_object();
        object->clear();
        object->reserve(obj.size()); // Pre-allocate space for better performance

        for (const auto& item : obj) {
          T temp;
          Read<T>{}(item.value(), &temp);
          object->emplace(String(item.key()), std::move(temp));
        }
      } else {
        throw_type_mismatch("object", value);
      }
    }
  };

  /**
 * @brief Specialization for Array<T, N>
 * @tparam T Element type
 * @tparam N Array size (known at compile time)
 */
  template <typename T, std::size_t N> struct Read<Array<T, N>> {
    static_assert(!std::is_pointer_v<T>,
                  "Raw pointers are not supported in Array for deserialization. "
                  "Use std::unique_ptr, std::shared_ptr, or std::optional instead.");

    inline void operator()(const Value& value, Array<T, N>* object) const {
      if (value.is_null()) [[unlikely]] {
        // For null values, we can't clear a fixed-size array, so we throw an error
        throw_array_null();
      } else if (value.is_array()) [[likely]] {
        const boost::json::array& arr = value.as_array();

        // Check that the JSON array size matches the Array size
        if (arr.size() != N) {
          throw_array_size_mismatch(arr.size(), N);
        }

        // Deserialize each element
        for (std::size_t i = 0; i < N; ++i) {
          Read<T>{}(arr[i], &(*object)[i]);
        }
      } else {
        throw_type_mismatch("array", value);
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
  template <typename T> void Unmarshal(const Value& value, T& object) {
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
  void UnmarshalFromString(const String& json_string, T& object, StoragePtr sp = {}) {
    boost::system::error_code ec;
    Value value =
        sp.get() ? boost::json::parse(json_string, ec, sp) : boost::json::parse(json_string, ec);
    if (ec) {
      throw_json_parse_error(ec.message());
    }
    Read<T>{}(value, &object);
  }

} // namespace json

#endif // JSON_H_INCLUDED