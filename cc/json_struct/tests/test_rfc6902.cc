#define BOOST_TEST_MODULE RFC6902 Tests
#include <boost/json.hpp>
#include <boost/test/unit_test.hpp>
#include <string>

#include "../rfc6902.h"

BOOST_AUTO_TEST_SUITE(RFC6902_Type_Checking)

BOOST_AUTO_TEST_CASE(Test_IsNull) {
  boost::json::value null_val = nullptr;
  boost::json::value string_val = "hello";
  boost::json::value number_val = 42;
  boost::json::value bool_val = true;
  boost::json::value array_val = boost::json::array{};
  boost::json::value object_val = boost::json::object{};

  BOOST_TEST(rfc6902::IsNull(null_val) == true);
  BOOST_TEST(rfc6902::IsNull(string_val) == false);
  BOOST_TEST(rfc6902::IsNull(number_val) == false);
  BOOST_TEST(rfc6902::IsNull(bool_val) == false);
  BOOST_TEST(rfc6902::IsNull(array_val) == false);
  BOOST_TEST(rfc6902::IsNull(object_val) == false);
}

BOOST_AUTO_TEST_CASE(Test_IsNumber) {
  boost::json::value null_val = nullptr;
  boost::json::value string_val = "hello";
  boost::json::value number_val = 42;
  boost::json::value bool_val = true;
  boost::json::value array_val = boost::json::array{};
  boost::json::value object_val = boost::json::object{};

  BOOST_TEST(rfc6902::IsNumber(null_val) == false);
  BOOST_TEST(rfc6902::IsNumber(string_val) == false);
  BOOST_TEST(rfc6902::IsNumber(number_val) == true);
  BOOST_TEST(rfc6902::IsNumber(bool_val) == false);
  BOOST_TEST(rfc6902::IsNumber(array_val) == false);
  BOOST_TEST(rfc6902::IsNumber(object_val) == false);
}

BOOST_AUTO_TEST_CASE(Test_IsString) {
  boost::json::value null_val = nullptr;
  boost::json::value string_val = "hello";
  boost::json::value number_val = 42;
  boost::json::value bool_val = true;
  boost::json::value array_val = boost::json::array{};
  boost::json::value object_val = boost::json::object{};

  BOOST_TEST(rfc6902::IsString(null_val) == false);
  BOOST_TEST(rfc6902::IsString(string_val) == true);
  BOOST_TEST(rfc6902::IsString(number_val) == false);
  BOOST_TEST(rfc6902::IsString(bool_val) == false);
  BOOST_TEST(rfc6902::IsString(array_val) == false);
  BOOST_TEST(rfc6902::IsString(object_val) == false);
}

BOOST_AUTO_TEST_CASE(Test_IsBool) {
  boost::json::value null_val = nullptr;
  boost::json::value string_val = "hello";
  boost::json::value number_val = 42;
  boost::json::value bool_val = true;
  boost::json::value array_val = boost::json::array{};
  boost::json::value object_val = boost::json::object{};

  BOOST_TEST(rfc6902::IsBool(null_val) == false);
  BOOST_TEST(rfc6902::IsBool(string_val) == false);
  BOOST_TEST(rfc6902::IsBool(number_val) == false);
  BOOST_TEST(rfc6902::IsBool(bool_val) == true);
  BOOST_TEST(rfc6902::IsBool(array_val) == false);
  BOOST_TEST(rfc6902::IsBool(object_val) == false);
}

BOOST_AUTO_TEST_CASE(Test_IsList) {
  boost::json::value null_val = nullptr;
  boost::json::value string_val = "hello";
  boost::json::value number_val = 42;
  boost::json::value bool_val = true;
  boost::json::value array_val = boost::json::array{};
  boost::json::value object_val = boost::json::object{};

  BOOST_TEST(rfc6902::IsList(null_val) == false);
  BOOST_TEST(rfc6902::IsList(string_val) == false);
  BOOST_TEST(rfc6902::IsList(number_val) == false);
  BOOST_TEST(rfc6902::IsList(bool_val) == false);
  BOOST_TEST(rfc6902::IsList(array_val) == true);
  BOOST_TEST(rfc6902::IsList(object_val) == false);
}

