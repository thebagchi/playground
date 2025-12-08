//
// Copyright 2020 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//
#ifndef CBOR_H_INCLUDED
#define CBOR_H_INCLUDED
/*
    This header implements parsing and serialization of CBOR (Concise Binary Object Representation)
    with support for JSON-compatible types plus additional CBOR features including:
    - Indefinite length encoding for arrays, objects, and strings
    - Half-precision (16-bit) floating point numbers
    - UTF-8 validation via boost::json (strings must be valid UTF-8)
    - Semantic tag parsing (tags preserved as {"_tag": tag_number, "_value": tagged_value})
    - Binary strings (base64 encoded as {"_binary": "base64..."})
*/
/*
    CBOR Types (RFC 8949: https://www.rfc-editor.org/rfc/rfc8949.html)
    CBOR uses a type system based on major types (3 bits) and minor types/additional information (5 bits):
    Major Type 0: Unsigned integers (0-18446744073709551615)
    Major Type 1: Negative integers (-1 to -18446744073709551616)  
    Major Type 2: Byte strings (binary data)
    Major Type 3: Text strings (UTF-8 encoded text)
    Major Type 4: Arrays (sequences of values)
    Major Type 5: Maps/objects (key-value pairs)
    Major Type 6: Semantic tags (type annotations)
    Major Type 7: Simple values and floats (booleans, null, floats, etc.)
    Each major type has minor type encodings for different sizes:
    - 0-23: Direct values
    - 24: 1-byte additional data
    - 25: 2-byte additional data  
    - 26: 4-byte additional data
    - 27: 8-byte additional data
    - 31: Indefinite length encoding (for strings, arrays, maps)
    This implementation focuses on JSON-compatible types and rejects non-JSON constructs.
*/
/*
    Well-Known Semantic Tag Values (IANA Registry: https://www.iana.org/assignments/cbor-tags/cbor-tags.xhtml)
    
    Date/Time (RFC 8949 Section 3.4):
    Tag 0: Standard date/time string (RFC 3339) - e.g., "2013-03-21T20:04:00Z"
    Tag 1: Epoch-based date/time (numeric, seconds since 1970-01-01 00:00:00 UTC)
    Tag 100: Number of days since epoch 1970-01-01
    Tag 1001: Extended time with additional precision
    Tag 1002: Duration
    Tag 1003: Period
    Tag 1004: RFC 3339 full-date string
    
    Numeric Encodings (RFC 8949 Section 3.4):
    Tag 2: Positive bignum (byte string, unsigned arbitrary-precision integer)
    Tag 3: Negative bignum (byte string, negative arbitrary-precision integer)
    Tag 4: Decimal fraction (array [exponent, mantissa] → mantissa × 10^exponent)
    Tag 5: Bigfloat (array [exponent, mantissa] → mantissa × 2^exponent)
    Tag 30: Rational number
    Tag 264: Decimal fraction with arbitrary exponent
    Tag 265: Bigfloat with arbitrary exponent
    Tag 268: Extended decimal fraction
    Tag 269: Extended bigfloat
    Tag 270: Extended rational number
    
    Encoded Content (RFC 8949 Section 3.4):
    Tag 21: Expected base64url encoding hint
    Tag 22: Expected base64 encoding hint
    Tag 23: Expected base16 encoding hint
    Tag 24: Encoded CBOR data item (nested CBOR)
    Tag 63: Encoded CBOR Sequence
    Tag 108: Expected base16 encoding (lowercase)
    Tag 262: Embedded JSON Object
    Tag 263: Hexadecimal string
    
    Network Addresses:
    Tag 32: URI (text string)
    Tag 33: base64url (text string)
    Tag 34: base64 (text string)
    Tag 36: MIME message
    Tag 37: Binary UUID (RFC 9562)
    Tag 48: IEEE MAC Address
    Tag 52: IPv4 address/prefix
    Tag 54: IPv6 address/prefix
    Tag 1048: IEEE OUI/CID
    
    Cryptographic (RFC 9052, RFC 9338):
    Tag 16: COSE_Encrypt0
    Tag 17: COSE_Mac0
    Tag 18: COSE_Sign1
    Tag 19: COSE_Countersignature
    Tag 61: CBOR Web Token (CWT)
    Tag 96: COSE_Encrypt
    Tag 97: COSE_Mac
    Tag 98: COSE_Sign
    
    Typed Arrays (RFC 8746):
    Tag 40-41: Multi-dimensional arrays
    Tag 64-87: Typed arrays (uint8, uint16, uint32, uint64, int8, int16, int32, int64, float16, float32, float64, float128 in big/little endian)
    
    Language & Text:
    Tag 35: Regular expression
    Tag 38: Language-tagged string
    Tag 266: Internationalized resource identifier (IRI)
    Tag 267: IRI reference
    Tag 21065: I-Regexp
    Tag 21066: ECMAScript RegExp
    
    Special Markers:
    Tag 55799: Self-described CBOR (magic number 0xd9d9f7)
    Tag 55800: File contains CBOR Sequences
    Tag 55801: File starts with CBOR-Labeled Non-CBOR Data
    
    This implementation preserves all semantic tags as {"_tag": tag_number, "_value": tagged_value}
    for round-trip compatibility, regardless of tag number. See IANA registry for complete list
    of registered tags (currently 0-18446744073709551615).
*/
/*
    Changes from Boost JSON CBOR example:
    - Added MajorTypes and MinorTypes enums for type safety
    - Renamed parse_cbor_type7 to parse_simple_value for clarity
    - Updated function signatures to use type aliases from utils.h (ByteBuffer, UInt64, UChar)
    - Added half-precision float support (IEEE 754 conversion)
    - Added overlong encoding rejection for RFC 8949 compliance (Section 3.9)
    - Added indefinite length encoding support for strings, arrays, objects, and byte strings
    - Modernized API to use std::string_view instead of raw pointers and offsets
    - Removed curr parameter from parsing functions for cleaner interfaces
    - Used auto type deduction extensively for better readability
    - Implemented storage reuse pattern (v.storage()) for efficient memory management
    - Used emplace operations (emplace_back, emplace) for in-place construction
    - Changed parse_number to return UInt64 instead of taking output parameter
    - Used boost::json::string with storage instead of std::string in parse_string
    - Merged array parsing loops with do-while(false) pattern for cleaner control flow
    - Cached MAJOR_TYPE results to avoid redundant bit shift operations
    - Removed duplicate array declarations and assignments for better performance
    - Added binary string support with base64 encoding wrapped as {"_binary": "base64..."}
    - Added semantic tag preservation as {"_tag": tag_number, "_value": tagged_value}
    - Implemented two-pass binary string parsing (calculate size, then encode directly)
    - Added hex-encoded marker constants to prevent naming conflicts with user data
    - Organized code into detail::parse and detail::serialize namespaces with TitleCamelCase
    - Added utility functions: IsByteString() and IsSemanticTag()
    - Added extraction functions: ExtractByteString() with template specializations for ByteVector, ByteBuffer, and ByteArray<N>
    - Added extraction functions: ExtractSemanticTag() with reference parameter and Optional return overloads
    - Full parse/serialize function parity for complete round-trip support
*/
#include <boost/json.hpp>
#include <boost/endian.hpp>
#include <boost/beast/core/detail/base64.hpp>
#include <cstdint>
#include <stdexcept>
#include <string_view>
#include <vector>
#include <cmath>
#include <limits>
#include "utils.h"

