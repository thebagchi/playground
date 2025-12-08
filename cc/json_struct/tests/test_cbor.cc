#include "cbor.h"
#include <iostream>
#include <iomanip>
#include <vector>

void test_overlong_encoding_rejection() {
  std::cout << "\n=== Testing Overlong Encoding Rejection (RFC 8949 Section 3.9) ===" << std::endl;

  // Test 1: Value 10 encoded with 1-byte form (correct)
  {
    std::cout << "\n1. Value 10 with shortest form (0x0A): ";
    std::vector<unsigned char> data = { 0x0A }; // Direct encoding
    boost::json::value v;
    try {
      std::string_view sv(reinterpret_cast<const char*>(data.data()), data.size());
      cbor::parse_cbor_value(sv, v);
      std::cout << "✓ Accepted: " << v << std::endl;
    } catch (const std::exception& e) {
      std::cout << "✗ Rejected: " << e.what() << std::endl;
    }
  }

  // Test 2: Value 10 encoded with 2-byte form (overlong - should reject)
  {
    std::cout << "\n2. Value 10 with overlong 1-byte encoding (0x18 0x0A): ";
    std::vector<unsigned char> data = { 0x18, 0x0A }; // additional info 24 + 1 byte
    boost::json::value v;
    try {
      std::string_view sv(reinterpret_cast<const char*>(data.data()), data.size());
      cbor::parse_cbor_value(sv, v);
      std::cout << "✗ ERROR: Accepted overlong encoding: " << v << std::endl;
    } catch (const std::exception& e) {
      std::cout << "✓ Correctly rejected: " << e.what() << std::endl;
    }
  }

  // Test 3: Value 100 encoded with 2-byte form (overlong - should reject)
  {
    std::cout << "\n3. Value 100 with overlong 2-byte encoding (0x19 0x00 0x64): ";
    std::vector<unsigned char> data = { 0x19, 0x00, 0x64 }; // 2-byte form
    boost::json::value v;
    try {
      std::string_view sv(reinterpret_cast<const char*>(data.data()), data.size());
      cbor::parse_cbor_value(sv, v);
      std::cout << "✗ ERROR: Accepted overlong encoding: " << v << std::endl;
    } catch (const std::exception& e) {
      std::cout << "✓ Correctly rejected: " << e.what() << std::endl;
    }
  }

  // Test 4: Value 1000 encoded with 2-byte form (correct)
  {
    std::cout << "\n4. Value 1000 with correct 2-byte encoding (0x19 0x03 0xE8): ";
    std::vector<unsigned char> data = { 0x19, 0x03, 0xE8 };
    boost::json::value v;
    try {
      std::string_view sv(reinterpret_cast<const char*>(data.data()), data.size());
      cbor::parse_cbor_value(sv, v);
      std::cout << "✓ Accepted: " << v << std::endl;
    } catch (const std::exception& e) {
      std::cout << "✗ Rejected: " << e.what() << std::endl;
    }
  }

  // Test 5: Value 1000 encoded with 4-byte form (overlong - should reject)
  {
    std::cout << "\n5. Value 1000 with overlong 4-byte encoding (0x1A 0x00 0x00 0x03 0xE8): ";
    std::vector<unsigned char> data = { 0x1A, 0x00, 0x00, 0x03, 0xE8 };
    boost::json::value v;
    try {
      std::string_view sv(reinterpret_cast<const char*>(data.data()), data.size());
      cbor::parse_cbor_value(sv, v);
      std::cout << "✗ ERROR: Accepted overlong encoding: " << v << std::endl;
    } catch (const std::exception& e) {
      std::cout << "✓ Correctly rejected: " << e.what() << std::endl;
    }
  }

  // Test 6: Value 100000 encoded with 8-byte form (overlong - should reject)
  {
    std::cout << "\n6. Value 100000 with overlong 8-byte encoding: ";
    std::vector<unsigned char> data = { 0x1B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x86, 0xA0 };
    boost::json::value v;
    try {
      std::string_view sv(reinterpret_cast<const char*>(data.data()), data.size());
      cbor::parse_cbor_value(sv, v);
      std::cout << "✗ ERROR: Accepted overlong encoding: " << v << std::endl;
    } catch (const std::exception& e) {
      std::cout << "✓ Correctly rejected: " << e.what() << std::endl;
    }
  }
}

