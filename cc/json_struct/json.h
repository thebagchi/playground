#ifndef JSON_H_INCLUDED
#define JSON_H_INCLUDED

#include <boost/json.hpp>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

// =============================================================================
// Type Traits
// =============================================================================

/**
 * @brief Trait to check if a type has a static properties member
 */
template <typename T, typename = void>
struct has_properties : std::false_type {};

template <typename T>
struct has_properties<T, std::void_t<decltype(T::properties)>>
    : std::true_type {};

template <typename T>
inline constexpr bool has_properties_v = has_properties<T>::value;

/**
 * @brief Trait to check if a type is optional (has operator bool)
 */
template <typename T>
struct is_optional : std::false_type {};

template <typename T>
struct is_optional<std::optional<T>> : std::true_type {};

template <typename T>
struct is_optional<std::unique_ptr<T>> : std::true_type {};

template <typename T>
struct is_optional<std::shared_ptr<T>> : std::true_type {};

template <typename T>
inline constexpr bool is_optional_v = is_optional<T>::value;

/**
 * @brief Trait to get the value type from a optional type
 */
template <typename T, typename = void>
struct opt {
  using type = T;
};

template <typename T>
struct opt<std::optional<T>> {
  using type = T;
};

template <typename T>
struct opt<std::unique_ptr<T>> {
  using type = T;
};

template <typename T>
struct opt<std::shared_ptr<T>> {
  using type = T;
};

template <typename T>
struct opt<const T> : opt<T> {};

template <typename T>
using opt_t = typename opt<T>::type;

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
template <typename C, typename T, bool Nullable = false, bool Required = false>
struct Props {
  static_assert(!Nullable || is_optional_v<T>,
                "If Nullable is true, T must be a pointer or optional type");
  static_assert(!std::is_pointer_v<T>,
                "Naked pointers are not allowed as member types");

  /**
   * @brief Constructor for property descriptor
   * @param member Pointer to member variable
   * @param name JSON field name
   */
  constexpr Props(T C::* member, const char* name)
      : member_{member}, name_{name} {
    // Do Nothing ...
  }

  using Type = T;                              ///< The smart pointer type
  T C::* member_;                              ///< Pointer to member variable
  const char* name_;                           ///< JSON field name
  static constexpr bool nullable_ = Nullable;  ///< Whether field can be null
  static constexpr bool required_ = Required;  ///< Whether field is required
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
constexpr Props<C, T, Nullable, Required> prop(T C::* member,
                                               const char* name) {
  return Props<C, T, Nullable, Required>{member, name};
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
  msg.reserve(50 + std::strlen(field_name));  // Pre-allocate space
  msg.append("Field '").append(field_name).append("' is null but not nullable");
  throw std::runtime_error(msg);
}

[[noreturn]] inline void throw_field_missing(const char* field_name) {
  std::string msg;
  msg.reserve(25 + std::strlen(field_name));  // Pre-allocate space
  msg.append("Field '").append(field_name).append("' is missing");
  throw std::runtime_error(msg);
}

// =============================================================================
// JSON Serialization Templates
// =============================================================================

/**
 * @brief Forward declaration of primary template for JSON serialization
 */
template <typename T>
struct Write;

// =============================================================================
// Specializations for Basic Types
// =============================================================================

/**
 * @brief Specialization for std::string
 */
template <>
struct Write<std::string> {
  inline boost::json::value operator()(
      const std::string* object) const noexcept {
    if (object) {
      return boost::json::string_view(*object);
    } else {
      return nullptr;
    }
  }
};

/**
 * @brief Specialization for const std::string
 */
template <>
struct Write<const std::string> {
  inline boost::json::value operator()(
      const std::string* object) const noexcept {
    if (object) {
      return boost::json::string_view(*object);
    } else {
      return nullptr;
    }
  }
};

/**
 * @brief Specialization for int64_t
 */
template <>
struct Write<std::int64_t> {
  inline boost::json::value operator()(
      const std::int64_t* object) const noexcept {
    if (object) {
      return *object;
    } else {
      return nullptr;
    }
  }
};

/**
 * @brief Specialization for const int64_t
 */
template <>
struct Write<const std::int64_t> {
  inline boost::json::value operator()(
      const std::int64_t* object) const noexcept {
    if (object) {
      return *object;
    } else {
      return nullptr;
    }
  }
};

/**
 * @brief Specialization for uint64_t
 */
template <>
struct Write<std::uint64_t> {
  inline boost::json::value operator()(
      const std::uint64_t* object) const noexcept {
    if (object) {
      return *object;
    } else {
      return nullptr;
    }
  }
};

/**
 * @brief Specialization for const uint64_t
 */
template <>
struct Write<const std::uint64_t> {
  inline boost::json::value operator()(
      const std::uint64_t* object) const noexcept {
    if (object) {
      return *object;
    } else {
      return nullptr;
    }
  }
};

/**
 * @brief Specialization for double
 */
template <>
struct Write<double> {
  inline boost::json::value operator()(const double* object) const noexcept {
    if (object) {
      return *object;
    } else {
      return nullptr;
    }
  }
};

/**
 * @brief Specialization for const double
 */
template <>
struct Write<const double> {
  inline boost::json::value operator()(const double* object) const noexcept {
    if (object) {
      return *object;
    } else {
      return nullptr;
    }
  }
};

/**
 * @brief Specialization for bool
 */
template <>
struct Write<bool> {
  inline boost::json::value operator()(const bool* object) const noexcept {
    if (object) {
      return *object;
    } else {
      return nullptr;
    }
  }
};

/**
 * @brief Specialization for const bool
 */
template <>
struct Write<const bool> {
  inline boost::json::value operator()(const bool* object) const noexcept {
    if (object) {
      return *object;
    } else {
      return nullptr;
    }
  }
};

// =============================================================================
// Primary Template Definition
// =============================================================================

/**
 * @brief Primary template for JSON serialization
 * @tparam T Type to serialize (must have static properties member)
 */
template <typename T>
struct Write {
  /**
   * @brief Serialize object to JSON value
   * @param object Pointer to object to serialize
   * @return JSON value representation
   */
  boost::json::value operator()(const T* object) {
    static_assert(has_properties_v<T>,
                  "T must have a static properties member for serialization");
    boost::json::object obj;
    constexpr auto props = std::tuple_size<decltype(T::properties)>::value;
    obj.reserve(props);  // Pre-allocate space for better performance

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
        if (ptr) {
          obj[property.name_] = Write<P>{}(&*ptr);
        } else if (property.nullable_) {
          obj[property.name_] = nullptr;
        } else {
          throw_field_null_not_nullable(property.name_);
        }
      } else {
        obj[property.name_] = Write<P>{}(&ptr);
      }
    });

    return obj;
  }
};