// Macro for more readable encode_simple_value calls: ENCODE_SV(type) instead of encode_sv<type>
#define ENCODE_SV(minor_type) encode_simple_value(static_cast<UChar>(minor_type))

// Macro for UChar cast to reduce verbosity: UC(value)
#define UC(value) static_cast<UChar>(value)

// Macro for pointer increment: INCR_PTR(PTR, X)
#define INCR_PTR(PTR, X) ((PTR) += (X))

// Macro for byte count calculation: BYTES_CONSUMED(RESULT, FIRST)
#define BYTES_CONSUMED(RESULT, FIRST) static_cast<std::size_t>((RESULT) - (FIRST))

// Macro for major type extraction: MAJOR_TYPE(BYTE)
#define MAJOR_TYPE(BYTE) static_cast<MajorTypes>((BYTE) >> 5)

// Macro for minor type extraction: MINOR_TYPE(BYTE)
#define MINOR_TYPE(BYTE) ((BYTE) & 31)

namespace cbor {
  namespace base64 = boost::beast::detail::base64;

  namespace detail {
    // =============================================================================
    // CBOR Constants
    // =============================================================================
    // Encoding thresholds for different number sizes
    constexpr UInt64 CBOR_ONE_BYTE_MAX = 255;            ///< Maximum value encodable in 1 byte
    constexpr UInt64 CBOR_TWO_BYTE_MAX = 65535;          ///< Maximum value encodable in 2 bytes
    constexpr UInt64 CBOR_FOUR_BYTE_MAX = 0xFFFFFFFFull; ///< Maximum value encodable in 4 bytes

    // Encoding markers
    constexpr UChar CBOR_BREAK_STOP_CODE = 0xFF;    ///< Break stop code for indefinite length
    constexpr UChar CBOR_DOUBLE_MARKER = 0xFB;      ///< Marker for double precision floats
    constexpr UChar CBOR_MAJOR_TYPE_MASK = 0xE0;    ///< Mask for major type (top 3 bits)
    constexpr UChar CBOR_MINOR_TYPE_MASK = 0x1F;    ///< Mask for minor type (bottom 5 bits)
    constexpr UChar CBOR_NEGATIVE_INT_START = 0x40; ///< Start of negative integer major type
    constexpr UInt64 CBOR_SIMPLE_VALUE_BASE = 224;  ///< Base for simple value encoding (7 << 5)

    // Half-precision float constants
    constexpr int CBOR_FLOAT16_MANTISSA_BITS = 10;   ///< Mantissa bits in half-precision float
    constexpr int CBOR_FLOAT16_MANTISSA_MAX = 1024;  ///< Maximum mantissa value (2^10)
    constexpr int CBOR_FLOAT16_EXPONENT_MAX = 31;    ///< Maximum exponent value
    constexpr int CBOR_FLOAT16_SUBNORMAL_EXP = -14;  ///< Exponent for subnormal numbers
    constexpr int CBOR_FLOAT16_NORMAL_EXP_BIAS = 15; ///< Exponent bias for normal numbers

