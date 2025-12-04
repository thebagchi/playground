#define BOOST_TEST_MODULE JSON Error Handling Tests
#include <boost/json.hpp>
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <memory>

#include "json.h"

// using namespace json;

class Person {
public:
  String name_;
  UniquePtr<UInt64> age_;
  SharedPtr<String> city_;
  Optional<String> email_;
public:
  constexpr const static auto properties = std::make_tuple(json::prop(&Person::name_, "name"),
   json::prop(&Person::age_, "age"),
   json::prop(&Person::city_, "city"),
   json::prop(&Person::email_, "email"));
};

template <> struct json::STRUCT<Person> {
  static constexpr auto properties = Person::properties;
};

BOOST_AUTO_TEST_CASE(TEST_ERROR_TYPE_MISMATCH) {
  std::cout << "==> TEST_ERROR_TYPE_MISMATCH" << std::endl;

  // Test: Type mismatch - string instead of number for age field
  String invalid_json = R"({"name":"John","age":"thirty","city":"NY"})";
  Person p;

  BOOST_CHECK_THROW(json::UnmarshalFromString(invalid_json, p), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(TEST_ERROR_INVALID_JSON_STRUCTURE) {
  std::cout << "==> TEST_ERROR_INVALID_JSON_STRUCTURE" << std::endl;

  // Test: Invalid JSON structure - string instead of object
  String invalid_json = R"("just a string")";
  Person p;

  BOOST_CHECK_THROW(json::UnmarshalFromString(invalid_json, p), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(TEST_ERROR_MALFORMED_JSON) {
  std::cout << "==> TEST_ERROR_MALFORMED_JSON" << std::endl;

  // Test: Malformed JSON - incomplete JSON
  std::string invalid_json = R"({"name":"John","age":)";
  Person p;

  BOOST_CHECK_THROW(json::UnmarshalFromString(invalid_json, p), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(TEST_ERROR_ARRAY_INSTEAD_OF_OBJECT) {
  std::cout << "==> TEST_ERROR_ARRAY_INSTEAD_OF_OBJECT" << std::endl;

  // Test: Array instead of object
  std::string invalid_json = R"(["name","age","city"])";
  Person p;

  BOOST_CHECK_THROW(json::UnmarshalFromString(invalid_json, p), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(TEST_SUCCESS_VALID_JSON) {
  std::cout << "==> TEST_SUCCESS_VALID_JSON" << std::endl;

  // Test: Valid JSON should work correctly
  std::string valid_json = R"({"name":"John","age":30,"city":"NY","email":"john@test.com"})";
  Person p;

  BOOST_CHECK_NO_THROW(json::UnmarshalFromString(valid_json, p));

  // Verify the parsed data
  BOOST_CHECK_EQUAL(p.name_, "John");
  BOOST_CHECK(p.age_ != nullptr);
  BOOST_CHECK_EQUAL(*p.age_, 30u);
  BOOST_CHECK(p.city_ != nullptr);
  BOOST_CHECK_EQUAL(*p.city_, "NY");
  BOOST_CHECK(p.email_.has_value());
  BOOST_CHECK_EQUAL(*p.email_, "john@test.com");
}