#define BOOST_TEST_MODULE Describe Tests
#include <boost/describe.hpp>
#include <boost/json.hpp>
#include <boost/test/unit_test.hpp>

#include "json.h"

// using namespace json;

struct MyStruct {
  int field_name;
};

BOOST_DESCRIBE_STRUCT(MyStruct, (), (field_name))

// Custom serialization to JSON with hyphenated key
void tag_invoke(const boost::json::value_from_tag&, boost::json::value& jv, const MyStruct& s) {
  jv = {
    { "field-name", s.field_name } // Custom key name
  };
}

// Custom deserialization from JSON
MyStruct tag_invoke(const boost::json::value_to_tag<MyStruct>&, const boost::json::value& jv) {
  const auto& obj = jv.as_object();
  return MyStruct{
    boost::json::value_to<int>(obj.at("field-name")) // Map back to member
  };
}

BOOST_AUTO_TEST_SUITE(test_describe_suite)

BOOST_AUTO_TEST_CASE(test_encode_decode) {
  // Create a MyStruct instance
  MyStruct original{ 42 };

  // Encode to JSON
  boost::json::value jv = boost::json::value_from(original);
  String json_str = boost::json::serialize(jv);

  // Expected JSON string
  String expected_json = R"({"field-name":42})";
  BOOST_CHECK_EQUAL(json_str, expected_json);

  // Decode back to MyStruct
  MyStruct decoded = boost::json::value_to<MyStruct>(jv);

  // Verify the decoded value matches the original
  BOOST_CHECK_EQUAL(decoded.field_name, original.field_name);
}

BOOST_AUTO_TEST_SUITE_END()