    // Special marker keys for JSON round-trip encoding (hex-encoded to avoid conflicts)
    constexpr std::string_view CBOR_BINARY_MARKER =
     "0X5F62696E617279"; ///< "_binary" - Marker for binary data (base64 encoded)
    constexpr std::string_view CBOR_TAG_MARKER =
     "0X5F746167"; ///< "_tag" - Marker for semantic tag number
    constexpr std::string_view CBOR_VALUE_MARKER =
     "0X5F76616C7565"; ///< "_value" - Marker for tagged value
    // =============================================================================
    // CBOR MajorTypes Enumeration
    // =============================================================================
    /**
   * @brief CBOR MajorTypes as defined in RFC 8949
   */
    enum class MajorTypes : UChar {
      UNSIGNED_INTEGER = 0, ///< Major type 0: Unsigned integers
      NEGATIVE_INTEGER = 1, ///< Major type 1: Negative integers
      BYTE_STRING = 2,      ///< Major type 2: Byte strings (binary data)
      TEXT_STRING = 3,      ///< Major type 3: Text strings (UTF-8)
      ARRAY = 4,            ///< Major type 4: Arrays
      MAP = 5,              ///< Major type 5: Maps/objects
      SEMANTIC_TAG = 6,     ///< Major type 6: Semantic tags
      SIMPLE_VALUE = 7      ///< Major type 7: Simple values and floats
    };
    /**
   * @brief CBOR minor type encodings as defined in RFC 8949
   */
    enum class MinorTypes : UChar {
      DIRECT_MIN = 0,   ///< Minimum direct value (0)
      DIRECT_MAX = 23,  ///< Maximum direct value (23)
      ONE_BYTE = 24,    ///< 1-byte additional data follows
      TWO_BYTES = 25,   ///< 2-byte additional data follows
      FOUR_BYTES = 26,  ///< 4-byte additional data follows
      EIGHT_BYTES = 27, ///< 8-byte additional data follows
      INDEFINITE = 31   ///< Indefinite length encoding
    };
    /**
   * @brief CBOR simple value types as defined in RFC 8949
   */
    enum class SimpleValueTypes : UChar {
      FALSE = 20,        ///< Boolean false
      TRUE = 21,         ///< Boolean true
      NULL_VALUE = 22,   ///< Null value
      UNDEFINED = 23,    ///< Undefined value (not supported in JSON)
      SIMPLE_VALUE = 24, ///< Simple value with additional byte
      FLOAT16 = 25,      ///< IEEE 754 half-precision float (16 bits)
      FLOAT32 = 26,      ///< IEEE 754 single-precision float (32 bits)
      FLOAT64 = 27       ///< IEEE 754 double-precision float (64 bits)
    };
    // =============================================================================
    // CBOR Serialization Functions
    // =============================================================================
    namespace serialize {
      /**
     * @brief Serialize a CBOR number with major type and value
     * @param mt Major type enum
     * @param n Number value
     * @param out Output buffer to append serialized data
     */
      void Number(MajorTypes mt, UInt64 n, ByteBuffer& out);
      /**
     * @brief Serialize a CBOR string
     * @param sv String view to serialize
     * @param out Output buffer to append serialized data
     */
      void String(boost::json::string_view sv, ByteBuffer& out);
      /**
     * @brief Serialize a CBOR binary string
     * @param data Pointer to binary data
     * @param len Length of binary data
     * @param out Output buffer to append serialized data
     */
      void Binary(const void* data, std::size_t len, ByteBuffer& out);
      /**
     * @brief Serialize a JSON value to CBOR
     * @param jv JSON value to serialize
     * @param out Output buffer to append serialized data
     */
      void Value(const boost::json::value& jv, ByteBuffer& out);
      /**
     * @brief Serialize a CBOR array
     * @param ja JSON array to serialize
     * @param out Output buffer to append serialized data
     */
      void Array(const boost::json::array& ja, ByteBuffer& out);
      /**
     * @brief Serialize a CBOR object/map
     * @param jo JSON object to serialize
     * @param out Output buffer to append serialized data
     */
      void Object(const boost::json::object& jo, ByteBuffer& out);
      /**
     * @brief Serialize an unsigned integer
     * @param n Unsigned integer value
     * @param out Output buffer to append serialized data
     */
      void Unsigned(UInt64 n, ByteBuffer& out);
      /**
     * @brief Serialize a signed integer
     * @param n Signed integer value
     * @param out Output buffer to append serialized data
     */
      void Signed(std::int64_t n, ByteBuffer& out);
      /**
     * @brief Serialize a simple value (bool, null, float)
     * @param jv JSON value (must be bool, null, or double)
     * @param out Output buffer to append serialized data
     */
      void SimpleValue(const boost::json::value& jv, ByteBuffer& out);
      /**
     * @brief Serialize a semantic tag (from {"_tag": tag_number, "_value": tagged_value})
     * @param jv JSON value containing tag object
     * @param out Output buffer to append serialized data
     */
      void SemanticTag(const boost::json::value& jv, ByteBuffer& out);
    } // namespace serialize
    // =============================================================================
    // CBOR Parsing Functions
    // =============================================================================
    namespace parse {
      /**
     * @brief Parse CBOR number
     * @param sv String view containing the CBOR data (will be modified to remove consumed data)
     * @return The parsed number value
     */
      UInt64 Number(std::string_view& sv);
      /**
     * @brief Parse CBOR binary string
     * @param sv String view containing the CBOR data (will be modified to remove consumed data)
     * @param v Output JSON value (binary data encoded as base64 string)
     * @return Number of bytes consumed
     */
      std::size_t Binary(std::string_view& sv, boost::json::value& v);
      /**
     * @brief Parse CBOR text string
     * @param sv String view containing the CBOR data (will be modified to remove consumed data)
     * @param v Output JSON value
     * @return Number of bytes consumed
     */
      std::size_t String(std::string_view& sv, boost::json::value& v);
      /**
     * @brief Parse CBOR array
     * @param sv String view containing the CBOR data (will be modified to remove consumed data)
     * @param v Output JSON value
     * @return Number of bytes consumed
     */
      std::size_t Array(std::string_view& sv, boost::json::value& v);
      /**
     * @brief Parse CBOR object
     * @param sv String view containing the CBOR data (will be modified to remove consumed data)
     * @param v Output JSON value
     * @return Number of bytes consumed
     */
      std::size_t Object(std::string_view& sv, boost::json::value& v);
      /**
     * @brief Parse CBOR unsigned integer
     * @param sv String view containing the CBOR data (will be modified to remove consumed data)
     * @param v Output JSON value
     * @return Number of bytes consumed
     */
      std::size_t Unsigned(std::string_view& sv, boost::json::value& v);
      /**
     * @brief Parse CBOR signed integer (negative)
     * @param sv String view containing the CBOR data (will be modified to remove consumed data)
     * @param v Output JSON value
     * @return Number of bytes consumed
     */
      std::size_t Signed(std::string_view& sv, boost::json::value& v);
      /**
     * @brief Parse CBOR semantic tag (preserved as {"_tag": tag_number, "_value": tagged_value})
     * @param sv String view containing the CBOR data (will be modified to remove consumed data)
     * @param v Output JSON value containing wrapped tag object
     * @return Number of bytes consumed
     */
      std::size_t SemanticTag(std::string_view& sv, boost::json::value& v);
      /**
     * @brief Parse CBOR major type 7 (floats, bools, null)
     * @param sv String view containing the CBOR data (will be modified to remove consumed data)
     * @param v Output JSON value
     * @return Number of bytes consumed
     */
      std::size_t SimpleValue(std::string_view& sv, boost::json::value& v);
      /**
     * @brief Parse a CBOR value
     * @param sv String view containing the CBOR data (will be modified to remove consumed data)
     * @param v Output JSON value
     * @return Number of bytes consumed
     */
      std::size_t Value(std::string_view& sv, boost::json::value& v);
    } // namespace parse
    // =============================================================================
    // Error Handling Functions
    // =============================================================================
    /**
   * @brief Throw exception for unexpected end of input
   * @throws std::runtime_error
   */
    [[noreturn]] void throw_eof_error();
    /**
   * @brief Throw exception for format error
   * @param err Error message
   * @throws std::runtime_error
   */
    [[noreturn]] void throw_format_error(const char* err);
    /**
   * @brief Ensure at least n bytes are available in the input range
   * @param n Number of bytes required
   * @param sv String view containing the data
   * @throws std::runtime_error if insufficient bytes
   */
    void ensure(std::size_t n, std::string_view sv);
    // =============================================================================
    // Utility Functions
    // =============================================================================
    /**
   * @brief Check if a JSON value represents a CBOR byte string
   * @param jv JSON value to check
   * @return true if the value is an object with "_binary" marker key
   */
    bool IsByteString(const boost::json::value& jv);
    /**
   * @brief Check if a JSON value represents a CBOR semantic tag
   * @param jv JSON value to check
   * @return true if the value is an object with both "_tag" and "_value" marker keys
   */
    bool IsSemanticTag(const boost::json::value& jv);
    // =============================================================================
    // Function Definitions
    // =============================================================================
    namespace serialize {

