#include <boost/json.hpp>
#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <tuple>

#include "json.h"

class Person {
 public:
  std::string name_;
  std::unique_ptr<std::uint64_t> age_;
  std::shared_ptr<std::string> city_;
  std::optional<std::string> email_;

 public:
  constexpr const static auto properties = std::make_tuple(
      prop(&Person::name_, "name"), prop(&Person::age_, "age"),
      prop(&Person::city_, "city"), prop(&Person::email_, "email"));
};

void encode_decode() {
  try {
    // Create a Person with mixed types
    Person p1;
    p1.name_ = "John Doe";                                 // direct string
    p1.age_ = std::make_unique<std::uint64_t>(30);         // unique_ptr
    p1.city_ = std::make_shared<std::string>("New York");  // shared_ptr
    p1.email_ = "john@example.com";                        // optional

    // Marshal to JSON
    boost::json::value json_value = Marshal(p1);
    std::cout << "Encoded JSON: " << json_value << std::endl;

    // Unmarshal back to Person
    Person p2;
    Unmarshal(json_value, p2);

    // Display decoded results
    std::cout << "Decoded Person:" << std::endl;
    std::cout << "  Name: " << p2.name_ << std::endl;
    if (p2.age_) {
      std::cout << "  Age: " << *p2.age_ << std::endl;
    }
    if (p2.city_) {
      std::cout << "  City: " << *p2.city_ << std::endl;
    }
    if (p2.email_) {
      std::cout << "  Email: " << *p2.email_ << std::endl;
    }

  } catch (const std::exception& e) {
    std::cout << "Error in encode_decode: " << e.what() << std::endl;
  }
}

int main(int argc, char* argv[]) {
  std::cout << "hello world" << std::endl;

  try {
    encode_decode();
  } catch (const std::exception& e) {
    std::cerr << "Error in main: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
