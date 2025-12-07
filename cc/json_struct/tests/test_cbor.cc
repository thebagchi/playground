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
      const unsigned char* first = data.data();
      const unsigned char* last = first + data.size();
      cbor::parse_cbor_value(first, last, v);
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
      const unsigned char* first = data.data();
      const unsigned char* last = first + data.size();
      cbor::parse_cbor_value(first, last, v);
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
      const unsigned char* first = data.data();
      const unsigned char* last = first + data.size();
      cbor::parse_cbor_value(first, last, v);
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
      const unsigned char* first = data.data();
      const unsigned char* last = first + data.size();
      cbor::parse_cbor_value(first, last, v);
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
      const unsigned char* first = data.data();
      const unsigned char* last = first + data.size();
      cbor::parse_cbor_value(first, last, v);
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
      const unsigned char* first = data.data();
      const unsigned char* last = first + data.size();
      cbor::parse_cbor_value(first, last, v);
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
    const unsigned char* first = cbor_data.data();
    const unsigned char* last = first + cbor_data.size();
    cbor::parse_cbor_value(first, last, parsed);

    if (parsed.as_uint64() == value) {
      std::cout << "  ✓ Round-trip successful" << std::endl;
    } else {
      std::cout << "  ✗ Round-trip failed: got " << parsed.as_uint64() << std::endl;
    }
  }
}

int main() {
  try {
    test_overlong_encoding_rejection();
    test_shortest_encoding_generation();

    std::cout << "\n=== All Compliance Tests Completed ===" << std::endl;
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "Test failed with exception: " << e.what() << std::endl;
    return 1;
  }
}