      inline void Number(MajorTypes mt, UInt64 n, ByteBuffer& out) {
        auto mt_val = UC(mt);
        mt_val <<= 5;
        do {
          if (n <= UC(MinorTypes::DIRECT_MAX)) {
            out.push_back(UC(mt_val + n));
            break;
          }
          if (n < CBOR_ONE_BYTE_MAX + 1) {
            UChar data[] = { UC(mt_val + UC(MinorTypes::ONE_BYTE)), UC(n) };
            out.insert(out.end(), std::begin(data), std::end(data));
            break;
          }
          if (n < CBOR_TWO_BYTE_MAX + 1) {
            UChar data[] = { UC(mt_val + UC(MinorTypes::TWO_BYTES)), UC(n >> 8), UC(n) };
            out.insert(out.end(), std::begin(data), std::end(data));
            break;
          }
          if (n <= CBOR_FOUR_BYTE_MAX) {
            UChar data[5];
            data[0] = UC(mt_val + UC(MinorTypes::FOUR_BYTES));
            boost::endian::endian_store<std::uint32_t, 4, boost::endian::order::big>(
             data + 1, static_cast<std::uint32_t>(n));
            out.insert(out.end(), std::begin(data), std::end(data));
            break;
          }
          {
            UChar data[9];
            data[0] = UC(mt_val + UC(MinorTypes::EIGHT_BYTES));
            boost::endian::endian_store<UInt64, 8, boost::endian::order::big>(data + 1, n);
            out.insert(out.end(), std::begin(data), std::end(data));
          }
        } while (false);
      }

      inline void Unsigned(UInt64 n, ByteBuffer& out) {
        Number(MajorTypes::UNSIGNED_INTEGER, n, out);
      }

      inline void Signed(std::int64_t n, ByteBuffer& out) {
        if (n >= 0) {
          Number(MajorTypes::UNSIGNED_INTEGER, static_cast<UInt64>(n), out);
        } else {
          Number(MajorTypes::NEGATIVE_INTEGER, static_cast<UInt64>(~n), out);
        }
      }

      inline void SimpleValue(const boost::json::value& jv, ByteBuffer& out) {
        if (jv.is_bool()) {
          out.push_back(CBOR_SIMPLE_VALUE_BASE + UC(SimpleValueTypes::FALSE) + jv.get_bool());
        } else if (jv.is_null()) {
          out.push_back(CBOR_SIMPLE_VALUE_BASE + UC(SimpleValueTypes::NULL_VALUE));
        } else if (jv.is_double()) {
          UChar data[9];
          data[0] = (UC(MajorTypes::SIMPLE_VALUE) << 5) + UC(MinorTypes::EIGHT_BYTES);
          boost::endian::endian_store<double, 8, boost::endian::order::big>(
           data + 1, jv.get_double());
          out.insert(out.end(), std::begin(data), std::end(data));
        }
      }

      inline void SemanticTag(const boost::json::value& jv, ByteBuffer& out) {
        if (!jv.is_object()) {
          return;
        }

        const auto& jo = jv.get_object();
        auto tag_it = jo.find(CBOR_TAG_MARKER);
        auto value_it = jo.find(CBOR_VALUE_MARKER);

        if (tag_it != jo.end() && value_it != jo.end() && tag_it->value().is_uint64() &&
            jo.size() == 2) {
          // Serialize tag number
          auto tag_number = tag_it->value().get_uint64();
          Number(MajorTypes::SEMANTIC_TAG, tag_number, out);
          // Serialize tagged value
          Value(value_it->value(), out);
        }
      }

      inline void String(boost::json::string_view sv, ByteBuffer& out) {
        auto n = sv.size();
        Number(MajorTypes::TEXT_STRING, n, out);
        out.insert(out.end(), sv.data(), sv.data() + n);
      }

      inline void Binary(const void* data, std::size_t len, ByteBuffer& out) {
        Number(MajorTypes::BYTE_STRING, len, out);
        auto bytes = static_cast<const UChar*>(data);
        out.insert(out.end(), bytes, bytes + len);
      }

