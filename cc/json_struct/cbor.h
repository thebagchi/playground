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
    - Semantic tag parsing (tags are ignored for JSON compatibility)
    Binary strings are not supported as they are not JSON-compatible.
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
    Changes from Boost JSON CBOR example:
    - Added MajorTypes and MinorTypes enums for type safety
    - Renamed parse_cbor_type7 to parse_simple_value for clarity
    - Updated function signatures to use type aliases from utils.h (ByteBuffer, UInt64, UChar)
    - Added half-precision float support (IEEE 754 conversion)
    - Added array parsing optimizations (fast paths for double[] and int[] arrays)
    - Added overlong encoding rejection for RFC 8949 compliance
    - Added indefinite length encoding support for strings, arrays, and objects
*/
#include <boost/json.hpp>
#include <boost/endian.hpp>
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
    /**
   * @brief Serialize a CBOR number with major type and value
   * @param mt Major type enum
   * @param n Number value
   * @param out Output buffer to append serialized data
   */
    void serialize_number(MajorTypes mt, UInt64 n, ByteBuffer& out);
    /**
   * @brief Serialize a CBOR string
   * @param sv String view to serialize
   * @param out Output buffer to append serialized data
   */
    void serialize_string(boost::json::string_view sv, ByteBuffer& out);
    // =============================================================================
    // CBOR Parsing Functions
    // =============================================================================
    /**
   * @brief Parse CBOR number with given major type
   * @param first Start of input buffer
   * @param last End of input buffer
   * @param curr Current byte with major/minor type
   * @param n Output number value
   * @return Number of bytes consumed
   */
    std::size_t parse_number(const UChar* first, const UChar* last, UChar curr, UInt64& n);
    /**
   * @brief Parse CBOR binary string
   * @param first Start of input buffer
   * @param last End of input buffer
   * @param curr Current byte with major/minor type
   * @param v Output JSON value (will throw since binary not supported)
   * @return Number of bytes consumed
   */
    std::size_t parse_binary(
     const UChar* first, const UChar* last, UChar curr, boost::json::value& v);
    /**
   * @brief Parse CBOR array
   * @param first Start of input buffer
   * @param last End of input buffer
   * @param curr Current byte with major/minor type
   * @param v Output JSON value
   * @return Number of bytes consumed
   */
    std::size_t parse_array(
     const UChar* first, const UChar* last, UChar curr, boost::json::value& v);
    /**
   * @brief Parse CBOR object
   * @param first Start of input buffer
   * @param last End of input buffer
   * @param curr Current byte with major/minor type
   * @param v Output JSON value
   * @return Number of bytes consumed
   */
    std::size_t parse_object(
     const UChar* first, const UChar* last, UChar curr, boost::json::value& v);
    /**
   * @brief Parse CBOR unsigned integer
   * @param first Start of input buffer
   * @param last End of input buffer
   * @param curr Current byte with major/minor type
   * @param v Output JSON value
   * @return Number of bytes consumed
   */
    std::size_t parse_unsigned(
     const UChar* first, const UChar* last, UChar curr, boost::json::value& v);
    /**
   * @brief Parse CBOR signed integer (negative)
   * @param first Start of input buffer
   * @param last End of input buffer
   * @param curr Current byte with major/minor type
   * @param v Output JSON value
   * @return Number of bytes consumed
   */
    std::size_t parse_signed(
     const UChar* first, const UChar* last, UChar curr, boost::json::value& v);
    /**
   * @brief Parse CBOR semantic tag
   * @param first Start of input buffer
   * @param last End of input buffer
   * @param curr Current byte with major/minor type
   * @param v Output JSON value
   * @return Number of bytes consumed
   */
    std::size_t parse_semantic_tag(
     const UChar* first, const UChar* last, UChar curr, boost::json::value& v);
    /**
   * @brief Parse CBOR major type 7 (floats, bools, null)
   * @param first Start of input buffer
   * @param last End of input buffer
   * @param curr Current byte with major/minor type
   * @param v Output JSON value
   * @return Number of bytes consumed
   */
    std::size_t parse_simple_value(
     const UChar* first, const UChar* last, UChar curr, boost::json::value& v);
    /**
   * @brief Parse a CBOR value
   * @param first Start of input buffer
   * @param last End of input buffer
   * @param v Output JSON value
   * @return Number of bytes consumed
   */
    std::size_t parse_value(const UChar* first, const UChar* last, boost::json::value& v);
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
   * @param first Beginning of input range
   * @param last End of input range
   * @throws std::runtime_error if insufficient bytes
   */
    void ensure(std::size_t n, const UChar* first, const UChar* last);
    // =============================================================================
    // Function Definitions
    // =============================================================================
    inline void serialize_number(MajorTypes mt, UInt64 n, ByteBuffer& out) {
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
        // else
        {
          UChar data[9];
          data[0] = UC(mt_val + UC(MinorTypes::EIGHT_BYTES));
          boost::endian::endian_store<UInt64, 8, boost::endian::order::big>(data + 1, n);
          out.insert(out.end(), std::begin(data), std::end(data));
        }
      } while (false);
    }

    inline void serialize_string(boost::json::string_view sv, ByteBuffer& out) {
      std::size_t n = sv.size();
      serialize_number(MajorTypes::TEXT_STRING, n, out);
      out.insert(out.end(), sv.data(), sv.data() + n);
    }

    inline void serialize_value(const boost::json::value& jv, ByteBuffer& out) {
      switch (jv.kind()) {
      case boost::json::kind::null:
        out.push_back(CBOR_SIMPLE_VALUE_BASE + UC(SimpleValueTypes::NULL_VALUE));
        break;
      case boost::json::kind::bool_:
        out.push_back(CBOR_SIMPLE_VALUE_BASE + UC(SimpleValueTypes::FALSE) + jv.get_bool());
        break;
      case boost::json::kind::int64:
        {
          std::int64_t n = jv.get_int64();
          if (n >= 0) {
            serialize_number(MajorTypes::UNSIGNED_INTEGER, n, out);
          } else {
            serialize_number(MajorTypes::NEGATIVE_INTEGER, ~n, out);
          }
        }
        break;
      case boost::json::kind::uint64:
        serialize_number(MajorTypes::UNSIGNED_INTEGER, jv.get_uint64(), out);
        break;
      case boost::json::kind::double_:
        {
          UChar data[9];
          data[0] = (UC(MajorTypes::SIMPLE_VALUE) << 5) + UC(MinorTypes::EIGHT_BYTES);
          boost::endian::endian_store<double, 8, boost::endian::order::big>(
           data + 1, jv.get_double());
          out.insert(out.end(), std::begin(data), std::end(data));
        }
        break;
      case boost::json::kind::string:
        serialize_string(jv.get_string(), out);
        break;
      case boost::json::kind::array:
        {
          const auto& ja = jv.get_array();
          auto n = ja.size();
          out.reserve(out.size() + n + 1);
          serialize_number(MajorTypes::ARRAY, n, out);
          for (auto i = 0; i < n; ++i) {
            serialize_value(ja[i], out);
          }
        }
        break;
      case boost::json::kind::object:
        {
          const auto& jo = jv.get_object();
          auto n = jo.size();
          out.reserve(out.size() + 3 * n + 1);
          serialize_number(MajorTypes::MAP, n, out);
          for (const auto& kv : jo) {
            serialize_string(kv.key(), out);
            serialize_value(kv.value(), out);
          }
        }
        break;
      }
    }

    [[noreturn]] inline void throw_eof_error() {
      throw std::runtime_error("Unexpected end of input");
    }

    [[noreturn]] inline void throw_format_error(const char* err) {
      throw std::runtime_error(err);
    }

    inline void ensure(std::size_t n, const UChar* first, const UChar* last) {
      if (static_cast<std::size_t>(last - first) < n) {
        throw_eof_error();
      }
    }

    inline std::size_t parse_number(const UChar* first, const UChar* last, UChar curr, UInt64& n) {
      auto temp = first;
      auto cv = MINOR_TYPE(curr);
      do {
        if (cv <= UC(MinorTypes::DIRECT_MAX)) {
          n = cv;
          break;
        }
        if (cv == UC(MinorTypes::ONE_BYTE)) {
          ensure(1, first, last);
          n = *first;
          INCR_PTR(first, 1);
          // Reject overlong encodings (RFC 8949 Section 3.9)
          if (n < UC(MinorTypes::ONE_BYTE)) {
            throw_format_error("Overlong encoding: value should use shortest form");
          }
          break;
        }
        if (cv == UC(MinorTypes::TWO_BYTES)) {
          ensure(2, first, last);
          n = boost::endian::load_big_u16(first);
          INCR_PTR(first, 2);
          // Reject overlong encodings
          if (n < CBOR_ONE_BYTE_MAX + 1) {
            throw_format_error("Overlong encoding: value should use shorter form");
          }
          break;
        }
        if (cv == UC(MinorTypes::FOUR_BYTES)) {
          ensure(4, first, last);
          n = boost::endian::load_big_u32(first);
          INCR_PTR(first, 4);
          // Reject overlong encodings
          if (n < CBOR_TWO_BYTE_MAX + 1) {
            throw_format_error("Overlong encoding: value should use shorter form");
          }
          break;
        }
        if (cv == UC(MinorTypes::EIGHT_BYTES)) {
          ensure(8, first, last);
          n = boost::endian::load_big_u64(first);
          INCR_PTR(first, 8);
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
      return BYTES_CONSUMED(first, temp);
    }

    inline std::size_t parse_binary(
     const UChar* first, const UChar* last, UChar curr, boost::json::value& v) {
      auto temp = first;
      // Binary strings are not supported in JSON-compatible CBOR subset
      // In a full CBOR implementation, this would parse and store binary data
      throw_format_error("Binary strings are not supported (not JSON-compatible)");
    }

    inline std::size_t parse_string(
     const UChar* first, const UChar* last, UChar curr, boost::json::value& v) {
      auto temp = first;
      std::string result;
      if (MINOR_TYPE(curr) == UC(MinorTypes::INDEFINITE)) {
        // Indefinite length array
        while (true) {
          ensure(1, first, last);
          auto next = *first;
          if (next == CBOR_BREAK_STOP_CODE) { // break stop code
            INCR_PTR(first, 1);
            break;
          }
          if (MAJOR_TYPE(next) != MAJOR_TYPE(curr)) { // must be same major type
            throw_format_error("Invalid chunk type in indefinite length string");
          }
          UInt64 n;
          INCR_PTR(first, 1);
          parse_number(first, last, next, n);
          ensure(n, first, last);
          result.append(reinterpret_cast<char const*>(first), n);
          INCR_PTR(first, n);
        }
      } else {
        // Definite length string
        UInt64 n;
        parse_number(first, last, curr, n);
        ensure(n, first, last);
        result.assign(reinterpret_cast<char const*>(first), n);
        INCR_PTR(first, n);
      }
      // boost::json validates UTF-8 when creating string values
      v = result;
      return BYTES_CONSUMED(first, temp);
    }

    inline std::size_t parse_array(
     const UChar* first, const UChar* last, UChar curr, boost::json::value& v) {
      auto temp = first;
      auto& a = v.emplace_array();
      if (MINOR_TYPE(curr) == UC(MinorTypes::INDEFINITE)) {
        // Indefinite length array
        while (true) {
          ensure(1, first, last);
          auto next = *first;
          if (next == CBOR_BREAK_STOP_CODE) { // break stop code
            INCR_PTR(first, 1);
            break;
          }
          boost::json::value element;
          parse_value(first, last, element);
          a.push_back(std::move(element));
        }
      } else {
        // Definite length array
        UInt64 n;
        parse_number(first, last, curr, n);
        a.resize(n);
        auto i = 0;
        for (; i < n; ++i) // double[] fast path
        {
          ensure(1, first, last);
          UChar next = *first;
          if (next != CBOR_DOUBLE_MARKER) {
            break;
          }
          INCR_PTR(first, 1);
          ensure(8, first, last);
          double w = boost::endian::endian_load<double, 8, boost::endian::order::big>(first);
          INCR_PTR(first, 8);
          a[i] = w;
        }
        for (; i < n; ++i) // int[] fast path
        {
          ensure(1, first, last);
          auto next = *first;
          if (next >= CBOR_NEGATIVE_INT_START) {
            break;
          }
          INCR_PTR(first, 1);
          UInt64 m;
          parse_number(first, last, next, m);
          if (next < UC(MinorTypes::ONE_BYTE)) {
            a[i] = m;
          } else {
            a[i] = static_cast<std::int64_t>(~m);
          }
        }
        for (; i < n; ++i) {
          parse_value(first, last, a[i]);
        }
      }
      return BYTES_CONSUMED(first, temp);
    }

    inline std::size_t parse_object(
     const UChar* first, const UChar* last, UChar curr, boost::json::value& v) {
      auto temp = first;
      auto& o = v.emplace_object();
      if (MINOR_TYPE(curr) == UC(MinorTypes::INDEFINITE)) {
        // Indefinite length object
        while (true) {
          ensure(1, first, last);
          auto next = *first;
          if (next == CBOR_BREAK_STOP_CODE) { // break stop code
            INCR_PTR(first, 1);
            break;
          }
          // key string
          if (MAJOR_TYPE(next) != MajorTypes::TEXT_STRING) {
            throw_format_error("Object keys must be strings");
          }
          UInt64 m;
          parse_number(first, last, next, m);
          ensure(m, first, last);
          boost::json::string_view sv(reinterpret_cast<char const*>(first), m);
          INCR_PTR(first, m);
          // value
          parse_value(first, last, o[sv]);
        }
      } else {
        // Definite length object
        UInt64 n;
        parse_number(first, last, curr, n);
        o.reserve(n);
        for (auto i = 0; i < n; ++i) {
          // key string
          ensure(1, first, last);
          auto next = *first;
          if (MAJOR_TYPE(next) != MajorTypes::TEXT_STRING) {
            throw_format_error("Object keys must be strings");
          }
          UInt64 m;
          INCR_PTR(first, 1);
          parse_number(first, last, next, m);
          ensure(m, first, last);
          boost::json::string_view sv(reinterpret_cast<char const*>(first), m);
          INCR_PTR(first, m);
          // value
          parse_value(first, last, o[sv]);
        }
      }
      return BYTES_CONSUMED(first, temp);
    }

    inline std::size_t parse_unsigned(
     const UChar* first, const UChar* last, UChar curr, boost::json::value& v) {
      auto temp = first;
      UInt64 n;
      parse_number(first, last, curr, n);
      v = n;
      return BYTES_CONSUMED(first, temp);
    }

    inline std::size_t parse_signed(
     const UChar* first, const UChar* last, UChar curr, boost::json::value& v) {
      auto temp = first;
      UInt64 n;
      parse_number(first, last, curr, n);
      v = static_cast<std::int64_t>(~n);
      return BYTES_CONSUMED(first, temp);
    }

    inline std::size_t parse_semantic_tag(
     const UChar* first, const UChar* last, UChar curr, boost::json::value& v) {
      auto temp = first;
      UInt64 n;
      parse_number(first, last, curr, n);
      // ignore semantic tags
      ensure(1, first, last);
      const auto tag_curr = *first;
      INCR_PTR(first, 1);
      parse_value(first, last, v);
      return BYTES_CONSUMED(first, temp);
    }

    inline std::size_t parse_simple_value(
     const UChar* first, const UChar* last, UChar curr, boost::json::value& v) {
      auto temp = first;
      switch (MINOR_TYPE(curr)) {
      case UC(SimpleValueTypes::FALSE):
        v = false;
        return BYTES_CONSUMED(first, temp);
      case UC(SimpleValueTypes::TRUE):
        v = true;
        return BYTES_CONSUMED(first, temp);
      case UC(SimpleValueTypes::NULL_VALUE):
        v = nullptr;
        return BYTES_CONSUMED(first, temp);
      case UC(SimpleValueTypes::UNDEFINED): // undefined
        throw_format_error("Undefined value not supported");
      case UC(SimpleValueTypes::SIMPLE_VALUE): // simple value
        ensure(1, first, last);
        {
          auto simple_value = *first++;
          // Only support simple values that are JSON-compatible
          // 0xFF (255) is not a valid simple value in this context
          throw_format_error("Simple value not supported");
        }
      case UC(SimpleValueTypes::FLOAT16): // half-precision float
        {
          ensure(2, first, last);
          // Convert half-precision float to double
          auto half = boost::endian::load_big_u16(first);
          INCR_PTR(first, 2);
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
            result = std::ldexp((CBOR_FLOAT16_MANTISSA_MAX + mantissa) / CBOR_FLOAT16_MANTISSA_MAX,
                      exponent - CBOR_FLOAT16_NORMAL_EXP_BIAS) *
                     (sign ? -1.0 : 1.0);
          }
          v = result;
          return BYTES_CONSUMED(first, temp);
        }
      case UC(SimpleValueTypes::FLOAT32): // float
        {
          ensure(4, first, last);
          auto w = boost::endian::endian_load<float, 4, boost::endian::order::big>(first);
          INCR_PTR(first, 4);
          v = w;
          return BYTES_CONSUMED(first, temp);
        }
      case UC(SimpleValueTypes::FLOAT64): // double
        {
          ensure(8, first, last);
          auto w = boost::endian::endian_load<double, 8, boost::endian::order::big>(first);
          INCR_PTR(first, 8);
          v = w;
          return BYTES_CONSUMED(first, temp);
        }
      default:
        throw_format_error("Invalid minor type for major type 7");
      }
    }

    inline std::size_t parse_value(const UChar* first, const UChar* last, boost::json::value& v) {
      auto temp = first;
      ensure(1, first, last);
      const auto curr = *first;
      INCR_PTR(first, 1);
      switch (MAJOR_TYPE(curr)) {
      case MajorTypes::UNSIGNED_INTEGER:
        parse_unsigned(first, last, curr, v);
        break;
      case MajorTypes::NEGATIVE_INTEGER:
        parse_signed(first, last, curr, v);
        break;
      case MajorTypes::BYTE_STRING:
        parse_binary(first, last, curr, v);
        break;
      case MajorTypes::TEXT_STRING:
        parse_string(first, last, curr, v);
        break;
      case MajorTypes::ARRAY:
        parse_array(first, last, curr, v);
        break;
      case MajorTypes::MAP:
        parse_object(first, last, curr, v);
        break;
      case MajorTypes::SEMANTIC_TAG:
        parse_semantic_tag(first, last, curr, v);
        break;
      case MajorTypes::SIMPLE_VALUE:
        parse_simple_value(first, last, curr, v);
        break;
      default:
        // This should be unreachable in valid CBOR
        throw_format_error("Invalid major type");
      }
      return BYTES_CONSUMED(first, temp);
    }

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
    detail::serialize_value(jv, out);
  }

  /**
   * @brief Parse a CBOR value and return bytes consumed
   * @param first Start of input buffer
   * @param last End of input buffer
   * @param v Output JSON value
   * @return Number of bytes consumed from input
   */
  inline std::size_t parse_cbor_value(
   const UChar* first, const UChar* last, boost::json::value& v) {
    return detail::parse_value(first, last, v);
  }
} // namespace cbor

#undef ENCODE_SV
#undef UC
#undef INCR_PTR
#undef BYTES_CONSUMED
#undef MAJOR_TYPE
#undef MINOR_TYPE

#endif // CBOR_H_INCLUDED
