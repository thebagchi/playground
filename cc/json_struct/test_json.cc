#define BOOST_TEST_MODULE JSON Struct Tests
#include <boost/json.hpp>
#include <boost/test/unit_test.hpp>
#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <tuple>

#include "json.h"

class Person {
 public:
  std::unique_ptr<std::string> name_;
  std::unique_ptr<std::uint64_t> age_;
  std::unique_ptr<std::string> city_;
  std::unique_ptr<std::string> email_;

 public:
  constexpr const static auto properties = std::make_tuple(
      prop(&Person::name_, "name"), prop(&Person::age_, "age"),
      prop(&Person::city_, "city"), prop<true>(&Person::email_, "email"));
};

class PersonList {
 public:
  std::unique_ptr<std::vector<Person>> persons_;

 public:
  constexpr const static auto properties =
      std::make_tuple(prop(&PersonList::persons_, "persons"));
};

class PersonShared {
 public:
  std::shared_ptr<std::string> name_;
  std::shared_ptr<std::uint64_t> age_;
  std::shared_ptr<std::string> city_;
  std::shared_ptr<std::string> email_;

 public:
  constexpr const static auto properties = std::make_tuple(
      prop(&PersonShared::name_, "name"), prop(&PersonShared::age_, "age"),
      prop(&PersonShared::city_, "city"),
      prop<true>(&PersonShared::email_, "email"));
};

class PersonOptional {
 public:
  std::optional<std::string> name_;
  std::optional<std::uint64_t> age_;
  std::optional<std::string> city_;
  std::optional<std::string> email_;

 public:
  constexpr const static auto properties = std::make_tuple(
      prop(&PersonOptional::name_, "name"), prop(&PersonOptional::age_, "age"),
      prop(&PersonOptional::city_, "city"),
      prop<true>(&PersonOptional::email_, "email"));
};

class PersonScalars {
 public:
  std::string name_;
  std::uint64_t age_;
  std::string city_;
  std::string email_;

 public:
  constexpr const static auto properties = std::make_tuple(
      prop(&PersonScalars::name_, "name"), prop(&PersonScalars::age_, "age"),
      prop(&PersonScalars::city_, "city"),
      prop(&PersonScalars::email_, "email"));
};

BOOST_AUTO_TEST_CASE(test_parse_json_person) {
  boost::json::value json_value;

  try {
    // Block 1
    {
      Person p;
      p.name_ = std::make_unique<std::string>("John Doe");
      p.age_ = std::make_unique<std::uint64_t>(30);
      p.city_ = std::make_unique<std::string>("New York");
      p.email_ = nullptr;  // Nullable field set to null

      // Serialize to JSON
      json_value = Marshal(p);
      std::cout << "Serialized JSON: " << json_value << std::endl;
    }

    // Block 2
    {
      Person p;
      // Deserialize back to object
      Unmarshal(json_value, p);

      // Verify deserialization worked
      BOOST_CHECK(p.name_ != nullptr);
      BOOST_CHECK(p.age_ != nullptr);
      BOOST_CHECK(p.city_ != nullptr);
      BOOST_CHECK(p.email_ == nullptr);

      if (p.name_ && p.age_ && p.city_) {
        std::cout << "Deserialized Person:" << std::endl;
        std::cout << "Name: " << *p.name_ << std::endl;
        std::cout << "Age: " << *p.age_ << std::endl;
        std::cout << "City: " << *p.city_ << std::endl;
        if (p.email_) {
          std::cout << "Email: " << *p.email_ << std::endl;
        } else {
          std::cout << "Email: null" << std::endl;
        }
      } else {
        std::cout << "Deserialization failed - some fields are null"
                  << std::endl;
      }
    }
  } catch (const std::exception& e) {
    std::cout << "Error: " << e.what() << std::endl;
    BOOST_FAIL("Exception thrown: " + std::string(e.what()));
  }
}

BOOST_AUTO_TEST_CASE(test_parse_json_person_shared) {
  boost::json::value json_value;

  try {
    PersonShared p1;
    PersonShared p2;
    // Block 1
    {
      p1.name_ = std::make_shared<std::string>("Charlie Brown");
      p1.age_ = std::make_shared<std::uint64_t>(35);
      p1.city_ = std::make_shared<std::string>("San Francisco");
      p1.email_ = nullptr;  // nullable field

      // Serialize to JSON
      json_value = Marshal(p1);
      std::cout << "Shared Person serialization: " << json_value << std::endl;
    }

    // Block 2
    {
      // Deserialize back to PersonShared
      Unmarshal(json_value, p2);

      // Verify deserialization worked
      BOOST_CHECK(p2.name_ != nullptr);
      BOOST_CHECK(*p2.name_ == "Charlie Brown");
      BOOST_CHECK(p2.age_ != nullptr);
      BOOST_CHECK(*p2.age_ == 35);
      BOOST_CHECK(p2.city_ != nullptr);
      BOOST_CHECK(*p2.city_ == "San Francisco");
      BOOST_CHECK(p2.email_ == nullptr);

      // Display results
      std::cout << "Shared Person deserialization:" << std::endl;
      if (p2.name_) std::cout << "  Name: " << *p2.name_ << std::endl;
      if (p2.age_) std::cout << "  Age: " << *p2.age_ << std::endl;
      if (p2.city_) std::cout << "  City: " << *p2.city_ << std::endl;
      if (p2.email_) {
        std::cout << "  Email: " << *p2.email_ << std::endl;
      } else {
        std::cout << "  Email: null" << std::endl;
      }

      // Test shared ownership
      std::cout << "Shared ownership test:" << std::endl;
      std::cout << "  p1.name_ use_count: " << p1.name_.use_count()
                << std::endl;
      std::cout << "  p2.name_ use_count: " << p2.name_.use_count()
                << std::endl;
    }
  } catch (const std::exception& e) {
    std::cout << "Error in shared person test: " << e.what() << std::endl;
    BOOST_FAIL("Exception thrown: " + std::string(e.what()));
  }
}