      inline void Array(const boost::json::array& ja, ByteBuffer& out) {
        auto n = ja.size();
        out.reserve(out.size() + n + 1);
        Number(MajorTypes::ARRAY, n, out);
        for (auto i = 0; i < n; ++i) {
          Value(ja[i], out);
        }
      }

      inline void Object(const boost::json::object& jo, ByteBuffer& out) {
        // Check for special _binary marker
        auto binary_it = jo.find(CBOR_BINARY_MARKER);
        if (binary_it != jo.end() && binary_it->value().is_string()) {
          // Decode base64 and serialize as CBOR binary string
          const auto& base64_str = binary_it->value().get_string();
          auto decoded_size = base64::decoded_size(base64_str.size());
          ByteBuffer binary_data(decoded_size);
          auto result = base64::decode(binary_data.data(), base64_str.data(), base64_str.size());
          Binary(binary_data.data(), result.first, out);
          return;
        }

        // Check for special _tag marker
        auto tag_it = jo.find(CBOR_TAG_MARKER);
        auto value_it = jo.find(CBOR_VALUE_MARKER);
        if (tag_it != jo.end() && value_it != jo.end() && tag_it->value().is_uint64() &&
            jo.size() == 2) {
          // Use dedicated SemanticTag function
          boost::json::value tag_obj(jo);
          SemanticTag(tag_obj, out);
          return;
        }

        // Regular object serialization
        auto n = jo.size();
        out.reserve(out.size() + 3 * n + 1);
        Number(MajorTypes::MAP, n, out);
        for (const auto& kv : jo) {
          String(kv.key(), out);
          Value(kv.value(), out);
        }
      }

      inline void Value(const boost::json::value& jv, ByteBuffer& out) {
        switch (jv.kind()) {
        case boost::json::kind::null:
        case boost::json::kind::bool_:
        case boost::json::kind::double_:
          SimpleValue(jv, out);
          break;
        case boost::json::kind::int64:
          Signed(jv.get_int64(), out);
          break;
        case boost::json::kind::uint64:
          Unsigned(jv.get_uint64(), out);
          break;
        case boost::json::kind::string:
          String(jv.get_string(), out);
          break;
        case boost::json::kind::array:
          Array(jv.get_array(), out);
          break;
        case boost::json::kind::object:
          Object(jv.get_object(), out);
          break;
        }
      }

    } // namespace serialize

    [[noreturn]] inline void throw_eof_error() {
      throw std::runtime_error("Unexpected end of input");
    }

    [[noreturn]] inline void throw_format_error(const char* err) {
      throw std::runtime_error(err);
    }

    inline void ensure(std::size_t n, std::string_view sv) {
      if (sv.size() < n) {
        throw_eof_error();
      }
    }

    inline bool IsByteString(const boost::json::value& jv) {
      if (!jv.is_object()) {
        return false;
      }
      const auto& jo = jv.get_object();
      return jo.contains(CBOR_BINARY_MARKER) && jo.size() == 1 &&
             jo.at(CBOR_BINARY_MARKER).is_string();
    }

    inline bool IsSemanticTag(const boost::json::value& jv) {
      if (!jv.is_object()) {
        return false;
      }
      const auto& jo = jv.get_object();
      return jo.contains(CBOR_TAG_MARKER) && jo.contains(CBOR_VALUE_MARKER) && jo.size() == 2 &&
             jo.at(CBOR_TAG_MARKER).is_uint64();
    }

    namespace parse {

      inline UInt64 Number(std::string_view& sv) {
        ensure(1, sv);
        auto curr = sv[0];
        sv.remove_prefix(1);
        auto cv = MINOR_TYPE(curr);
        UInt64 n;
        do {
          if (cv <= UC(MinorTypes::DIRECT_MAX)) {
            n = cv;
            break;
          }
          if (cv == UC(MinorTypes::ONE_BYTE)) {
            ensure(1, sv);
            n = static_cast<UChar>(sv[0]);
            sv.remove_prefix(1);
            // Reject overlong encodings (RFC 8949 Section 3.9)
            if (n < UC(MinorTypes::ONE_BYTE)) {
              throw_format_error("Overlong encoding: value should use shortest form");
            }
            break;
          }
          if (cv == UC(MinorTypes::TWO_BYTES)) {
            ensure(2, sv);
            auto data = reinterpret_cast<const UChar*>(sv.data());
            n = boost::endian::load_big_u16(data);
            sv.remove_prefix(2);
            // Reject overlong encodings
            if (n < CBOR_ONE_BYTE_MAX + 1) {
              throw_format_error("Overlong encoding: value should use shorter form");
            }
            break;
          }
          if (cv == UC(MinorTypes::FOUR_BYTES)) {
            ensure(4, sv);
            auto data = reinterpret_cast<const UChar*>(sv.data());
            n = boost::endian::load_big_u32(data);
            sv.remove_prefix(4);
            // Reject overlong encodings
            if (n < CBOR_TWO_BYTE_MAX + 1) {
              throw_format_error("Overlong encoding: value should use shorter form");
            }
            break;
          }
          if (cv == UC(MinorTypes::EIGHT_BYTES)) {
            ensure(8, sv);
            auto data = reinterpret_cast<const UChar*>(sv.data());
            n = boost::endian::load_big_u64(data);
            sv.remove_prefix(8);
            // Reject overlong encodings
            if (n <= CBOR_FOUR_BYTE_MAX) {
              throw_format_error("Overlong encoding: value should use shorter form");
            }
            break;
          }
          if (cv == UC(MinorTypes::INDEFINITE)) {
            // indefinite length - handled by caller
            n = UInt64(-1);
            break;
          }
          // else
          throw_format_error("Invalid minor type");
        } while (false);
        return n;
      }