void test_shortest_encoding_generation() {
  std::cout << "\n=== Testing Shortest Encoding Generation ===" << std::endl;

  // Test various values to ensure we use shortest form
  std::vector<std::pair<uint64_t, size_t>> test_cases = {
    { 0, 1 },           // direct
    { 23, 1 },          // direct
    { 24, 2 },          // 1-byte additional
    { 255, 2 },         // 1-byte additional
    { 256, 3 },         // 2-byte additional
    { 65535, 3 },       // 2-byte additional
    { 65536, 5 },       // 4-byte additional
    { 0xFFFFFFFF, 5 },  // 4-byte additional
    { 0x100000000, 9 }, // 8-byte additional
  };

  for (const auto& [value, expected_size] : test_cases) {
    std::vector<unsigned char> cbor_data;
    boost::json::value json_value = value;
    cbor::serialize_cbor_value(json_value, cbor_data);

    std::cout << "\nValue " << value << " (0x" << std::hex << value << std::dec << "): ";
    std::cout << cbor_data.size() << " bytes ";
    if (cbor_data.size() == expected_size) {
      std::cout << "✓ Correct length" << std::endl;
    } else {
      std::cout << "✗ Expected " << expected_size << " bytes" << std::endl;
    }

    // Show bytes
    std::cout << "  Bytes: ";
    for (auto byte : cbor_data) {
      std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
    }
    std::cout << std::dec << std::endl;

    // Verify round-trip
    boost::json::value parsed;
    std::string_view sv(reinterpret_cast<const char*>(cbor_data.data()), cbor_data.size());
    cbor::parse_cbor_value(sv, parsed);

    if (parsed.as_uint64() == value) {
      std::cout << "  ✓ Round-trip successful" << std::endl;
    } else {
      std::cout << "  ✗ Round-trip failed: got " << parsed.as_uint64() << std::endl;
    }
  }
}