BOOST_AUTO_TEST_CASE(test_parse_json_person_optional) {
  try {
    // Create a PersonOptional with std::optional
    PersonOptional p1;
    p1.name_ = "Diana Prince";
    p1.age_ = 28;
    p1.city_ = "Washington DC";
    p1.email_ = std::nullopt;  // nullable field

    // Serialize to JSON
    boost::json::value json_value = Marshal(p1);
    std::cout << "Optional Person serialization: " << json_value << std::endl;

    // Deserialize back to PersonOptional
    PersonOptional p2;
    Unmarshal(json_value, p2);

    // Verify deserialization worked
    BOOST_CHECK(p2.name_ == "Diana Prince");
    BOOST_CHECK(p2.age_ == 28);
    BOOST_CHECK(p2.city_ == "Washington DC");
    BOOST_CHECK(!p2.email_.has_value());

    // Display results
    std::cout << "Optional Person deserialization:" << std::endl;
    if (p2.name_) std::cout << "  Name: " << *p2.name_ << std::endl;
    if (p2.age_) std::cout << "  Age: " << *p2.age_ << std::endl;
    if (p2.city_) std::cout << "  City: " << *p2.city_ << std::endl;
    if (p2.email_) {
      std::cout << "  Email: " << *p2.email_ << std::endl;
    } else {
      std::cout << "  Email: null" << std::endl;
    }

  } catch (const std::exception& e) {
    std::cout << "Error in optional person test: " << e.what() << std::endl;
    BOOST_FAIL("Exception thrown: " + std::string(e.what()));
  }
}

BOOST_AUTO_TEST_CASE(test_parse_json_person_scalars) {
  try {
    // Create a PersonScalars with direct scalar types
    PersonScalars p1;
    p1.name_ = "John Doe";
    p1.age_ = 30;
    p1.city_ = "New York";
    p1.email_ = "john@example.com";

    // Serialize to JSON
    boost::json::value json_value = Marshal(p1);
    std::cout << "Scalars Person serialization: " << json_value << std::endl;

    // Deserialize back to PersonScalars
    PersonScalars p2;
    Unmarshal(json_value, p2);

    // Verify deserialization worked
    BOOST_CHECK(p2.name_ == "John Doe");
    BOOST_CHECK(p2.age_ == 30);
    BOOST_CHECK(p2.city_ == "New York");
    BOOST_CHECK(p2.email_ == "john@example.com");

    // Display results
    std::cout << "Scalars Person deserialization:" << std::endl;
    std::cout << "  Name: " << p2.name_ << std::endl;
    std::cout << "  Age: " << p2.age_ << std::endl;
    std::cout << "  City: " << p2.city_ << std::endl;
    std::cout << "  Email: " << p2.email_ << std::endl;

  } catch (const std::exception& e) {
    std::cout << "Error in scalars person test: " << e.what() << std::endl;
    BOOST_FAIL("Exception thrown: " + std::string(e.what()));
  }
}