      inline std::size_t Binary(std::string_view& sv, boost::json::value& v) {
        auto original_size = sv.size();
        ensure(1, sv);
        auto head = static_cast<UChar>(sv[0]);

        bool const indefinite = (MINOR_TYPE(head) == UC(MinorTypes::INDEFINITE));

        // ──────── First pass: calculate total raw byte length (no modification) ────────
        auto counter = sv;
        std::size_t total_bytes = 0;

        if (indefinite) {
          counter.remove_prefix(1); // skip 0x5F
          while (true) {
            ensure(1, counter);
            auto next = static_cast<UChar>(counter[0]);
            if (next == CBOR_BREAK_STOP_CODE) {
              counter.remove_prefix(1);
              break;
            }
            if (MAJOR_TYPE(next) != MajorTypes::BYTE_STRING) {
              throw_format_error("Invalid chunk in indefinite byte string");
            }

            auto len = Number(counter);
            ensure(len, counter);
            total_bytes += len;
            counter.remove_prefix(len);
          }
        } else {
          // Definite: read length and advance counter past head+length
          auto len = Number(counter);
          ensure(len, counter);
          total_bytes = len;
        }

        // ──────── Allocate exact base64 output once ────────
        auto encoded_size = base64::encoded_size(total_bytes);
        boost::json::string base64_str(v.storage());
        base64_str.resize(encoded_size);
        char* out_ptr = const_cast<char*>(base64_str.data());

        // ──────── Second pass: consume and encode directly ────────
        if (indefinite) {
          sv.remove_prefix(1); // consume 0x5F
          while (true) {
            ensure(1, sv);
            auto next = static_cast<UChar>(sv[0]);
            if (next == CBOR_BREAK_STOP_CODE) {
              sv.remove_prefix(1);
              break;
            }
            auto len = Number(sv);
            ensure(len, sv);
            auto written = base64::encode(out_ptr, sv.data(), len);
            out_ptr += written;
            sv.remove_prefix(len);
          }
        } else {
          // Definite length: consume head+length, then encode payload
          Number(sv); // consume head and length bytes (already calculated in first pass)
          ensure(total_bytes, sv);
          base64::encode(out_ptr, sv.data(), total_bytes);
          sv.remove_prefix(total_bytes);
        }

        // Wrap in {"_binary": "..."}
        boost::json::object obj(v.storage());
        obj.emplace(CBOR_BINARY_MARKER, std::move(base64_str));
        v = std::move(obj);

        return original_size - sv.size();
      }

      inline std::size_t String(std::string_view& sv, boost::json::value& v) {
        auto original_size = sv.size();
        boost::json::string result(v.storage());
        ensure(1, sv);
        auto curr = sv[0];
        if (MINOR_TYPE(curr) == UC(MinorTypes::INDEFINITE)) {
          sv.remove_prefix(1);
          while (true) {
            ensure(1, sv);
            auto next = sv[0];
            if (next == CBOR_BREAK_STOP_CODE) {
              sv.remove_prefix(1);
              break;
            }
            if (MAJOR_TYPE(next) != MAJOR_TYPE(curr)) {
              throw_format_error("Invalid chunk type in indefinite length string");
            }
            auto len = Number(sv);
            ensure(len, sv);
            result.append(sv.data(), sv.data() + len);
            sv.remove_prefix(len);
          }
        } else {
          // Definite length string
          auto len = Number(sv);
          ensure(len, sv);
          result.assign(sv.data(), sv.data() + len);
          sv.remove_prefix(len);
        }
        // boost::json validates UTF-8 when creating string values
        v = std::move(result);
        return original_size - sv.size();
      }

      inline std::size_t Array(std::string_view& sv, boost::json::value& v) {
        auto original_size = sv.size();
        ensure(1, sv);
        auto curr = sv[0];
        boost::json::array a(v.storage());
        if (MINOR_TYPE(curr) == UC(MinorTypes::INDEFINITE)) {
          sv.remove_prefix(1);
          while (true) {
            ensure(1, sv);
            auto next = static_cast<UChar>(sv[0]);
            if (next == CBOR_BREAK_STOP_CODE) { // break stop code
              sv.remove_prefix(1);
              break;
            }
            boost::json::value element(v.storage());
            Value(sv, element);
            a.emplace_back(std::move(element));
          }
        } else {
          // Definite length array
          auto len = Number(sv);
          a.reserve(len);
          for (auto i = 0; i < len; ++i) {
            boost::json::value element(v.storage());
            Value(sv, element);
            a.emplace_back(std::move(element));
          }
        }
        v = std::move(a);
        return original_size - sv.size();
      }

      inline std::size_t Object(std::string_view& sv, boost::json::value& v) {
        auto original_size = sv.size();
        ensure(1, sv);
        auto curr = sv[0];
        boost::json::object o(v.storage());
        if (MINOR_TYPE(curr) == UC(MinorTypes::INDEFINITE)) {
          sv.remove_prefix(1);
          while (true) {
            ensure(1, sv);
            auto next = static_cast<UChar>(sv[0]);
            if (next == CBOR_BREAK_STOP_CODE) { // break stop code
              sv.remove_prefix(1);
              break;
            }
            if (MAJOR_TYPE(next) != MajorTypes::TEXT_STRING) {
              throw_format_error("Object keys must be strings");
            }
            auto len = Number(sv);
            ensure(len, sv);
            boost::json::string key(v.storage());
            key.assign(sv.data(), sv.data() + len);
            sv.remove_prefix(len);
            boost::json::value val(v.storage());
            Value(sv, val);
            o.emplace(std::move(key), std::move(val));
          }
        } else {
          // Definite length object
          auto len = Number(sv);
          o.reserve(len);
          for (auto i = 0; i < len; ++i) {
            ensure(1, sv);
            auto next = static_cast<UChar>(sv[0]);
            if (MAJOR_TYPE(next) != MajorTypes::TEXT_STRING) {
              throw_format_error("Object keys must be strings");
            }
            auto key_len = Number(sv);
            ensure(key_len, sv);
            boost::json::string key(v.storage());
            key.assign(sv.data(), sv.data() + key_len);
            sv.remove_prefix(key_len);
            boost::json::value val(v.storage());
            Value(sv, val);
            o.emplace(std::move(key), std::move(val));
          }
        }
        v = std::move(o);
        return original_size - sv.size();
      }