void test_byte_strings() {
  std::cout << "\n=== Testing Byte String Encoding ===" << std::endl;

  // Test 1: Definite length byte string
  {
    std::cout << "\n1. Definite length byte string (5 bytes): ";
    // CBOR: 0x45 (major type 2, length 5) + "Hello" bytes
    std::vector<unsigned char> data = { 0x45, 0x48, 0x65, 0x6C, 0x6C, 0x6F };
    boost::json::value v;
    try {
      std::string_view sv(reinterpret_cast<const char*>(data.data()), data.size());
      cbor::parse_cbor_value(sv, v);
      std::cout << "✓ Parsed: " << v << std::endl;

      // Verify it's an object with _binary key
      if (v.is_object() && v.as_object().contains(cbor::detail::CBOR_BINARY_MARKER) &&
          v.as_object().at(cbor::detail::CBOR_BINARY_MARKER).is_string()) {
        std::cout << "  ✓ Correctly wrapped in {\"_binary\": \"base64...\"}" << std::endl;

        // Test round-trip
        std::vector<unsigned char> encoded;
        cbor::serialize_cbor_value(v, encoded);
        if (encoded == data) {
          std::cout << "  ✓ Round-trip successful" << std::endl;
        } else {
          std::cout << "  ✗ Round-trip failed" << std::endl;
        }
      } else {
        std::cout << "  ✗ ERROR: Not wrapped correctly" << std::endl;
      }
    } catch (const std::exception& e) {
      std::cout << "✗ Failed: " << e.what() << std::endl;
    }
  }

  // Test 2: Indefinite length byte string with single chunk
  {
    std::cout << "\n2. Indefinite length byte string (single chunk): ";
    // CBOR: 0x5F (major type 2, indefinite) + 0x43 (chunk length 3) + "ABC" + 0xFF (break)
    std::vector<unsigned char> data = { 0x5F, 0x42, 0x41, 0x42, 0x43, 0x43, 0x44, 0x45, 0xFF };
    boost::json::value v;
    try {
      std::string_view sv(reinterpret_cast<const char*>(data.data()), data.size());
      cbor::parse_cbor_value(sv, v);
      std::cout << "✓ Parsed: " << v << std::endl;

      if (v.is_object() && v.as_object().contains(cbor::detail::CBOR_BINARY_MARKER)) {
        std::cout << "  ✓ Correctly wrapped in {\"_binary\": \"base64...\"}" << std::endl;
      } else {
        std::cout << "  ✗ ERROR: Not wrapped correctly" << std::endl;
      }
    } catch (const std::exception& e) {
      std::cout << "✗ Failed: " << e.what() << std::endl;
    }
  }

  // Test 3: Indefinite length byte string with multiple chunks
  {
    std::cout << "\n3. Indefinite length byte string (multiple chunks): ";
    // CBOR: 0x5F (indefinite) + 0x42 (chunk len 2) + "AB" + 0x43 (chunk len 3) + "CDE" + 0xFF (break)
    std::vector<unsigned char> data = { 0x5F, 0x42, 0x41, 0x42, 0x43, 0x43, 0x44, 0x45, 0xFF };
    boost::json::value v;
    try {
      std::string_view sv(reinterpret_cast<const char*>(data.data()), data.size());
      cbor::parse_cbor_value(sv, v);
      std::cout << "✓ Parsed: " << v << std::endl;

      if (v.is_object() && v.as_object().contains(cbor::detail::CBOR_BINARY_MARKER)) {
        std::cout << "  ✓ Correctly wrapped in {\"_binary\": \"base64...\"}" << std::endl;
      } else {
        std::cout << "  ✗ ERROR: Not wrapped correctly" << std::endl;
      }
    } catch (const std::exception& e) {
      std::cout << "✗ Failed: " << e.what() << std::endl;
    }
  }

  // Test 4: Empty byte string (definite length)
  {
    std::cout << "\n4. Empty byte string (definite length): ";
    // CBOR: 0x40 (major type 2, length 0)
    std::vector<unsigned char> data = { 0x40 };
    boost::json::value v;
    try {
      std::string_view sv(reinterpret_cast<const char*>(data.data()), data.size());
      cbor::parse_cbor_value(sv, v);
      std::cout << "✓ Parsed: " << v << std::endl;

      if (v.is_object() && v.as_object().contains(cbor::detail::CBOR_BINARY_MARKER) &&
          v.as_object().at(cbor::detail::CBOR_BINARY_MARKER).as_string().empty()) {
        std::cout << "  ✓ Correctly wrapped with empty base64" << std::endl;
      } else {
        std::cout << "  ✗ ERROR: Not wrapped correctly" << std::endl;
      }
    } catch (const std::exception& e) {
      std::cout << "✗ Failed: " << e.what() << std::endl;
    }
  }

  // Test 5: Empty byte string (indefinite length)
  {
    std::cout << "\n5. Empty byte string (indefinite length): ";
    // CBOR: 0x5F (indefinite) + 0xFF (break immediately)
    std::vector<unsigned char> data = { 0x5F, 0xFF };
    boost::json::value v;
    try {
      std::string_view sv(reinterpret_cast<const char*>(data.data()), data.size());
      cbor::parse_cbor_value(sv, v);
      std::cout << "✓ Parsed: " << v << std::endl;

      if (v.is_object() && v.as_object().contains(cbor::detail::CBOR_BINARY_MARKER) &&
          v.as_object().at(cbor::detail::CBOR_BINARY_MARKER).as_string().empty()) {
        std::cout << "  ✓ Correctly wrapped with empty base64" << std::endl;
      } else {
        std::cout << "  ✗ ERROR: Not wrapped correctly" << std::endl;
      }
    } catch (const std::exception& e) {
      std::cout << "✗ Failed: " << e.what() << std::endl;
    }
  }

  // Test 6: Byte string with binary data (non-ASCII)
  {
    std::cout << "\n6. Byte string with binary data: ";
    // CBOR: 0x44 (length 4) + binary bytes 0x00, 0xFF, 0x80, 0x7F
    std::vector<unsigned char> data = { 0x44, 0x00, 0xFF, 0x80, 0x7F };
    boost::json::value v;
    try {
      std::string_view sv(reinterpret_cast<const char*>(data.data()), data.size());
      cbor::parse_cbor_value(sv, v);
      std::cout << "✓ Parsed: " << v << std::endl;

      if (v.is_object() && v.as_object().contains(cbor::detail::CBOR_BINARY_MARKER)) {
        std::cout << "  ✓ Correctly wrapped in {\"_binary\": \"base64...\"}" << std::endl;
      } else {
        std::cout << "  ✗ ERROR: Not wrapped correctly" << std::endl;
      }
    } catch (const std::exception& e) {
      std::cout << "✗ Failed: " << e.what() << std::endl;
    }
  }
}