BOOST_AUTO_TEST_CASE(test_parse_json_person_list) {
  // Create sample JSON for a list of persons
  std::string json_str = R"(
  {
    "persons": [
      {
        "name": "John Doe",
        "age": 30,
        "city": "New York",
        "email": "john@example.com"
      },
      {
        "name": "Jane Smith",
        "age": 25,
        "city": "Los Angeles"
      },
      {
        "name": "Bob Johnson",
        "age": 35,
        "city": "Chicago",
        "email": null
      }
    ]
  }
  )";

  try {
    // Parse JSON string
    boost::json::value json_value = boost::json::parse(json_str);

    // Deserialize to PersonList
    PersonList person_list;
    Unmarshal(json_value, person_list);

    // Verify deserialization worked
    BOOST_CHECK(person_list.persons_ != nullptr);
    BOOST_CHECK(person_list.persons_->size() == 3);

    // Check first person
    const auto& p1 = (*person_list.persons_)[0];
    BOOST_CHECK(p1.name_ != nullptr && *p1.name_ == "John Doe");
    BOOST_CHECK(p1.age_ != nullptr && *p1.age_ == 30);
    BOOST_CHECK(p1.city_ != nullptr && *p1.city_ == "New York");
    BOOST_CHECK(p1.email_ != nullptr && *p1.email_ == "john@example.com");

    // Check second person
    const auto& p2 = (*person_list.persons_)[1];
    BOOST_CHECK(p2.name_ != nullptr && *p2.name_ == "Jane Smith");
    BOOST_CHECK(p2.age_ != nullptr && *p2.age_ == 25);
    BOOST_CHECK(p2.city_ != nullptr && *p2.city_ == "Los Angeles");
    BOOST_CHECK(p2.email_ == nullptr);

    // Check third person
    const auto& p3 = (*person_list.persons_)[2];
    BOOST_CHECK(p3.name_ != nullptr && *p3.name_ == "Bob Johnson");
    BOOST_CHECK(p3.age_ != nullptr && *p3.age_ == 35);
    BOOST_CHECK(p3.city_ != nullptr && *p3.city_ == "Chicago");
    BOOST_CHECK(p3.email_ == nullptr);

    // Display results
    std::cout << "Parsed Person List:" << std::endl;
    std::cout << "Number of persons: " << person_list.persons_->size()
              << std::endl;

    for (size_t i = 0; i < person_list.persons_->size(); ++i) {
      const auto& person = (*person_list.persons_)[i];
      std::cout << "\nPerson " << (i + 1) << ":" << std::endl;
      if (person.name_) std::cout << "  Name: " << *person.name_ << std::endl;
      if (person.age_) std::cout << "  Age: " << *person.age_ << std::endl;
      if (person.city_) std::cout << "  City: " << *person.city_ << std::endl;
      if (person.email_) {
        std::cout << "  Email: " << *person.email_ << std::endl;
      } else {
        std::cout << "  Email: null" << std::endl;
      }
    }
  } catch (const std::exception& e) {
    std::cout << "Error parsing person list: " << e.what() << std::endl;
    BOOST_FAIL("Exception thrown: " + std::string(e.what()));
  }
}

BOOST_AUTO_TEST_CASE(test_parse_json_vector_person) {
  try {
    // Create a vector of persons directly
    std::vector<Person> people1;

    // Person 1
    Person p1;
    p1.name_ = std::make_unique<std::string>("Alice Cooper");
    p1.age_ = std::make_unique<std::uint64_t>(28);
    p1.city_ = std::make_unique<std::string>("Boston");
    p1.email_ = std::make_unique<std::string>("alice@example.com");
    people1.push_back(std::move(p1));

    // Person 2
    Person p2;
    p2.name_ = std::make_unique<std::string>("David Wilson");
    p2.age_ = std::make_unique<std::uint64_t>(42);
    p2.city_ = std::make_unique<std::string>("Seattle");
    p2.email_ = nullptr;  // nullable field
    people1.push_back(std::move(p2));

    // Serialize vector directly to JSON
    boost::json::value json_value = Marshal(people1);
    std::cout << "Direct vector serialization: " << json_value << std::endl;

    // Deserialize back to vector
    std::vector<Person> people2;
    Unmarshal(json_value, people2);

    // Verify deserialization worked
    BOOST_CHECK(people2.size() == 2);

    // Check first person
    const auto& person1 = people2[0];
    BOOST_CHECK(person1.name_ != nullptr && *person1.name_ == "Alice Cooper");
    BOOST_CHECK(person1.age_ != nullptr && *person1.age_ == 28);
    BOOST_CHECK(person1.city_ != nullptr && *person1.city_ == "Boston");
    BOOST_CHECK(person1.email_ != nullptr &&
                *person1.email_ == "alice@example.com");

    // Check second person
    const auto& person2 = people2[1];
    BOOST_CHECK(person2.name_ != nullptr && *person2.name_ == "David Wilson");
    BOOST_CHECK(person2.age_ != nullptr && *person2.age_ == 42);
    BOOST_CHECK(person2.city_ != nullptr && *person2.city_ == "Seattle");
    BOOST_CHECK(person2.email_ == nullptr);

    // Display results
    std::cout << "Direct vector deserialization:" << std::endl;
    std::cout << "Number of persons: " << people2.size() << std::endl;

    for (size_t i = 0; i < people2.size(); ++i) {
      const auto& person = people2[i];
      std::cout << "\nPerson " << (i + 1) << ":" << std::endl;
      if (person.name_) std::cout << "  Name: " << *person.name_ << std::endl;
      if (person.age_) std::cout << "  Age: " << *person.age_ << std::endl;
      if (person.city_) std::cout << "  City: " << *person.city_ << std::endl;
      if (person.email_) {
        std::cout << "  Email: " << *person.email_ << std::endl;
      } else {
        std::cout << "  Email: null" << std::endl;
      }
    }
  } catch (const std::exception& e) {
    std::cout << "Error in direct vector parsing: " << e.what() << std::endl;
    BOOST_FAIL("Exception thrown: " + std::string(e.what()));
  }
}