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
  //     constexpr static auto properties = std::make_tuple(
  //       prop(&MyStruct::name, "name")
  //     );
  //   };
  //
  //   MyStruct obj;
  //   Value json = Write<MyStruct>{}(&obj);
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
    String msg;
    const size_t name_len = std::strlen(field_name);
    msg.reserve(30 + name_len); // Pre-allocate exact space needed
    msg.append("Field '").append(field_name, name_len).append("' is null but not nullable");
    throw std::runtime_error(std::move(msg));
  }

  [[noreturn]] inline void throw_field_missing(const char* field_name) {
    String msg;
    const size_t name_len = std::strlen(field_name);
    msg.reserve(17 + name_len); // Pre-allocate exact space needed
    msg.append("Field '").append(field_name, name_len).append("' is missing");
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
      static_assert(has_properties_v<T>,
                    "T must have a static properties member for serialization");
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
      arr.reserve(object->size());

      for (const auto& item : *object) {
        Value temp(out->storage());
        Write<T>{}(&item, &temp);
        arr.emplace_back(std::move(temp));
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
      arr.reserve(object->size());

      for (const auto& item : *object) {
        Value temp(out->storage());
        Write<T>{}(&item, &temp);
        arr.emplace_back(std::move(temp));
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
      obj.reserve(object->size()); // Pre-allocate space for better performance

      for (const auto& item : *object) {
        Value temp(out->storage());
        Write<T>{}(&item.second, &temp);
        obj.emplace(item.first, std::move(temp));
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

      boost::json::object obj(out->storage());

      // Group values by key
      String current_key;
      boost::json::array current_array(out->storage());

      for (const auto& item : *object) {
        if (current_key.empty()) {
          // First item
          current_key = item.first;
        } else if (current_key != item.first) {
          // Key changed - emit previous key's values
          if (current_array.size() == 1) {
            // Single value - store directly
            obj.emplace(current_key, std::move(current_array[0]));
          } else {
            // Multiple values - store as array
            obj.emplace(current_key, std::move(current_array));
          }
          current_array = boost::json::array(out->storage());
          current_key = item.first;
        }

        // Add current value to array
        Value temp(out->storage());
        Write<T>{}(&item.second, &temp);
        current_array.emplace_back(std::move(temp));
      }

      // Emit last key's values
      if (!current_key.empty()) {
        if (current_array.size() == 1) {
          obj.emplace(current_key, std::move(current_array[0]));
        } else {
          obj.emplace(current_key, std::move(current_array));
        }
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

      boost::json::object obj(out->storage());

      // Build a map of keys to their values
      std::unordered_map<String, boost::json::array> key_values;

      for (const auto& item : *object) {
        auto it = key_values.find(item.first);
        if (it == key_values.end()) {
          it = key_values.emplace(item.first, boost::json::array(out->storage())).first;
        }

        Value temp(out->storage());
        Write<T>{}(&item.second, &temp);
        it->second.emplace_back(std::move(temp));
      }

      // Emit all keys
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
      obj.reserve(object->size()); // Pre-allocate space for better performance

      for (const auto& item : *object) {
        Value temp(out->storage());
        Write<T>{}(&item.second, &temp);
        obj.emplace(item.first, std::move(temp));
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
                ptr = MAKE_UNIQUE(P);
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
      const auto& str = value.as_string();
      object->reserve(str.size());
      *object = str;
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
      auto [decoded_size, input_size] = base64::decode(out->data(), str.data(), str.size());
      if (decoded_size == 0 && !str.empty()) {
        throw_invalid_base64();
      }
      out->resize(decoded_size);
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
      auto [decoded_size, input_size] = base64::decode(out->data(), str.data(), str.size());
      if (decoded_size == 0 && !str.empty()) {
        throw_invalid_base64();
      }

      // Verify that the decoded size matches the expected array size
      if (decoded_size != N) {
        throw_byte_array_size_mismatch(decoded_size, N);
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
          object->emplace(boost::json::string_view(item.key()), std::move(temp));
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
          const String key = String(item.key());

          if (item.value().is_array()) {
            // Multiple values for this key
            const boost::json::array& arr = item.value().as_array();
            for (const auto& arr_item : arr) {
              T temp;
              Read<T>{}(arr_item, &temp);
              object->emplace(key, std::move(temp));
            }
          } else {
            // Single value for this key
            T temp;
            Read<T>{}(item.value(), &temp);
            object->emplace(key, std::move(temp));
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
          const String key = String(item.key());

          if (item.value().is_array()) {
            // Multiple values for this key
            const boost::json::array& arr = item.value().as_array();
            for (const auto& arr_item : arr) {
              T temp;
              Read<T>{}(arr_item, &temp);
              object->emplace(key, std::move(temp));
            }
          } else {
            // Single value for this key
            T temp;
            Read<T>{}(item.value(), &temp);
            object->emplace(key, std::move(temp));
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
        object->first = String(key_it->value().as_string());

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
          object->emplace(boost::json::string_view(item.key()), std::move(temp));
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