BOOST_AUTO_TEST_CASE(Test_IsDict) {
  boost::json::value null_val = nullptr;
  boost::json::value string_val = "hello";
  boost::json::value number_val = 42;
  boost::json::value bool_val = true;
  boost::json::value array_val = boost::json::array{};
  boost::json::value object_val = boost::json::object{};

  BOOST_TEST(rfc6902::IsDict(null_val) == false);
  BOOST_TEST(rfc6902::IsDict(string_val) == false);
  BOOST_TEST(rfc6902::IsDict(number_val) == false);
  BOOST_TEST(rfc6902::IsDict(bool_val) == false);
  BOOST_TEST(rfc6902::IsDict(array_val) == false);
  BOOST_TEST(rfc6902::IsDict(object_val) == true);
}

BOOST_AUTO_TEST_CASE(Test_Kind) {
  boost::json::value null_val = nullptr;
  boost::json::value string_val = "hello";
  boost::json::value number_val = 42;
  boost::json::value bool_val = true;
  boost::json::value array_val = boost::json::array{};
  boost::json::value object_val = boost::json::object{};

  BOOST_TEST(std::string(rfc6902::Kind(null_val)) == "null");
  BOOST_TEST(std::string(rfc6902::Kind(string_val)) == "string");
  BOOST_TEST(std::string(rfc6902::Kind(number_val)) == "number");
  BOOST_TEST(std::string(rfc6902::Kind(bool_val)) == "bool");
  BOOST_TEST(std::string(rfc6902::Kind(array_val)) == "array");
  BOOST_TEST(std::string(rfc6902::Kind(object_val)) == "object");
}

BOOST_AUTO_TEST_CASE(Test_Extract) {
  // Create a test JSON object: {"a": {"b": [1, 2, {"c": "value"}]}}
  boost::json::value test_val = boost::json::parse(R"(
        {
            "a": {
                "b": [1, 2, {"c": "value"}]
            }
        }
    )");

  // Test root extraction
  auto root = rfc6902::Extract(test_val, "");
  BOOST_TEST(root.has_value());
  BOOST_TEST(root->is_object());

  // Test object field extraction
  auto a_val = rfc6902::Extract(test_val, "/a");
  BOOST_TEST(a_val.has_value());
  BOOST_TEST(a_val->is_object());

  // Test nested object field extraction
  auto b_val = rfc6902::Extract(test_val, "/a/b");
  BOOST_TEST(b_val.has_value());
  BOOST_TEST(b_val->is_array());
  BOOST_TEST(b_val->as_array().size() == 3);

  // Test array element extraction
  auto arr_elem_0 = rfc6902::Extract(test_val, "/a/b/0");
  BOOST_TEST(arr_elem_0.has_value());
  BOOST_TEST(arr_elem_0->is_number());
  BOOST_TEST(arr_elem_0->as_int64() == 1);

  // Test nested object in array
  auto nested_obj = rfc6902::Extract(test_val, "/a/b/2");
  BOOST_TEST(nested_obj.has_value());
  BOOST_TEST(nested_obj->is_object());

  // Test deeply nested field
  auto deep_val = rfc6902::Extract(test_val, "/a/b/2/c");
  BOOST_TEST(deep_val.has_value());
  BOOST_TEST(deep_val->is_string());
  BOOST_TEST(deep_val->as_string() == "value");

  // Test non-existent paths
  auto nonexistent = rfc6902::Extract(test_val, "/nonexistent");
  BOOST_TEST(!nonexistent.has_value());

  auto out_of_bounds = rfc6902::Extract(test_val, "/a/b/10");
  BOOST_TEST(!out_of_bounds.has_value());

  auto invalid_path = rfc6902::Extract(test_val, "/a/b/invalid");
  BOOST_TEST(!invalid_path.has_value());
}

BOOST_AUTO_TEST_SUITE_END()