// =============================================================================
// Specializations for Container Types
// =============================================================================

/**
 * @brief Specialization for std::vector<T>
 * @tparam T Element type (can be basic or complex)
 */
template <typename T>
struct Write<std::vector<T>> {
  boost::json::value operator()(const std::vector<T>* object) {
    if (!object) return nullptr;

    boost::json::array arr;
    arr.reserve(object->size());

    for (const auto& item : *object) {
      arr.push_back(Write<T>{}(&item));
    }

    return arr;
  }
};

/**
 * @brief Specialization for std::map<std::string, T>
 * @tparam T Value type (can be basic or complex)
 */
template <typename T>
struct Write<std::map<std::string, T>> {
  boost::json::value operator()(const std::map<std::string, T>* object) {
    if (!object) return nullptr;

    boost::json::object obj;

    for (const auto& item : *object) {
      obj[item.first] = Write<T>{}(&item.second);
    }

    return obj;
  }
};

// =============================================================================
// Marshal Function (Wrapper for Write)
// =============================================================================

/**
 * @brief Convenience function to marshal (serialize) an object to JSON
 * @tparam T Type to serialize
 * @param object Reference to the object to serialize
 * @return JSON value representation
 */
template <typename T>
boost::json::value Marshal(const T& object) {
  return Write<T>{}(&object);
}

// =============================================================================
// JSON Deserialization Templates
// =============================================================================

/**
 * @brief Primary template for JSON deserialization
 * @tparam T Type to deserialize (must have static properties member)
 */