void test_semantic_tags() {
  std::cout << "\n=== Testing Semantic Tag Preservation ===" << std::endl;

  // Test 1: Date/time string (tag 0)
  {
    std::cout << "\n1. Tag 0 (date/time) with string value: ";
    // CBOR: 0xC0 (tag 0) + 0x74 (text string length 20) + "2013-03-21T20:04:00Z"
    std::vector<unsigned char> data = { 0xC0,
      0x74,
      '2',
      '0',
      '1',
      '3',
      '-',
      '0',
      '3',
      '-',
      '2',
      '1',
      'T',
      '2',
      '0',
      ':',
      '0',
      '4',
      ':',
      '0',
      '0',
      'Z' };
    boost::json::value v;
    try {
      std::string_view sv(reinterpret_cast<const char*>(data.data()), data.size());
      cbor::parse_cbor_value(sv, v);
      std::cout << "✓ Parsed: " << v << std::endl;

      if (v.is_object() && v.as_object().contains(cbor::detail::CBOR_TAG_MARKER) &&
          v.as_object().contains(cbor::detail::CBOR_VALUE_MARKER)) {
        auto tag = v.as_object().at(cbor::detail::CBOR_TAG_MARKER).as_uint64();
        std::cout << "  ✓ Tag preserved: " << tag << std::endl;

        // Test round-trip
        ByteBuffer out;
        cbor::serialize_cbor_value(v, out);
        boost::json::value v2;
        std::string_view sv2(reinterpret_cast<const char*>(out.data()), out.size());
        cbor::parse_cbor_value(sv2, v2);

        if (v == v2) {
          std::cout << "  ✓ Round-trip successful" << std::endl;
        } else {
          std::cout << "  ✗ Round-trip failed" << std::endl;
        }
      } else {
        std::cout << "  ✗ ERROR: Not wrapped correctly" << std::endl;
      }
    } catch (const std::exception& e) {
      std::cout << "✗ Failed: " << e.what() << std::endl;
    }
  }

  // Test 2: Positive bignum (tag 2)
  {
    std::cout << "\n2. Tag 2 (positive bignum) with byte string: ";
    // CBOR: 0xC2 (tag 2) + 0x49 (byte string length 9) + bytes
    std::vector<unsigned char> data = {
      0xC2, 0x49, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    boost::json::value v;
    try {
      std::string_view sv(reinterpret_cast<const char*>(data.data()), data.size());
      cbor::parse_cbor_value(sv, v);
      std::cout << "✓ Parsed: " << v << std::endl;

      if (v.is_object() && v.as_object().contains(cbor::detail::CBOR_TAG_MARKER) &&
          v.as_object().contains(cbor::detail::CBOR_VALUE_MARKER)) {
        auto tag = v.as_object().at(cbor::detail::CBOR_TAG_MARKER).as_uint64();
        std::cout << "  ✓ Tag preserved: " << tag << std::endl;
      } else {
        std::cout << "  ✗ ERROR: Not wrapped correctly" << std::endl;
      }
    } catch (const std::exception& e) {
      std::cout << "✗ Failed: " << e.what() << std::endl;
    }
  }

  // Test 3: Nested tags (tag within tag)
  {
    std::cout << "\n3. Nested tags (tag 1 within tag 0): ";
    // CBOR: 0xC0 (tag 0) + 0xC1 (tag 1) + 0x1A (4-byte uint) + 1234567890
    std::vector<unsigned char> data = { 0xC0, 0xC1, 0x1A, 0x49, 0x96, 0x02, 0xD2 };
    boost::json::value v;
    try {
      std::string_view sv(reinterpret_cast<const char*>(data.data()), data.size());
      cbor::parse_cbor_value(sv, v);
      std::cout << "✓ Parsed: " << v << std::endl;

      if (v.is_object() && v.as_object().contains(cbor::detail::CBOR_TAG_MARKER)) {
        auto tag = v.as_object().at(cbor::detail::CBOR_TAG_MARKER).as_uint64();
        std::cout << "  ✓ Outer tag preserved: " << tag << std::endl;

        auto& inner = v.as_object().at(cbor::detail::CBOR_VALUE_MARKER);
        if (inner.is_object() && inner.as_object().contains(cbor::detail::CBOR_TAG_MARKER)) {
          auto inner_tag = inner.as_object().at(cbor::detail::CBOR_TAG_MARKER).as_uint64();
          std::cout << "  ✓ Inner tag preserved: " << inner_tag << std::endl;
        }

        // Test round-trip
        ByteBuffer out;
        cbor::serialize_cbor_value(v, out);
        boost::json::value v2;
        std::string_view sv2(reinterpret_cast<const char*>(out.data()), out.size());
        cbor::parse_cbor_value(sv2, v2);

        if (v == v2) {
          std::cout << "  ✓ Round-trip successful" << std::endl;
        } else {
          std::cout << "  ✗ Round-trip failed" << std::endl;
        }
      }
    } catch (const std::exception& e) {
      std::cout << "✗ Failed: " << e.what() << std::endl;
    }
  }

  // Test 4: Tag with array value
  {
    std::cout << "\n4. Tag 4 (decimal fraction) with array: ";
    // CBOR: 0xC4 (tag 4) + 0x82 (array length 2) + 0x21 (-2) + 0x19 0x03 0xE8 (1000)
    std::vector<unsigned char> data = { 0xC4, 0x82, 0x21, 0x19, 0x03, 0xE8 };
    boost::json::value v;
    try {
      std::string_view sv(reinterpret_cast<const char*>(data.data()), data.size());
      cbor::parse_cbor_value(sv, v);
      std::cout << "✓ Parsed: " << v << std::endl;

      if (v.is_object() && v.as_object().contains(cbor::detail::CBOR_TAG_MARKER) &&
          v.as_object().contains(cbor::detail::CBOR_VALUE_MARKER)) {
        auto tag = v.as_object().at(cbor::detail::CBOR_TAG_MARKER).as_uint64();
        std::cout << "  ✓ Tag preserved: " << tag << std::endl;

        auto& value = v.as_object().at(cbor::detail::CBOR_VALUE_MARKER);
        if (value.is_array()) {
          std::cout << "  ✓ Array value preserved" << std::endl;
        }
      } else {
        std::cout << "  ✗ ERROR: Not wrapped correctly" << std::endl;
      }
    } catch (const std::exception& e) {
      std::cout << "✗ Failed: " << e.what() << std::endl;
    }
  }
}

void test_utility_functions() {
  std::cout << "\n=== Testing Utility Functions ===" << std::endl;

  // Test IsByteString
  {
    std::cout << "\n1. IsByteString utility: ";
    std::vector<unsigned char> data = { 0x45, 0x48, 0x65, 0x6C, 0x6C, 0x6F }; // "Hello"
    boost::json::value v;
    std::string_view sv(reinterpret_cast<const char*>(data.data()), data.size());
    cbor::parse_cbor_value(sv, v);

    if (cbor::detail::IsByteString(v)) {
      std::cout << "✓ Correctly identified byte string: " << v << std::endl;
    } else {
      std::cout << "✗ ERROR: Failed to identify byte string" << std::endl;
    }

    // Test with non-byte string
    boost::json::value plain_string = "test";
    if (!cbor::detail::IsByteString(plain_string)) {
      std::cout << "  ✓ Correctly rejected plain string" << std::endl;
    } else {
      std::cout << "  ✗ ERROR: False positive on plain string" << std::endl;
    }
  }

  // Test IsSemanticTag
  {
    std::cout << "\n2. IsSemanticTag utility: ";
    std::vector<unsigned char> data = { 0xC0,
      0x74,
      0x32,
      0x30,
      0x31,
      0x33,
      0x2D,
      0x30,
      0x33,
      0x2D,
      0x32,
      0x31,
      0x54,
      0x32,
      0x30,
      0x3A,
      0x30,
      0x34,
      0x3A,
      0x30,
      0x30,
      0x5A };
    boost::json::value v;
    std::string_view sv(reinterpret_cast<const char*>(data.data()), data.size());
    cbor::parse_cbor_value(sv, v);

    if (cbor::detail::IsSemanticTag(v)) {
      std::cout << "✓ Correctly identified semantic tag: " << v << std::endl;
    } else {
      std::cout << "✗ ERROR: Failed to identify semantic tag" << std::endl;
    }

    // Test with non-semantic tag
    boost::json::value plain_object = boost::json::object{ { "key", "value" } };
    if (!cbor::detail::IsSemanticTag(plain_object)) {
      std::cout << "  ✓ Correctly rejected plain object" << std::endl;
    } else {
      std::cout << "  ✗ ERROR: False positive on plain object" << std::endl;
    }
  }
}

void test_byte_string_extraction() {
  std::cout << "\n=== Testing Byte String Extraction ===" << std::endl;

  // Test data: "Hello World"
  std::vector<unsigned char> cbor_data = {
    0x4B, 0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x20, 0x57, 0x6F, 0x72, 0x6C, 0x64
  };

  boost::json::value v;
  std::string_view sv(reinterpret_cast<const char*>(cbor_data.data()), cbor_data.size());
  cbor::parse_cbor_value(sv, v);

  // Test 1: Extract to ByteVector
  {
    std::cout << "\n1. Extract to ByteVector: ";
    ByteVector byte_vec;
    if (cbor::ExtractByteString(v, byte_vec)) {
      std::cout << "✓ Extracted " << byte_vec.size() << " bytes" << std::endl;
      // Verify content
      std::string result;
      for (auto b : byte_vec) {
        result += static_cast<char>(b);
      }
      if (result == "Hello World") {
        std::cout << "  ✓ Content matches: \"" << result << "\"" << std::endl;
      } else {
        std::cout << "  ✗ ERROR: Content mismatch: \"" << result << "\"" << std::endl;
      }
    } else {
      std::cout << "✗ ERROR: Extraction failed" << std::endl;
    }
  }

  // Test 2: Extract to ByteBuffer
  {
    std::cout << "\n2. Extract to ByteBuffer: ";
    ByteBuffer byte_buf;
    if (cbor::ExtractByteString(v, byte_buf)) {
      std::cout << "✓ Extracted " << byte_buf.size() << " bytes" << std::endl;
      // Verify content
      std::string result(byte_buf.begin(), byte_buf.end());
      if (result == "Hello World") {
        std::cout << "  ✓ Content matches: \"" << result << "\"" << std::endl;
      } else {
        std::cout << "  ✗ ERROR: Content mismatch: \"" << result << "\"" << std::endl;
      }
    } else {
      std::cout << "✗ ERROR: Extraction failed" << std::endl;
    }
  }

  // Test 3: Extract to ByteArray (fixed size)
  {
    std::cout << "\n3. Extract to ByteArray<32>: ";
    ByteArray<32> byte_arr;
    if (cbor::ExtractByteString(v, byte_arr)) {
      std::cout << "✓ Extracted to fixed array" << std::endl;
      // Verify content
      std::string result;
      for (std::size_t i = 0; i < 11; ++i) {
        result += static_cast<char>(byte_arr[i]);
      }
      if (result == "Hello World") {
        std::cout << "  ✓ Content matches: \"" << result << "\"" << std::endl;
      } else {
        std::cout << "  ✗ ERROR: Content mismatch: \"" << result << "\"" << std::endl;
      }
    } else {
      std::cout << "✗ ERROR: Extraction failed" << std::endl;
    }
  }

  // Test 4: Extract to too-small ByteArray (should fail)
  {
    std::cout << "\n4. Extract to ByteArray<5> (too small): ";
    ByteArray<5> small_arr;
    if (!cbor::ExtractByteString(v, small_arr)) {
      std::cout << "✓ Correctly rejected (buffer too small)" << std::endl;
    } else {
      std::cout << "✗ ERROR: Should have failed due to insufficient size" << std::endl;
    }
  }

  // Test 5: Extract from non-byte-string (should fail)
  {
    std::cout << "\n5. Extract from plain string (should fail): ";
    boost::json::value plain_str = "not a byte string";
    ByteBuffer buf;
    if (!cbor::ExtractByteString(plain_str, buf)) {
      std::cout << "✓ Correctly rejected non-byte-string" << std::endl;
    } else {
      std::cout << "✗ ERROR: Should have rejected non-byte-string" << std::endl;
    }
  }
}

void test_semantic_tag_extraction() {
  std::cout << "\n=== Testing Semantic Tag Extraction ===" << std::endl;

  // Test 1: Extract tag 0 (date/time string)
  {
    std::cout << "\n1. Extract tag 0 (date/time): ";
    std::vector<unsigned char> data = { 0xC0,
      0x74,
      0x32,
      0x30,
      0x31,
      0x33,
      0x2D,
      0x30,
      0x33,
      0x2D,
      0x32,
      0x31,
      0x54,
      0x32,
      0x30,
      0x3A,
      0x30,
      0x34,
      0x3A,
      0x30,
      0x30,
      0x5A };
    boost::json::value v;
    std::string_view sv(reinterpret_cast<const char*>(data.data()), data.size());
    cbor::parse_cbor_value(sv, v);

    SemanticTag tag;
    if (cbor::ExtractSemanticTag(v, tag)) {
      auto [tag_num, tag_val] = tag;
      std::cout << "✓ Extracted tag " << tag_num << std::endl;
      if (tag_num == 0 && tag_val.is_string()) {
        std::cout << "  ✓ Tag number correct: " << tag_num << std::endl;
        std::cout << "  ✓ Value: " << tag_val << std::endl;
      } else {
        std::cout << "  ✗ ERROR: Tag data incorrect" << std::endl;
      }
    } else {
      std::cout << "✗ ERROR: Extraction failed" << std::endl;
    }
  }

  // Test 2: Extract using optional return
  {
    std::cout << "\n2. Extract using optional return: ";
    std::vector<unsigned char> data = { 0xC4, 0x82, 0x21, 0x19, 0x03, 0xE8 };
    boost::json::value v;
    std::string_view sv(reinterpret_cast<const char*>(data.data()), data.size());
    cbor::parse_cbor_value(sv, v);

    auto tag_opt = cbor::ExtractSemanticTag(v);
    if (tag_opt.has_value()) {
      auto [tag_num, tag_val] = *tag_opt;
      std::cout << "✓ Extracted tag " << tag_num << std::endl;
      if (tag_num == 4 && tag_val.is_array()) {
        std::cout << "  ✓ Tag number correct: " << tag_num << std::endl;
        std::cout << "  ✓ Array value: " << tag_val << std::endl;
      } else {
        std::cout << "  ✗ ERROR: Tag data incorrect" << std::endl;
      }
    } else {
      std::cout << "✗ ERROR: Optional is empty" << std::endl;
    }
  }

  // Test 3: Extract nested tags
  {
    std::cout << "\n3. Extract nested tags: ";
    std::vector<unsigned char> data = { 0xC0, 0xC1, 0x1A, 0x49, 0x96, 0x02, 0xD2 };
    boost::json::value v;
    std::string_view sv(reinterpret_cast<const char*>(data.data()), data.size());
    cbor::parse_cbor_value(sv, v);

    SemanticTag outer_tag;
    if (cbor::ExtractSemanticTag(v, outer_tag)) {
      auto [outer_num, outer_val] = outer_tag;
      std::cout << "✓ Extracted outer tag " << outer_num << std::endl;

      SemanticTag inner_tag;
      if (cbor::ExtractSemanticTag(outer_val, inner_tag)) {
        auto [inner_num, inner_val] = inner_tag;
        std::cout << "  ✓ Extracted inner tag " << inner_num << std::endl;
        std::cout << "  ✓ Inner value: " << inner_val << std::endl;
      } else {
        std::cout << "  ✗ ERROR: Failed to extract inner tag" << std::endl;
      }
    } else {
      std::cout << "✗ ERROR: Extraction failed" << std::endl;
    }
  }

  // Test 4: Extract from non-semantic-tag (should fail)
  {
    std::cout << "\n4. Extract from plain object (should fail): ";
    boost::json::value plain_obj = boost::json::object{ { "key", "value" } };
    SemanticTag tag;
    if (!cbor::ExtractSemanticTag(plain_obj, tag)) {
      std::cout << "✓ Correctly rejected non-semantic-tag" << std::endl;
    } else {
      std::cout << "✗ ERROR: Should have rejected non-semantic-tag" << std::endl;
    }
  }

  // Test 5: Optional return with non-semantic-tag
  {
    std::cout << "\n5. Optional return with plain value (should be empty): ";
    boost::json::value plain_str = "not a semantic tag";
    auto tag_opt = cbor::ExtractSemanticTag(plain_str);
    if (!tag_opt.has_value()) {
      std::cout << "✓ Correctly returned empty optional" << std::endl;
    } else {
      std::cout << "✗ ERROR: Should have returned empty optional" << std::endl;
    }
  }
}

int main() {
  try {
    test_overlong_encoding_rejection();
    test_shortest_encoding_generation();
    test_byte_strings();
    test_semantic_tags();
    test_utility_functions();
    test_byte_string_extraction();
    test_semantic_tag_extraction();

    std::cout << "\n=== All Compliance Tests Completed ===" << std::endl;
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "Test failed with exception: " << e.what() << std::endl;
    return 1;
  }
}
