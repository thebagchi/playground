#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include <memory>
#include <optional>
#include <type_traits>
#include <boost/json.hpp>
#include <array>
#include <list>

// =============================================================================
// Type Aliases for Reduced Verbosity
// =============================================================================

/**
 * @brief Byte vector type for binary data (commonly used with Base64 encoding)
 */
using ByteVector = std::vector<std::byte>;

/**
 * @brief Byte array type alias (fixed-size array)
 */
template <std::size_t N> using ByteArray = std::array<std::byte, N>;

/**
 * @brief String type alias
 */
using String = std::string;

/**
 * @brief Basic type aliases
 */
using Int64 = std::int64_t;
using UInt64 = std::uint64_t;
using Double = double;
using Bool = bool;

/**
 * @brief Generic vector type alias
 */
template <typename T> using Vector = std::vector<T>;

/**
 * @brief Generic array type alias (fixed-size array)
 */
template <typename T, std::size_t N> using Array = std::array<T, N>;

/**
 * @brief Generic list type alias (doubly-linked list)
 */
template <typename T> using List = std::list<T>;

/**
 * @brief Map type alias (ordered map with string keys)
 */
template <typename T> using Map = std::map<std::string, T>;

/**
 * @brief MultiMap type alias (ordered multimap with string keys - allows duplicate keys)
 */
template <typename T> using MultiMap = std::multimap<std::string, T>;

/**
 * @brief Dictionary type alias (unordered map with string keys)
 */
template <typename T> using Dict = std::unordered_map<std::string, T>;

/**
 * @brief MultiDict type alias (unordered multimap with string keys - allows duplicate keys)
 */
template <typename T> using MultiDict = std::unordered_multimap<std::string, T>;

/**
 * @brief Pair type alias (string key-value pair)
 */
template <typename T> using Pair = std::pair<std::string, T>;

/**
 * @brief Unique pointer type alias
 */
template <typename T> using UniquePtr = std::unique_ptr<T>;

/**
 * @brief Shared pointer type alias
 */
template <typename T> using SharedPtr = std::shared_ptr<T>;

/**
 * @brief Optional type alias
 */
template <typename T> using Optional = std::optional<T>;

/**
 * @brief Boost JSON value type alias
 */
using Value = boost::json::value;

/**
 * @brief Boost JSON storage pointer type alias
 */
using StoragePtr = boost::json::storage_ptr;

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

template <typename T> struct is_optional<Optional<T>> : std::true_type {};

template <typename T> struct is_optional<UniquePtr<T>> : std::true_type {};

template <typename T> struct is_optional<SharedPtr<T>> : std::true_type {};

template <typename T> inline constexpr bool is_optional_v = is_optional<T>::value;

template <typename T> inline constexpr bool is_enum_v = std::is_enum_v<T>;

/**
 * @brief Trait to check if a type is a JSON value
 */
template <typename T> struct is_json_value : std::false_type {};

template <> struct is_json_value<Value> : std::true_type {};

template <typename T> inline constexpr bool is_json_value_v = is_json_value<T>::value;

/**
 * @brief Trait to get the value type from a optional type
 */
template <typename T, typename = void> struct opt {
  using type = T;
};

template <typename T> struct opt<Optional<T>> {
  using type = T;
};

template <typename T> struct opt<UniquePtr<T>> {
  using type = T;
};

template <typename T> struct opt<SharedPtr<T>> {
  using type = T;
};

template <typename T> struct opt<const T> : opt<T> {};

template <typename T> using opt_t = typename opt<T>::type;

// =============================================================================
// JSON Value Utilities
// =============================================================================

/**
 * @brief JSON Value Creation Examples
 *
 * // Create basic JSON values
 * auto null_val = make_null();           // null
 * auto bool_val = make_bool(true);       // true
 * auto str_val = make_string("hello");   // "hello"
 * auto int_val = make_int64(42);         // 42
 * auto uint_val = make_uint64(100);      // 100
 * auto double_val = make_double(3.14);   // 3.14
 *
 * // Create containers
 * auto obj = make_object();              // {}
 * auto arr = make_array();               // []
 *
 * // Create with custom storage
 * auto storage = make_monotonic_storage();
 * auto obj_with_storage = make_object(storage);
 * auto arr_with_storage = make_array(storage);
 */