template <typename T>
struct Read {
  /**
   * @brief Deserialize JSON value to object
   * @param value JSON value to deserialize from
   * @param object Pointer to object to populate
   */
  void operator()(const boost::json::value& value, T* object) {
    if (!value.is_object()) {
      return;  // Invalid JSON type for object deserialization
    }

    const boost::json::object& obj = value.as_object();
    constexpr auto props = std::tuple_size<decltype(T::properties)>::value;

    for_each(std::make_index_sequence<props>{}, [&](auto i) {
      constexpr auto property = std::get<i>(T::properties);
      using Type = typename decltype(property)::Type;

      // Check if JSON object contains this field
      auto it = obj.find(property.name_);
      if (it != obj.end()) {
        const auto& val = it->value();
        auto& ptr = object->*(property.member_);
        using M = std::remove_reference_t<decltype(ptr)>;
        using B = std::remove_const_t<M>;
        using P = opt_t<B>;

        if constexpr (is_optional_v<B>) {
          if (val.is_null()) {
            if (property.nullable_) {
              if constexpr (std::is_same_v<std::decay_t<decltype(ptr)>,
                                           std::optional<P>>) {
                ptr = std::nullopt;
              } else {
                ptr = nullptr;
              }
            } else {
              throw_field_null_not_nullable(property.name_);
            }
          } else {
            if constexpr (std::is_same_v<std::decay_t<decltype(ptr)>,
                                         std::optional<P>>) {
              P temp;
              Read<P>{}(val, &temp);
              ptr = std::move(temp);
            } else {
              ptr = std::make_unique<P>();
              Read<P>{}(val, ptr.get());
            }
          }
        } else {
          if (val.is_null()) {
            throw_field_null_not_nullable(property.name_);
          } else {
            Read<P>{}(val, &ptr);
          }
        }
      } else {
        // Field missing
        if (property.nullable_) {
          auto& ptr = object->*(property.member_);
          using M = std::remove_reference_t<decltype(ptr)>;
          using B = std::remove_const_t<M>;
          if constexpr (is_optional_v<B>) {
            using P = opt_t<B>;
            if constexpr (std::is_same_v<std::decay_t<decltype(ptr)>,
                                         std::optional<P>>) {
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
// Specializations for Basic Types (Read)
// =============================================================================

/**
 * @brief Specialization for std::string
 */
template <>
struct Read<std::string> {
  void operator()(const boost::json::value& value, std::string* object) const {
    if (!value.is_null() && value.is_string()) {
      *object = value.as_string().c_str();
    }
  }
};

/**
 * @brief Specialization for int64_t
 */
template <>
struct Read<std::int64_t> {
  void operator()(const boost::json::value& value, std::int64_t* object) const {
    if (!value.is_null()) {
      if (value.is_int64()) {
        *object = value.as_int64();
      } else if (value.is_uint64()) {
        auto val = value.as_uint64();
        if (val <= static_cast<std::uint64_t>(
                       std::numeric_limits<std::int64_t>::max())) {
          *object = static_cast<std::int64_t>(val);
        }
      }
    }
  }
};

/**
 * @brief Specialization for uint64_t
 */
template <>
struct Read<std::uint64_t> {
  void operator()(const boost::json::value& value,
                  std::uint64_t* object) const {
    if (!value.is_null()) {
      if (value.is_uint64()) {
        *object = value.as_uint64();
      } else if (value.is_int64()) {
        auto val = value.as_int64();
        if (val >= 0) {
          *object = static_cast<std::uint64_t>(val);
        }
      }
    }
  }
};

/**
 * @brief Specialization for double
 */
template <>
struct Read<double> {
  void operator()(const boost::json::value& value, double* object) const {
    if (!value.is_null() && value.is_double()) {
      *object = value.as_double();
    }
  }
};

/**
 * @brief Specialization for bool
 */
template <>
struct Read<bool> {
  void operator()(const boost::json::value& value, bool* object) const {
    if (!value.is_null() && value.is_bool()) {
      *object = value.as_bool();
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
template <typename T>
struct Read<std::vector<T>> {
  void operator()(const boost::json::value& value, std::vector<T>* object) {
    if (!value.is_null() && value.is_array()) {
      const boost::json::array& arr = value.as_array();
      object->clear();
      object->reserve(arr.size());

      for (const auto& item : arr) {
        T temp;
        Read<T>{}(item, &temp);
        object->push_back(std::move(temp));
      }
    }
  }
};

/**
 * @brief Specialization for std::map<std::string, T>
 * @tparam T Value type
 */
template <typename T>
struct Read<std::map<std::string, T>> {
  void operator()(const boost::json::value& value,
                  std::map<std::string, T>* object) {
    if (!value.is_null() && value.is_object()) {
      const boost::json::object& obj = value.as_object();
      object->clear();

      for (const auto& item : obj) {
        T temp;
        Read<T>{}(item.value(), &temp);
        (*object)[std::string(item.key())] = std::move(temp);
      }
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
template <typename T>
void Unmarshal(const boost::json::value& value, T& object) {
  Read<T>{}(value, &object);
}

#endif  // JSON_H_INCLUDED