      inline std::size_t Unsigned(std::string_view& sv, boost::json::value& v) {
        auto original_size = sv.size();
        v = Number(sv);
        return original_size - sv.size();
      }

      inline std::size_t Signed(std::string_view& sv, boost::json::value& v) {
        auto original_size = sv.size();
        auto n = Number(sv);
        v = static_cast<std::int64_t>(~n);
        return original_size - sv.size();
      }

      inline std::size_t SemanticTag(std::string_view& sv, boost::json::value& v) {
        auto original_size = sv.size();
        auto tag_number = Number(sv);

        // Parse the tagged value
        boost::json::value tagged_value(v.storage());
        Value(sv, tagged_value);

        // Wrap in object with _tag marker for round-trip preservation
        boost::json::object obj(v.storage());
        obj.emplace(CBOR_TAG_MARKER, tag_number);
        obj.emplace(CBOR_VALUE_MARKER, std::move(tagged_value));
        v = std::move(obj);

        return original_size - sv.size();
      }

      inline std::size_t SimpleValue(std::string_view& sv, boost::json::value& v) {
        auto original_size = sv.size();
        ensure(1, sv);
        auto curr = sv[0];
        sv.remove_prefix(1);
        switch (MINOR_TYPE(curr)) {
        case UC(SimpleValueTypes::FALSE):
          v = false;
          return original_size - sv.size();
        case UC(SimpleValueTypes::TRUE):
          v = true;
          return original_size - sv.size();
        case UC(SimpleValueTypes::NULL_VALUE):
          v = nullptr;
          return original_size - sv.size();
        case UC(SimpleValueTypes::UNDEFINED): // undefined
          throw_format_error("Undefined value not supported");
        case UC(SimpleValueTypes::SIMPLE_VALUE): // simple value
          ensure(1, sv);
          {
            auto simple_value = static_cast<UChar>(sv[0]);
            sv.remove_prefix(1);
            // Only support simple values that are JSON-compatible
            // 0xFF (255) is not a valid simple value in this context
            throw_format_error("Simple value not supported");
          }
        case UC(SimpleValueTypes::FLOAT16): // half-precision float
          {
            ensure(2, sv);
            auto data = reinterpret_cast<const UChar*>(sv.data());
            // Convert half-precision float to double
            auto half = boost::endian::load_big_u16(data);
            sv.remove_prefix(2);
            // IEEE 754 half-precision to double conversion
            auto sign = (half >> 15) & 0x1;
            auto exponent = (half >> 10) & CBOR_FLOAT16_EXPONENT_MAX;
            auto mantissa = half & (CBOR_FLOAT16_MANTISSA_MAX - 1);
            auto result = 0.0;
            if (exponent == 0) {
              if (mantissa == 0) {
                result = sign ? -0.0 : 0.0;
              } else {
                // Subnormal number
                result =
                 std::ldexp(mantissa / CBOR_FLOAT16_MANTISSA_MAX, CBOR_FLOAT16_SUBNORMAL_EXP) *
                 (sign ? -1.0 : 1.0);
              }
            } else if (exponent == 31) {
              if (mantissa == 0) {
                result = sign ? -std::numeric_limits<double>::infinity() :
                                std::numeric_limits<double>::infinity();
              } else {
                result = std::numeric_limits<double>::quiet_NaN();
              }
            } else {
              // Normal number
              result =
               std::ldexp((CBOR_FLOAT16_MANTISSA_MAX + mantissa) / CBOR_FLOAT16_MANTISSA_MAX,
                exponent - CBOR_FLOAT16_NORMAL_EXP_BIAS) *
               (sign ? -1.0 : 1.0);
            }
            v = result;
            return original_size - sv.size();
          }
        case UC(SimpleValueTypes::FLOAT32): // float
          {
            ensure(4, sv);
            auto data = reinterpret_cast<const UChar*>(sv.data());
            auto w = boost::endian::endian_load<float, 4, boost::endian::order::big>(data);
            sv.remove_prefix(4);
            v = w;
            return original_size - sv.size();
          }
        case UC(SimpleValueTypes::FLOAT64): // double
          {
            ensure(8, sv);
            auto data = reinterpret_cast<const UChar*>(sv.data());
            auto w = boost::endian::endian_load<double, 8, boost::endian::order::big>(data);
            sv.remove_prefix(8);
            v = w;
            return original_size - sv.size();
          }
        default:
          throw_format_error("Invalid minor type for major type 7");
        }
      }

      inline std::size_t Value(std::string_view& sv, boost::json::value& v) {
        auto original_size = sv.size();
        ensure(1, sv);
        const auto curr = static_cast<UChar>(sv[0]);
        switch (MAJOR_TYPE(curr)) {
        case MajorTypes::UNSIGNED_INTEGER:
          Unsigned(sv, v);
          break;
        case MajorTypes::NEGATIVE_INTEGER:
          Signed(sv, v);
          break;
        case MajorTypes::BYTE_STRING:
          Binary(sv, v);
          break;
        case MajorTypes::TEXT_STRING:
          String(sv, v);
          break;
        case MajorTypes::ARRAY:
          Array(sv, v);
          break;
        case MajorTypes::MAP:
          Object(sv, v);
          break;
        case MajorTypes::SEMANTIC_TAG:
          SemanticTag(sv, v);
          break;
        case MajorTypes::SIMPLE_VALUE:
          SimpleValue(sv, v);
          break;
        default:
          // This should be unreachable in valid CBOR
          throw_format_error("Invalid major type");
        }
        return original_size - sv.size();
      }

    } // namespace parse

  } // namespace detail
  // =============================================================================
  // Public API Functions
  // =============================================================================
  /**
   * @brief Serialize a JSON value to CBOR format
   * @param jv JSON value to serialize
   * @param out Output buffer to append serialized data
   */
  inline void serialize_cbor_value(const boost::json::value& jv, ByteBuffer& out) {
    detail::serialize::Value(jv, out);
  }