/**
 * @brief Create a JSON null value
 * @param storage Optional storage pointer (defaults to default storage)
 * @return Value representing null
 */
inline Value make_null(StoragePtr storage = {}) {
  return Value(nullptr, storage);
}

/**
 * @brief Create a JSON boolean value
 * @param b Boolean value
 * @param storage Optional storage pointer (defaults to default storage)
 * @return Value representing the boolean
 */
inline Value make_bool(bool b, StoragePtr storage = {}) {
  return Value(b, storage);
}

/**
 * @brief Create a JSON string value
 * @param s String value
 * @param storage Optional storage pointer (defaults to default storage)
 * @return Value representing the string
 */
inline Value make_string(const String& s, StoragePtr storage = {}) {
  return Value(s, storage);
}

/**
 * @brief Create a JSON number value from int64
 * @param n Integer value
 * @param storage Optional storage pointer (defaults to default storage)
 * @return Value representing the number
 */
inline Value make_int64(Int64 n, StoragePtr storage = {}) {
  return Value(n, storage);
}

/**
 * @brief Create a JSON number value from uint64
 * @param n Unsigned integer value
 * @param storage Optional storage pointer (defaults to default storage)
 * @return Value representing the number
 */
inline Value make_uint64(UInt64 n, StoragePtr storage = {}) {
  return Value(n, storage);
}

/**
 * @brief Create a JSON number value from double
 * @param d Double value
 * @param storage Optional storage pointer (defaults to default storage)
 * @return Value representing the number
 */
inline Value make_double(Double d, StoragePtr storage = {}) {
  return Value(d, storage);
}

/**
 * @brief Create an empty JSON object
 * @param storage Optional storage pointer (defaults to default storage)
 * @return Value representing an empty object
 */
inline Value make_object(StoragePtr storage = {}) {
  return Value(boost::json::object_kind, storage);
}

/**
 * @brief Create an empty JSON array
 * @param storage Optional storage pointer (defaults to default storage)
 * @return Value representing an empty array
 */
inline Value make_array(StoragePtr storage = {}) {
  return Value(boost::json::array_kind, storage);
}

// =============================================================================
// Storage Pointer Utilities
// =============================================================================

/**
 * @brief Storage Pointer Creation Examples
 *
 * // Default monotonic storage (good for JSON parsing/serialization)
 * auto storage = make_monotonic_storage();
 * auto value = make_object_with_storage(storage);
 *
 * // Custom allocator storage
 * std::pmr::monotonic_buffer_resource mono_res(4096);
 * auto custom_storage = make_monotonic_storage(mono_res);
 *
 * // Static buffer storage (no heap allocations)
 * std::byte buffer[4096];
 * auto static_storage = make_static_storage(buffer);
 * auto limited_value = make_object_with_storage(static_storage);
 *
 * // Dynamic buffer storage
 * auto dyn_buffer = std::make_unique<std::byte[]>(8192);
 * auto dyn_storage = make_static_storage(dyn_buffer.get(), 8192);
 */

/**
 * @brief Create a storage pointer with monotonic resource
 * @tparam Allocator The allocator type (defaults to monotonic_resource)
 * @param alloc The allocator instance (defaults to default-constructed monotonic_resource)
 * @return StoragePtr using the specified allocator
 */
template <typename Allocator = boost::json::monotonic_resource>
inline StoragePtr make_monotonic_storage(const Allocator& alloc = Allocator{}) {
  return boost::json::make_shared_resource(alloc);
}

/**
 * @brief Create a storage pointer with static resource using a fixed buffer
 * @tparam N The size of the static buffer in bytes
 * @param buffer The buffer to use for allocations
 * @return StoragePtr using the static resource
 */
template <std::size_t N> inline StoragePtr make_static_storage(std::byte (&buffer)[N]) {
  return StoragePtr(new boost::json::static_resource(buffer));
}

/**
 * @brief Create a storage pointer with static resource using a buffer pointer and size
 * @param buffer Pointer to the buffer to use for allocations
 * @param size Size of the buffer in bytes
 * @return StoragePtr using the static resource
 */
inline StoragePtr make_static_storage(std::byte* buffer, std::size_t size) {
  return StoragePtr(new boost::json::static_resource(buffer, size));
}

#endif // UTILS_H_INCLUDED