  /**
   * @brief Parse a CBOR value and return bytes consumed
   * @param sv String view containing the CBOR data
   * @param v Output JSON value
   * @return Number of bytes consumed from input
   */
  inline std::size_t parse_cbor_value(std::string_view sv, boost::json::value& v) {
    return detail::parse::Value(sv, v);
  }

  // =============================================================================
  // Byte String Extraction Functions
  // =============================================================================
  /**
   * @brief Extract byte string from CBOR-wrapped JSON value (generic template)
   * @tparam T Target container type
   * @param jv JSON value containing CBOR byte string
   * @param out Output container to receive decoded bytes
   * @return true if extraction successful, false otherwise
   */
  template <typename T> inline bool ExtractByteString(const boost::json::value& jv, T& out) {
    if (!detail::IsByteString(jv)) {
      return false;
    }

    const auto& jo = jv.get_object();
    const auto& base64_str = jo.at(detail::CBOR_BINARY_MARKER).get_string();

    auto decoded_size = base64::decoded_size(base64_str.size());
    ByteBuffer temp(decoded_size);
    auto result = base64::decode(temp.data(), base64_str.data(), base64_str.size());

    out.clear();
    out.reserve(result.first);
    for (std::size_t i = 0; i < result.first; ++i) {
      out.push_back(static_cast<typename T::value_type>(temp[i]));
    }

    return true;
  }

  /**
   * @brief Extract byte string to ByteVector (std::vector<std::byte>)
   * @param jv JSON value containing CBOR byte string
   * @param out Output ByteVector to receive decoded bytes
   * @return true if extraction successful, false otherwise
   */
  template <>
  inline bool ExtractByteString<ByteVector>(const boost::json::value& jv, ByteVector& out) {
    if (!detail::IsByteString(jv)) {
      return false;
    }

    const auto& jo = jv.get_object();
    const auto& base64_str = jo.at(detail::CBOR_BINARY_MARKER).get_string();

    auto decoded_size = base64::decoded_size(base64_str.size());
    out.resize(decoded_size);
    auto result =
     base64::decode(reinterpret_cast<char*>(out.data()), base64_str.data(), base64_str.size());
    out.resize(result.first);

    return true;
  }

  /**
   * @brief Extract byte string to ByteBuffer (std::vector<unsigned char>)
   * @param jv JSON value containing CBOR byte string
   * @param out Output ByteBuffer to receive decoded bytes
   * @return true if extraction successful, false otherwise
   */
  template <>
  inline bool ExtractByteString<ByteBuffer>(const boost::json::value& jv, ByteBuffer& out) {
    if (!detail::IsByteString(jv)) {
      return false;
    }

    const auto& jo = jv.get_object();
    const auto& base64_str = jo.at(detail::CBOR_BINARY_MARKER).get_string();

    auto decoded_size = base64::decoded_size(base64_str.size());
    out.resize(decoded_size);
    auto result = base64::decode(out.data(), base64_str.data(), base64_str.size());
    out.resize(result.first);

    return true;
  }

  /**
   * @brief Extract byte string to fixed-size ByteArray (std::array<std::byte, N>)
   * @tparam N Size of the array
   * @param jv JSON value containing CBOR byte string
   * @param out Output ByteArray to receive decoded bytes
   * @return true if extraction successful and size matches, false otherwise
   */
  template <std::size_t N>
  inline bool ExtractByteString(const boost::json::value& jv, ByteArray<N>& out) {
    if (!detail::IsByteString(jv)) {
      return false;
    }

    const auto& jo = jv.get_object();
    const auto& base64_str = jo.at(detail::CBOR_BINARY_MARKER).get_string();

    auto decoded_size = base64::decoded_size(base64_str.size());
    if (decoded_size > N) {
      return false; // Data too large for fixed array
    }

    ByteBuffer temp(decoded_size);
    auto result = base64::decode(temp.data(), base64_str.data(), base64_str.size());

    if (result.first > N) {
      return false; // Decoded data too large
    }

    std::fill(out.begin(), out.end(), std::byte{ 0 });
    for (std::size_t i = 0; i < result.first; ++i) {
      out[i] = static_cast<std::byte>(temp[i]);
    }

    return true;
  }

  // =============================================================================
  // Semantic Tag Extraction Function
  // =============================================================================
  /**
   * @brief Extract semantic tag from CBOR-wrapped JSON value
   * @param jv JSON value containing CBOR semantic tag
   * @param out Output SemanticTag (tuple<UInt64, Value>) to receive tag number and value
   * @return true if extraction successful, false otherwise
   */
  inline bool ExtractSemanticTag(const boost::json::value& jv, SemanticTag& out) {
    if (!detail::IsSemanticTag(jv)) {
      return false;
    }

    const auto& jo = jv.get_object();
    auto tag_number = jo.at(detail::CBOR_TAG_MARKER).as_uint64();
    const auto& tagged_value = jo.at(detail::CBOR_VALUE_MARKER);

    out = std::make_tuple(tag_number, tagged_value);
    return true;
  }

  /**
   * @brief Extract semantic tag and return optional SemanticTag
   * @param jv JSON value containing CBOR semantic tag
   * @return Optional<SemanticTag> containing tag number and value, or empty if not a semantic tag
   */
  inline Optional<SemanticTag> ExtractSemanticTag(const boost::json::value& jv) {
    if (!detail::IsSemanticTag(jv)) {
      return std::nullopt;
    }

    const auto& jo = jv.get_object();
    auto tag_number = jo.at(detail::CBOR_TAG_MARKER).as_uint64();
    const auto& tagged_value = jo.at(detail::CBOR_VALUE_MARKER);

    return std::make_tuple(tag_number, tagged_value);
  }

} // namespace cbor

#undef ENCODE_SV
#undef UC
#undef INCR_PTR
#undef BYTES_CONSUMED
#undef MAJOR_TYPE
#undef MINOR_TYPE

#endif // CBOR_H_INCLUDED
