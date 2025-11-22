#define BOOST_TEST_MODULE JSON Struct Tests
#include <boost/json.hpp>
#include <boost/test/unit_test.hpp>
#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <tuple>

#include "json.h"
#include "test_data.h"

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

using PersonVector = std::vector<Person>;
using PersonVectorUniquePtr = std::vector<std::unique_ptr<Person>>;
using PersonVectorSharedPtr = std::vector<std::shared_ptr<Person>>;
using PersonVectorOptional = std::vector<std::optional<Person>>;

using PersonMap = std::map<std::string, Person>;
using PersonMapUniquePtr = std::map<std::string, std::unique_ptr<Person>>;
using PersonMapSharedPtr = std::map<std::string, std::shared_ptr<Person>>;
using PersonMapOptional = std::map<std::string, std::optional<Person>>;

using ArbitraryList = std::vector<boost::json::value>;
using ArbitraryListUniquePtr = std::vector<std::unique_ptr<boost::json::value>>;
using ArbitraryListSharedPtr = std::vector<std::shared_ptr<boost::json::value>>;
using ArbitraryListOptional = std::vector<std::optional<boost::json::value>>;

using ArbitraryMap = std::map<std::string, boost::json::value>;
using ArbitraryMapUniquePtr =
    std::map<std::string, std::unique_ptr<boost::json::value>>;
using ArbitraryMapSharedPtr =
    std::map<std::string, std::shared_ptr<boost::json::value>>;
using ArbitraryMapOptional =
    std::map<std::string, std::optional<boost::json::value>>;

class Arbitrary {
 public:
  boost::json::value value_;

 public:
  constexpr const static auto properties =
      std::make_tuple(prop(&Arbitrary::value_, "value"));
};

using ArbitraryUniquePtrStruct = std::unique_ptr<Arbitrary>;
using ArbitrarySharedPtrStruct = std::shared_ptr<Arbitrary>;
using ArbitraryOptionalStruct = std::optional<Arbitrary>;

class ArbitraryUniquePtr {
 public:
  std::unique_ptr<boost::json::value> value_;

 public:
  constexpr const static auto properties =
      std::make_tuple(prop(&ArbitraryUniquePtr::value_, "value"));
};

class ArbitrarySharedPtr {
 public:
  std::shared_ptr<boost::json::value> value_;

 public:
  constexpr const static auto properties =
      std::make_tuple(prop(&ArbitrarySharedPtr::value_, "value"));
};

class ArbitraryOptional {
 public:
  std::optional<boost::json::value> value_;

 public:
  constexpr const static auto properties =
      std::make_tuple(prop(&ArbitraryOptional::value_, "value"));
};

class ArbitraryVector {
 public:
  ArbitraryList value_;

 public:
  constexpr const static auto properties =
      std::make_tuple(prop(&ArbitraryVector::value_, "value"));
};

class ArbitraryDict {
 public:
  ArbitraryMap value_;

 public:
  constexpr const static auto properties =
      std::make_tuple(prop(&ArbitraryDict::value_, "value"));
};

class PersonList {
 public:
  std::unique_ptr<PersonVector> persons_;

 public:
  constexpr const static auto properties =
      std::make_tuple(prop(&PersonList::persons_, "persons"));
};

class PersonListUniquePtr {
 public:
  std::unique_ptr<PersonVectorUniquePtr> persons_;

 public:
  constexpr const static auto properties =
      std::make_tuple(prop(&PersonListUniquePtr::persons_, "persons"));
};

class PersonListSharedPtr {
 public:
  std::shared_ptr<PersonVectorSharedPtr> persons_;

 public:
  constexpr const static auto properties =
      std::make_tuple(prop(&PersonListSharedPtr::persons_, "persons"));
};

class PersonListOptional {
 public:
  std::optional<std::vector<std::optional<Person>>> persons_;

 public:
  constexpr const static auto properties =
      std::make_tuple(prop(&PersonListOptional::persons_, "persons"));
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

class PersonDict {
 public:
  std::unique_ptr<PersonMap> persons_;

 public:
  constexpr const static auto properties =
      std::make_tuple(prop(&PersonDict::persons_, "persons"));
};

class PersonDictUniquePtr {
 public:
  std::unique_ptr<PersonMapUniquePtr> persons_;

 public:
  constexpr const static auto properties =
      std::make_tuple(prop(&PersonDictUniquePtr::persons_, "persons"));
};

class PersonDictSharedPtr {
 public:
  std::shared_ptr<PersonMapSharedPtr> persons_;

 public:
  constexpr const static auto properties =
      std::make_tuple(prop(&PersonDictSharedPtr::persons_, "persons"));
};

class PersonDictOptional {
 public:
  std::optional<PersonMapOptional> persons_;

 public:
  constexpr const static auto properties =
      std::make_tuple(prop(&PersonDictOptional::persons_, "persons"));
};

BOOST_AUTO_TEST_CASE(TEST_PARSE_JSON_PERSON) {
  boost::json::value json_value;

  try {
    {
      Person p;
      p.name_ = std::make_unique<std::string>("John Doe");
      p.age_ = std::make_unique<std::uint64_t>(30);
      p.city_ = std::make_unique<std::string>("New York");
      p.email_ = nullptr;

      json_value = Marshal(p);
      std::cout << "==> TEST_PARSE_JSON_PERSON" << std::endl;
      std::cout << json_value << std::endl;
      std::cout << std::endl;
    }

    {
      Person p;
      Unmarshal(json_value, p);

      BOOST_CHECK(p.name_ != nullptr);
      BOOST_CHECK(p.age_ != nullptr);
      BOOST_CHECK(p.city_ != nullptr);
      BOOST_CHECK(p.email_ == nullptr);
    }
  } catch (const std::exception& e) {
    std::cout << "Error in 'TEST_PARSE_JSON_PERSON': " << e.what() << std::endl;
    BOOST_FAIL("Exception thrown: " + std::string(e.what()));
  }
}

BOOST_AUTO_TEST_CASE(TEST_PARSE_JSON_PERSON_SHARED) {
  boost::json::value json_value;

  try {
    PersonShared p1;
    PersonShared p2;
    {
      p1.name_ = std::make_shared<std::string>("Charlie Brown");
      p1.age_ = std::make_shared<std::uint64_t>(35);
      p1.city_ = std::make_shared<std::string>("San Francisco");
      p1.email_ = nullptr;

      json_value = Marshal(p1);
      std::cout << "==> TEST_PARSE_JSON_PERSON_SHARED" << std::endl;
      std::cout << json_value << std::endl;
      std::cout << std::endl;
    }

    {
      Unmarshal(json_value, p2);

      BOOST_CHECK(p2.name_ != nullptr);
      BOOST_CHECK(*p2.name_ == "Charlie Brown");
      BOOST_CHECK(p2.age_ != nullptr);
      BOOST_CHECK(*p2.age_ == 35);
      BOOST_CHECK(p2.city_ != nullptr);
      BOOST_CHECK(*p2.city_ == "San Francisco");
      BOOST_CHECK(p2.email_ == nullptr);

      BOOST_CHECK(p1.name_.use_count() == 1);
      BOOST_CHECK(p2.name_.use_count() == 1);
    }
  } catch (const std::exception& e) {
    std::cout << "Error in 'TEST_PARSE_JSON_PERSON_SHARED': " << e.what()
              << std::endl;
    BOOST_FAIL("Exception thrown: " + std::string(e.what()));
  }
}

BOOST_AUTO_TEST_CASE(TEST_PARSE_JSON_PERSON_OPTIONAL) {
  try {
    PersonOptional p1;
    p1.name_ = "Diana Prince";
    p1.age_ = 28;
    p1.city_ = "Washington DC";
    p1.email_ = std::nullopt;

    boost::json::value json_value = Marshal(p1);
    std::cout << "==> TEST_PARSE_JSON_PERSON_OPTIONAL" << std::endl;
    std::cout << json_value << std::endl;
    std::cout << std::endl;

    PersonOptional p2;
    Unmarshal(json_value, p2);

    BOOST_CHECK(p2.name_ == "Diana Prince");
    BOOST_CHECK(p2.age_ == 28);
    BOOST_CHECK(p2.city_ == "Washington DC");
    BOOST_CHECK(!p2.email_.has_value());

  } catch (const std::exception& e) {
    std::cout << "Error in 'TEST_PARSE_JSON_PERSON_OPTIONAL': " << e.what()
              << std::endl;
    BOOST_FAIL("Exception thrown: " + std::string(e.what()));
  }
}

BOOST_AUTO_TEST_CASE(TEST_PARSE_JSON_PERSON_SCALARS) {
  try {
    PersonScalars p1;
    p1.name_ = "John Doe";
    p1.age_ = 30;
    p1.city_ = "New York";
    p1.email_ = "john@example.com";

    boost::json::value json_value = Marshal(p1);
    std::cout << "==> TEST_PARSE_JSON_PERSON_SCALARS" << std::endl;
    std::cout << json_value << std::endl;
    std::cout << std::endl;

    PersonScalars p2;
    Unmarshal(json_value, p2);

    BOOST_CHECK(p2.name_ == "John Doe");
    BOOST_CHECK(p2.age_ == 30);
    BOOST_CHECK(p2.city_ == "New York");
    BOOST_CHECK(p2.email_ == "john@example.com");

  } catch (const std::exception& e) {
    std::cout << "Error in 'TEST_PARSE_JSON_PERSON_SCALARS': " << e.what()
              << std::endl;
    BOOST_FAIL("Exception thrown: " + std::string(e.what()));
  }
}

BOOST_AUTO_TEST_CASE(TEST_PARSE_JSON_PERSON_LIST) {
  std::string json_str = data;

  try {
    boost::json::value json_value = boost::json::parse(json_str);

    PersonList person_list;
    Unmarshal(json_value, person_list);

    BOOST_CHECK(person_list.persons_ != nullptr);
    BOOST_CHECK(person_list.persons_->size() == 3);

    const auto& p1 = (*person_list.persons_)[0];
    BOOST_CHECK(p1.name_ != nullptr && *p1.name_ == "John Doe");
    BOOST_CHECK(p1.age_ != nullptr && *p1.age_ == 30);
    BOOST_CHECK(p1.city_ != nullptr && *p1.city_ == "New York");
    BOOST_CHECK(p1.email_ != nullptr && *p1.email_ == "john@example.com");

    const auto& p2 = (*person_list.persons_)[1];
    BOOST_CHECK(p2.name_ != nullptr && *p2.name_ == "Jane Smith");
    BOOST_CHECK(p2.age_ != nullptr && *p2.age_ == 25);
    BOOST_CHECK(p2.city_ != nullptr && *p2.city_ == "Los Angeles");
    BOOST_CHECK(p2.email_ == nullptr);

    const auto& p3 = (*person_list.persons_)[2];
    BOOST_CHECK(p3.name_ != nullptr && *p3.name_ == "Bob Johnson");
    BOOST_CHECK(p3.age_ != nullptr && *p3.age_ == 35);
    BOOST_CHECK(p3.city_ != nullptr && *p3.city_ == "Chicago");
    BOOST_CHECK(p3.email_ == nullptr);
  } catch (const std::exception& e) {
    std::cout << "Error in 'TEST_PARSE_JSON_PERSON_LIST': " << e.what()
              << std::endl;
    BOOST_FAIL("Exception thrown: " + std::string(e.what()));
  }
}

BOOST_AUTO_TEST_CASE(TEST_PARSE_JSON_PERSON_LIST_UNIQUE_PTR) {
  std::string json_str = data;

  try {
    boost::json::value json_value = boost::json::parse(json_str);

    PersonListUniquePtr person_list;
    Unmarshal(json_value, person_list);

    BOOST_CHECK(person_list.persons_ != nullptr);
    BOOST_CHECK(person_list.persons_->size() == 3);

    const auto& p1 = (*person_list.persons_)[0];
    BOOST_CHECK(p1 != nullptr);
    BOOST_CHECK(p1->name_ != nullptr && *p1->name_ == "John Doe");
    BOOST_CHECK(p1->age_ != nullptr && *p1->age_ == 30);
    BOOST_CHECK(p1->city_ != nullptr && *p1->city_ == "New York");
    BOOST_CHECK(p1->email_ != nullptr && *p1->email_ == "john@example.com");

    const auto& p2 = (*person_list.persons_)[1];
    BOOST_CHECK(p2 != nullptr);
    BOOST_CHECK(p2->name_ != nullptr && *p2->name_ == "Jane Smith");
    BOOST_CHECK(p2->age_ != nullptr && *p2->age_ == 25);
    BOOST_CHECK(p2->city_ != nullptr && *p2->city_ == "Los Angeles");
    BOOST_CHECK(p2->email_ == nullptr);

    const auto& p3 = (*person_list.persons_)[2];
    BOOST_CHECK(p3 != nullptr);
    BOOST_CHECK(p3->name_ != nullptr && *p3->name_ == "Bob Johnson");
    BOOST_CHECK(p3->age_ != nullptr && *p3->age_ == 35);
    BOOST_CHECK(p3->city_ != nullptr && *p3->city_ == "Chicago");
    BOOST_CHECK(p3->email_ == nullptr);
  } catch (const std::exception& e) {
    std::cout << "Error in 'TEST_PARSE_JSON_PERSON_LIST_UNIQUE_PTR': "
              << e.what() << std::endl;
    BOOST_FAIL("Exception thrown: " + std::string(e.what()));
  }
}

BOOST_AUTO_TEST_CASE(TEST_PARSE_JSON_PERSON_LIST_SHARED_PTR) {
  std::string json_str = data;

  try {
    boost::json::value json_value = boost::json::parse(json_str);

    PersonListSharedPtr person_list;
    Unmarshal(json_value, person_list);

    BOOST_CHECK(person_list.persons_ != nullptr);
    BOOST_CHECK(person_list.persons_->size() == 3);

    const auto& p1 = (*person_list.persons_)[0];
    BOOST_CHECK(p1 != nullptr);
    BOOST_CHECK(p1->name_ != nullptr && *p1->name_ == "John Doe");
    BOOST_CHECK(p1->age_ != nullptr && *p1->age_ == 30);
    BOOST_CHECK(p1->city_ != nullptr && *p1->city_ == "New York");
    BOOST_CHECK(p1->email_ != nullptr && *p1->email_ == "john@example.com");

    const auto& p2 = (*person_list.persons_)[1];
    BOOST_CHECK(p2 != nullptr);
    BOOST_CHECK(p2->name_ != nullptr && *p2->name_ == "Jane Smith");
    BOOST_CHECK(p2->age_ != nullptr && *p2->age_ == 25);
    BOOST_CHECK(p2->city_ != nullptr && *p2->city_ == "Los Angeles");
    BOOST_CHECK(p2->email_ == nullptr);

    const auto& p3 = (*person_list.persons_)[2];
    BOOST_CHECK(p3 != nullptr);
    BOOST_CHECK(p3->name_ != nullptr && *p3->name_ == "Bob Johnson");
    BOOST_CHECK(p3->age_ != nullptr && *p3->age_ == 35);
    BOOST_CHECK(p3->city_ != nullptr && *p3->city_ == "Chicago");
    BOOST_CHECK(p3->email_ == nullptr);
  } catch (const std::exception& e) {
    std::cout << "Error in 'TEST_PARSE_JSON_PERSON_LIST_SHARED_PTR': "
              << e.what() << std::endl;
    BOOST_FAIL("Exception thrown: " + std::string(e.what()));
  }
}

BOOST_AUTO_TEST_CASE(TEST_PARSE_JSON_PERSON_LIST_OPTIONAL) {
  std::string json_str = data_with_optional;

  try {
    boost::json::value json_value = boost::json::parse(json_str);

    PersonListOptional person_list;
    Unmarshal(json_value, person_list);

    BOOST_CHECK(person_list.persons_.has_value());
    BOOST_CHECK(person_list.persons_->size() == 4);

    const auto& p1 = (*person_list.persons_)[0];
    BOOST_CHECK(p1.has_value());
    BOOST_CHECK(p1->name_ != nullptr && *p1->name_ == "Ivy Chen");
    BOOST_CHECK(p1->age_ != nullptr && *p1->age_ == 33);
    BOOST_CHECK(p1->city_ != nullptr && *p1->city_ == "Denver");
    BOOST_CHECK(p1->email_ != nullptr && *p1->email_ == "ivy@example.com");

    const auto& p2 = (*person_list.persons_)[1];
    BOOST_CHECK(p2.has_value());
    BOOST_CHECK(p2->name_ != nullptr && *p2->name_ == "Jack Ryan");
    BOOST_CHECK(p2->age_ != nullptr && *p2->age_ == 38);
    BOOST_CHECK(p2->city_ != nullptr && *p2->city_ == "Portland");
    BOOST_CHECK(p2->email_ == nullptr);

    const auto& p3 = (*person_list.persons_)[2];
    BOOST_CHECK(!p3.has_value());

    const auto& p4 = (*person_list.persons_)[3];
    BOOST_CHECK(p4.has_value());
    BOOST_CHECK(p4->name_ != nullptr && *p4->name_ == "Kate Bishop");
    BOOST_CHECK(p4->age_ != nullptr && *p4->age_ == 26);
    BOOST_CHECK(p4->city_ != nullptr && *p4->city_ == "Seattle");
    BOOST_CHECK(p4->email_ == nullptr);
  } catch (const std::exception& e) {
    std::cout << "Error in 'TEST_PARSE_JSON_PERSON_LIST_OPTIONAL': " << e.what()
              << std::endl;
    BOOST_FAIL("Exception thrown: " + std::string(e.what()));
  }
}

BOOST_AUTO_TEST_CASE(TEST_PARSE_JSON_PERSON_VECTOR) {
  try {
    PersonVector people1;

    Person p1;
    p1.name_ = std::make_unique<std::string>("Alice Cooper");
    p1.age_ = std::make_unique<std::uint64_t>(28);
    p1.city_ = std::make_unique<std::string>("Boston");
    p1.email_ = std::make_unique<std::string>("alice@example.com");
    people1.push_back(std::move(p1));

    Person p2;
    p2.name_ = std::make_unique<std::string>("David Wilson");
    p2.age_ = std::make_unique<std::uint64_t>(42);
    p2.city_ = std::make_unique<std::string>("Seattle");
    p2.email_ = nullptr;
    people1.push_back(std::move(p2));

    boost::json::value json_value = Marshal(people1);
    std::cout << "==> TEST_PARSE_JSON_PERSON_VECTOR" << std::endl;
    std::cout << json_value << std::endl;
    std::cout << std::endl;

    PersonVector people2;
    Unmarshal(json_value, people2);

    BOOST_CHECK(people2.size() == 2);

    const auto& person1 = people2[0];
    BOOST_CHECK(person1.name_ != nullptr && *person1.name_ == "Alice Cooper");
    BOOST_CHECK(person1.age_ != nullptr && *person1.age_ == 28);
    BOOST_CHECK(person1.city_ != nullptr && *person1.city_ == "Boston");
    BOOST_CHECK(person1.email_ != nullptr &&
                *person1.email_ == "alice@example.com");

    const auto& person2 = people2[1];
    BOOST_CHECK(person2.name_ != nullptr && *person2.name_ == "David Wilson");
    BOOST_CHECK(person2.age_ != nullptr && *person2.age_ == 42);
    BOOST_CHECK(person2.city_ != nullptr && *person2.city_ == "Seattle");
    BOOST_CHECK(person2.email_ == nullptr);
  } catch (const std::exception& e) {
    std::cout << "Error in 'TEST_PARSE_JSON_PERSON_VECTOR': " << e.what()
              << std::endl;
    BOOST_FAIL("Exception thrown: " + std::string(e.what()));
  }
}

BOOST_AUTO_TEST_CASE(TEST_PARSE_JSON_PERSON_VECTOR_SHARED_PTR) {
  try {
    PersonVectorSharedPtr people1;

    auto p1 = std::make_shared<Person>();
    p1->name_ = std::make_unique<std::string>("Alice Cooper");
    p1->age_ = std::make_unique<std::uint64_t>(28);
    p1->city_ = std::make_unique<std::string>("Boston");
    p1->email_ = std::make_unique<std::string>("alice@example.com");
    people1.push_back(p1);

    auto p2 = std::make_shared<Person>();
    p2->name_ = std::make_unique<std::string>("David Wilson");
    p2->age_ = std::make_unique<std::uint64_t>(42);
    p2->city_ = std::make_unique<std::string>("Seattle");
    p2->email_ = nullptr;
    people1.push_back(p2);

    boost::json::value json_value = Marshal(people1);
    std::cout << "==> TEST_PARSE_JSON_PERSON_VECTOR_SHARED_PTR" << std::endl;
    std::cout << json_value << std::endl;
    std::cout << std::endl;

    PersonVectorSharedPtr people2;
    Unmarshal(json_value, people2);

    BOOST_CHECK(people2.size() == 2);

    const auto& person1 = people2[0];
    BOOST_CHECK(person1 != nullptr);
    BOOST_CHECK(person1->name_ != nullptr && *person1->name_ == "Alice Cooper");
    BOOST_CHECK(person1->age_ != nullptr && *person1->age_ == 28);
    BOOST_CHECK(person1->city_ != nullptr && *person1->city_ == "Boston");
    BOOST_CHECK(person1->email_ != nullptr &&
                *person1->email_ == "alice@example.com");

    const auto& person2 = people2[1];
    BOOST_CHECK(person2 != nullptr);
    BOOST_CHECK(person2->name_ != nullptr && *person2->name_ == "David Wilson");
    BOOST_CHECK(person2->age_ != nullptr && *person2->age_ == 42);
    BOOST_CHECK(person2->city_ != nullptr && *person2->city_ == "Seattle");
    BOOST_CHECK(person2->email_ == nullptr);
  } catch (const std::exception& e) {
    std::cout << "Error in 'TEST_PARSE_JSON_PERSON_VECTOR_SHARED_PTR': "
              << e.what() << std::endl;
    BOOST_FAIL("Exception thrown: " + std::string(e.what()));
  }
}

BOOST_AUTO_TEST_CASE(TEST_PARSE_JSON_PERSON_VECTOR_UNIQUE_PTR) {
  try {
    PersonVectorUniquePtr people1;

    auto p1 = std::make_unique<Person>();
    p1->name_ = std::make_unique<std::string>("Alice Cooper");
    p1->age_ = std::make_unique<std::uint64_t>(28);
    p1->city_ = std::make_unique<std::string>("Boston");
    p1->email_ = std::make_unique<std::string>("alice@example.com");
    people1.push_back(std::move(p1));

    auto p2 = std::make_unique<Person>();
    p2->name_ = std::make_unique<std::string>("David Wilson");
    p2->age_ = std::make_unique<std::uint64_t>(42);
    p2->city_ = std::make_unique<std::string>("Seattle");
    p2->email_ = nullptr;
    people1.push_back(std::move(p2));

    boost::json::value json_value = Marshal(people1);
    std::cout << "==> TEST_PARSE_JSON_PERSON_VECTOR_UNIQUE_PTR" << std::endl;
    std::cout << json_value << std::endl;
    std::cout << std::endl;

    PersonVectorUniquePtr people2;
    Unmarshal(json_value, people2);

    BOOST_CHECK(people2.size() == 2);

    const auto& person1 = people2[0];
    BOOST_CHECK(person1 != nullptr);
    BOOST_CHECK(person1->name_ != nullptr && *person1->name_ == "Alice Cooper");
    BOOST_CHECK(person1->age_ != nullptr && *person1->age_ == 28);
    BOOST_CHECK(person1->city_ != nullptr && *person1->city_ == "Boston");
    BOOST_CHECK(person1->email_ != nullptr &&
                *person1->email_ == "alice@example.com");

    const auto& person2 = people2[1];
    BOOST_CHECK(person2 != nullptr);
    BOOST_CHECK(person2->name_ != nullptr && *person2->name_ == "David Wilson");
    BOOST_CHECK(person2->age_ != nullptr && *person2->age_ == 42);
    BOOST_CHECK(person2->city_ != nullptr && *person2->city_ == "Seattle");
    BOOST_CHECK(person2->email_ == nullptr);
  } catch (const std::exception& e) {
    std::cout << "Error in 'TEST_PARSE_JSON_PERSON_VECTOR_UNIQUE_PTR': "
              << e.what() << std::endl;
    BOOST_FAIL("Exception thrown: " + std::string(e.what()));
  }
}

BOOST_AUTO_TEST_CASE(TEST_PARSE_JSON_PERSON_VECTOR_OPTIONAL) {
  try {
    PersonVectorOptional people1;

    Person p1;
    p1.name_ = std::make_unique<std::string>("Alice Cooper");
    p1.age_ = std::make_unique<std::uint64_t>(28);
    p1.city_ = std::make_unique<std::string>("Boston");
    p1.email_ = std::make_unique<std::string>("alice@example.com");
    people1.push_back(std::move(p1));

    people1.push_back(std::nullopt);

    Person p3;
    p3.name_ = std::make_unique<std::string>("David Wilson");
    p3.age_ = std::make_unique<std::uint64_t>(42);
    p3.city_ = std::make_unique<std::string>("Seattle");
    p3.email_ = nullptr;
    people1.push_back(std::move(p3));

    boost::json::value json_value = Marshal(people1);
    std::cout << "==> TEST_PARSE_JSON_PERSON_VECTOR_OPTIONAL" << std::endl;
    std::cout << json_value << std::endl;
    std::cout << std::endl;

    PersonVectorOptional people2;
    Unmarshal(json_value, people2);

    BOOST_CHECK(people2.size() == 3);

    const auto& person1_opt = people2[0];
    BOOST_CHECK(person1_opt.has_value());
    const auto& person1 = *person1_opt;
    BOOST_CHECK(person1.name_ != nullptr && *person1.name_ == "Alice Cooper");
    BOOST_CHECK(person1.age_ != nullptr && *person1.age_ == 28);
    BOOST_CHECK(person1.city_ != nullptr && *person1.city_ == "Boston");
    BOOST_CHECK(person1.email_ != nullptr &&
                *person1.email_ == "alice@example.com");

    const auto& person2_opt = people2[1];
    BOOST_CHECK(!person2_opt.has_value());

    const auto& person3_opt = people2[2];
    BOOST_CHECK(person3_opt.has_value());
    const auto& person3 = *person3_opt;
    BOOST_CHECK(person3.name_ != nullptr && *person3.name_ == "David Wilson");
    BOOST_CHECK(person3.age_ != nullptr && *person3.age_ == 42);
    BOOST_CHECK(person3.city_ != nullptr && *person3.city_ == "Seattle");
    BOOST_CHECK(person3.email_ == nullptr);
  } catch (const std::exception& e) {
    std::cout << "Error in 'TEST_PARSE_JSON_PERSON_VECTOR_OPTIONAL': "
              << e.what() << std::endl;
    BOOST_FAIL("Exception thrown: " + std::string(e.what()));
  }
}

BOOST_AUTO_TEST_CASE(TEST_PARSE_JSON_PERSON_MAP) {
  try {
    PersonMap persons_map;

    Person p1;
    p1.name_ = std::make_unique<std::string>("John Doe");
    p1.age_ = std::make_unique<std::uint64_t>(30);
    p1.city_ = std::make_unique<std::string>("New York");
    p1.email_ = std::make_unique<std::string>("john@example.com");
    persons_map["john"] = std::move(p1);

    Person p2;
    p2.name_ = std::make_unique<std::string>("Jane Smith");
    p2.age_ = std::make_unique<std::uint64_t>(25);
    p2.city_ = std::make_unique<std::string>("Los Angeles");
    p2.email_ = nullptr;
    persons_map["jane"] = std::move(p2);

    Person p3;
    p3.name_ = std::make_unique<std::string>("Bob Johnson");
    p3.age_ = std::make_unique<std::uint64_t>(35);
    p3.city_ = std::make_unique<std::string>("Chicago");
    p3.email_ = nullptr;
    persons_map["bob"] = std::move(p3);

    boost::json::value json_value = Marshal(persons_map);
    std::cout << "==> TEST_PARSE_JSON_PERSON_MAP" << std::endl;
    std::cout << json_value << std::endl;
    std::cout << std::endl;

    PersonMap persons_map2;
    Unmarshal(json_value, persons_map2);

    BOOST_CHECK(persons_map2.size() == 3);

    const auto& john_it = persons_map2.find("john");
    BOOST_CHECK(john_it != persons_map2.end());
    const auto& john = john_it->second;
    BOOST_CHECK(john.name_ != nullptr && *john.name_ == "John Doe");
    BOOST_CHECK(john.age_ != nullptr && *john.age_ == 30);
    BOOST_CHECK(john.city_ != nullptr && *john.city_ == "New York");
    BOOST_CHECK(john.email_ != nullptr && *john.email_ == "john@example.com");

    const auto& jane_it = persons_map2.find("jane");
    BOOST_CHECK(jane_it != persons_map2.end());
    const auto& jane = jane_it->second;
    BOOST_CHECK(jane.name_ != nullptr && *jane.name_ == "Jane Smith");
    BOOST_CHECK(jane.age_ != nullptr && *jane.age_ == 25);
    BOOST_CHECK(jane.city_ != nullptr && *jane.city_ == "Los Angeles");
    BOOST_CHECK(jane.email_ == nullptr);

    const auto& bob_it = persons_map2.find("bob");
    BOOST_CHECK(bob_it != persons_map2.end());
    const auto& bob = bob_it->second;
    BOOST_CHECK(bob.name_ != nullptr && *bob.name_ == "Bob Johnson");
    BOOST_CHECK(bob.age_ != nullptr && *bob.age_ == 35);
    BOOST_CHECK(bob.city_ != nullptr && *bob.city_ == "Chicago");
    BOOST_CHECK(bob.email_ == nullptr);
  } catch (const std::exception& e) {
    std::cout << "Error in 'TEST_PARSE_JSON_PERSON_MAP': " << e.what()
              << std::endl;
    BOOST_FAIL("Exception thrown: " + std::string(e.what()));
  }
}

BOOST_AUTO_TEST_CASE(TEST_PARSE_JSON_PERSON_MAP_UNIQUE_PTR) {
  try {
    PersonMapUniquePtr persons_map;

    auto p1 = std::make_unique<Person>();
    p1->name_ = std::make_unique<std::string>("John Doe");
    p1->age_ = std::make_unique<std::uint64_t>(30);
    p1->city_ = std::make_unique<std::string>("New York");
    p1->email_ = std::make_unique<std::string>("john@example.com");
    persons_map["john"] = std::move(p1);

    auto p2 = std::make_unique<Person>();
    p2->name_ = std::make_unique<std::string>("Jane Smith");
    p2->age_ = std::make_unique<std::uint64_t>(25);
    p2->city_ = std::make_unique<std::string>("Los Angeles");
    p2->email_ = nullptr;
    persons_map["jane"] = std::move(p2);

    auto p3 = std::make_unique<Person>();
    p3->name_ = std::make_unique<std::string>("Bob Johnson");
    p3->age_ = std::make_unique<std::uint64_t>(35);
    p3->city_ = std::make_unique<std::string>("Chicago");
    p3->email_ = nullptr;
    persons_map["bob"] = std::move(p3);

    boost::json::value json_value = Marshal(persons_map);
    std::cout << "==> TEST_PARSE_JSON_PERSON_MAP_UNIQUE_PTR" << std::endl;
    std::cout << json_value << std::endl;
    std::cout << std::endl;

    PersonMapUniquePtr persons_map2;
    Unmarshal(json_value, persons_map2);

    BOOST_CHECK(persons_map2.size() == 3);

    const auto& john_it = persons_map2.find("john");
    BOOST_CHECK(john_it != persons_map2.end());
    const auto& john = john_it->second;
    BOOST_CHECK(john != nullptr);
    BOOST_CHECK(john->name_ != nullptr && *john->name_ == "John Doe");
    BOOST_CHECK(john->age_ != nullptr && *john->age_ == 30);
    BOOST_CHECK(john->city_ != nullptr && *john->city_ == "New York");
    BOOST_CHECK(john->email_ != nullptr && *john->email_ == "john@example.com");

    const auto& jane_it = persons_map2.find("jane");
    BOOST_CHECK(jane_it != persons_map2.end());
    const auto& jane = jane_it->second;
    BOOST_CHECK(jane != nullptr);
    BOOST_CHECK(jane->name_ != nullptr && *jane->name_ == "Jane Smith");
    BOOST_CHECK(jane->age_ != nullptr && *jane->age_ == 25);
    BOOST_CHECK(jane->city_ != nullptr && *jane->city_ == "Los Angeles");
    BOOST_CHECK(jane->email_ == nullptr);

    const auto& bob_it = persons_map2.find("bob");
    BOOST_CHECK(bob_it != persons_map2.end());
    const auto& bob = bob_it->second;
    BOOST_CHECK(bob != nullptr);
    BOOST_CHECK(bob->name_ != nullptr && *bob->name_ == "Bob Johnson");
    BOOST_CHECK(bob->age_ != nullptr && *bob->age_ == 35);
    BOOST_CHECK(bob->city_ != nullptr && *bob->city_ == "Chicago");
    BOOST_CHECK(bob->email_ == nullptr);
  } catch (const std::exception& e) {
    std::cout << "Error in 'TEST_PARSE_JSON_PERSON_MAP_UNIQUE_PTR': "
              << e.what() << std::endl;
    BOOST_FAIL("Exception thrown: " + std::string(e.what()));
  }
}

BOOST_AUTO_TEST_CASE(TEST_PARSE_JSON_PERSON_MAP_SHARED_PTR) {
  try {
    PersonMapSharedPtr persons_map;

    auto p1 = std::make_shared<Person>();
    p1->name_ = std::make_unique<std::string>("John Doe");
    p1->age_ = std::make_unique<std::uint64_t>(30);
    p1->city_ = std::make_unique<std::string>("New York");
    p1->email_ = std::make_unique<std::string>("john@example.com");
    persons_map["john"] = p1;

    auto p2 = std::make_shared<Person>();
    p2->name_ = std::make_unique<std::string>("Jane Smith");
    p2->age_ = std::make_unique<std::uint64_t>(25);
    p2->city_ = std::make_unique<std::string>("Los Angeles");
    p2->email_ = nullptr;
    persons_map["jane"] = p2;

    auto p3 = std::make_shared<Person>();
    p3->name_ = std::make_unique<std::string>("Bob Johnson");
    p3->age_ = std::make_unique<std::uint64_t>(35);
    p3->city_ = std::make_unique<std::string>("Chicago");
    p3->email_ = nullptr;
    persons_map["bob"] = p3;

    boost::json::value json_value = Marshal(persons_map);
    std::cout << "==> TEST_PARSE_JSON_PERSON_MAP_SHARED_PTR" << std::endl;
    std::cout << json_value << std::endl;
    std::cout << std::endl;

    PersonMapSharedPtr persons_map2;
    Unmarshal(json_value, persons_map2);

    BOOST_CHECK(persons_map2.size() == 3);

    const auto& john_it = persons_map2.find("john");
    BOOST_CHECK(john_it != persons_map2.end());
    const auto& john = john_it->second;
    BOOST_CHECK(john != nullptr);
    BOOST_CHECK(john->name_ != nullptr && *john->name_ == "John Doe");
    BOOST_CHECK(john->age_ != nullptr && *john->age_ == 30);
    BOOST_CHECK(john->city_ != nullptr && *john->city_ == "New York");
    BOOST_CHECK(john->email_ != nullptr && *john->email_ == "john@example.com");

    const auto& jane_it = persons_map2.find("jane");
    BOOST_CHECK(jane_it != persons_map2.end());
    const auto& jane = jane_it->second;
    BOOST_CHECK(jane != nullptr);
    BOOST_CHECK(jane->name_ != nullptr && *jane->name_ == "Jane Smith");
    BOOST_CHECK(jane->age_ != nullptr && *jane->age_ == 25);
    BOOST_CHECK(jane->city_ != nullptr && *jane->city_ == "Los Angeles");
    BOOST_CHECK(jane->email_ == nullptr);

    const auto& bob_it = persons_map2.find("bob");
    BOOST_CHECK(bob_it != persons_map2.end());
    const auto& bob = bob_it->second;
    BOOST_CHECK(bob != nullptr);
    BOOST_CHECK(bob->name_ != nullptr && *bob->name_ == "Bob Johnson");
    BOOST_CHECK(bob->age_ != nullptr && *bob->age_ == 35);
    BOOST_CHECK(bob->city_ != nullptr && *bob->city_ == "Chicago");
    BOOST_CHECK(bob->email_ == nullptr);
  } catch (const std::exception& e) {
    std::cout << "Error in 'TEST_PARSE_JSON_PERSON_MAP_SHARED_PTR': "
              << e.what() << std::endl;
    BOOST_FAIL("Exception thrown: " + std::string(e.what()));
  }
}

BOOST_AUTO_TEST_CASE(TEST_PARSE_JSON_PERSON_MAP_OPTIONAL) {
  try {
    PersonMapOptional persons_map;

    Person p1;
    p1.name_ = std::make_unique<std::string>("John Doe");
    p1.age_ = std::make_unique<std::uint64_t>(30);
    p1.city_ = std::make_unique<std::string>("New York");
    p1.email_ = std::make_unique<std::string>("john@example.com");
    persons_map["john"] = std::move(p1);

    Person p2;
    p2.name_ = std::make_unique<std::string>("Jane Smith");
    p2.age_ = std::make_unique<std::uint64_t>(25);
    p2.city_ = std::make_unique<std::string>("Los Angeles");
    p2.email_ = nullptr;
    persons_map["jane"] = std::move(p2);

    persons_map["bob"] = std::nullopt;

    Person p4;
    p4.name_ = std::make_unique<std::string>("Alice Cooper");
    p4.age_ = std::make_unique<std::uint64_t>(28);
    p4.city_ = std::make_unique<std::string>("Boston");
    p4.email_ = nullptr;
    persons_map["alice"] = std::move(p4);

    boost::json::value json_value = Marshal(persons_map);
    std::cout << "==> TEST_PARSE_JSON_PERSON_MAP_OPTIONAL" << std::endl;
    std::cout << json_value << std::endl;
    std::cout << std::endl;

    PersonMapOptional persons_map2;
    Unmarshal(json_value, persons_map2);

    BOOST_CHECK(persons_map2.size() == 4);

    const auto& john_it = persons_map2.find("john");
    BOOST_CHECK(john_it != persons_map2.end());
    const auto& john_opt = john_it->second;
    BOOST_CHECK(john_opt.has_value());
    const auto& john = *john_opt;
    BOOST_CHECK(john.name_ != nullptr && *john.name_ == "John Doe");
    BOOST_CHECK(john.age_ != nullptr && *john.age_ == 30);
    BOOST_CHECK(john.city_ != nullptr && *john.city_ == "New York");
    BOOST_CHECK(john.email_ != nullptr && *john.email_ == "john@example.com");

    const auto& jane_it = persons_map2.find("jane");
    BOOST_CHECK(jane_it != persons_map2.end());
    const auto& jane_opt = jane_it->second;
    BOOST_CHECK(jane_opt.has_value());
    const auto& jane = *jane_opt;
    BOOST_CHECK(jane.name_ != nullptr && *jane.name_ == "Jane Smith");
    BOOST_CHECK(jane.age_ != nullptr && *jane.age_ == 25);
    BOOST_CHECK(jane.city_ != nullptr && *jane.city_ == "Los Angeles");
    BOOST_CHECK(jane.email_ == nullptr);

    const auto& bob_it = persons_map2.find("bob");
    BOOST_CHECK(bob_it != persons_map2.end());
    const auto& bob_opt = bob_it->second;
    BOOST_CHECK(!bob_opt.has_value());
    const auto& alice_it = persons_map2.find("alice");
    BOOST_CHECK(alice_it != persons_map2.end());
    const auto& alice_opt = alice_it->second;
    BOOST_CHECK(alice_opt.has_value());
    const auto& alice = *alice_opt;
    BOOST_CHECK(alice.name_ != nullptr && *alice.name_ == "Alice Cooper");
    BOOST_CHECK(alice.age_ != nullptr && *alice.age_ == 28);
    BOOST_CHECK(alice.city_ != nullptr && *alice.city_ == "Boston");
    BOOST_CHECK(alice.email_ == nullptr);
  } catch (const std::exception& e) {
    std::cout << "Error in 'TEST_PARSE_JSON_PERSON_MAP_OPTIONAL': " << e.what()
              << std::endl;
    BOOST_FAIL("Exception thrown: " + std::string(e.what()));
  }
}

BOOST_AUTO_TEST_CASE(TEST_PARSE_JSON_PERSON_DICT) {
  try {
    auto persons_map = std::make_unique<PersonMap>();

    Person p1;
    p1.name_ = std::make_unique<std::string>("John Doe");
    p1.age_ = std::make_unique<std::uint64_t>(30);
    p1.city_ = std::make_unique<std::string>("New York");
    p1.email_ = std::make_unique<std::string>("john@example.com");
    (*persons_map)["john"] = std::move(p1);

    Person p2;
    p2.name_ = std::make_unique<std::string>("Jane Smith");
    p2.age_ = std::make_unique<std::uint64_t>(25);
    p2.city_ = std::make_unique<std::string>("Los Angeles");
    p2.email_ = nullptr;
    (*persons_map)["jane"] = std::move(p2);

    Person p3;
    p3.name_ = std::make_unique<std::string>("Bob Johnson");
    p3.age_ = std::make_unique<std::uint64_t>(35);
    p3.city_ = std::make_unique<std::string>("Chicago");
    p3.email_ = nullptr;
    (*persons_map)["bob"] = std::move(p3);

    PersonDict person_dict;
    person_dict.persons_ = std::move(persons_map);

    boost::json::value json_value = Marshal(person_dict);
    std::cout << "==> TEST_PARSE_JSON_PERSON_DICT" << std::endl;
    std::cout << json_value << std::endl;
    std::cout << std::endl;

    PersonDict person_dict2;
    Unmarshal(json_value, person_dict2);

    BOOST_CHECK(person_dict2.persons_ != nullptr);
    BOOST_CHECK(person_dict2.persons_->size() == 3);

    const auto& john_it = person_dict2.persons_->find("john");
    BOOST_CHECK(john_it != person_dict2.persons_->end());
    const auto& john = john_it->second;
    BOOST_CHECK(john.name_ != nullptr && *john.name_ == "John Doe");
    BOOST_CHECK(john.age_ != nullptr && *john.age_ == 30);
    BOOST_CHECK(john.city_ != nullptr && *john.city_ == "New York");
    BOOST_CHECK(john.email_ != nullptr && *john.email_ == "john@example.com");

    const auto& jane_it = person_dict2.persons_->find("jane");
    BOOST_CHECK(jane_it != person_dict2.persons_->end());
    const auto& jane = jane_it->second;
    BOOST_CHECK(jane.name_ != nullptr && *jane.name_ == "Jane Smith");
    BOOST_CHECK(jane.age_ != nullptr && *jane.age_ == 25);
    BOOST_CHECK(jane.city_ != nullptr && *jane.city_ == "Los Angeles");
    BOOST_CHECK(jane.email_ == nullptr);

    const auto& bob_it = person_dict2.persons_->find("bob");
    BOOST_CHECK(bob_it != person_dict2.persons_->end());
    const auto& bob = bob_it->second;
    BOOST_CHECK(bob.name_ != nullptr && *bob.name_ == "Bob Johnson");
    BOOST_CHECK(bob.age_ != nullptr && *bob.age_ == 35);
    BOOST_CHECK(bob.city_ != nullptr && *bob.city_ == "Chicago");
    BOOST_CHECK(bob.email_ == nullptr);
  } catch (const std::exception& e) {
    std::cout << "Error in 'TEST_PARSE_JSON_PERSON_DICT': " << e.what()
              << std::endl;
    BOOST_FAIL("Exception thrown: " + std::string(e.what()));
  }
}

BOOST_AUTO_TEST_CASE(TEST_PARSE_JSON_PERSON_DICT_UNIQUE_PTR) {
  try {
    auto persons_map = std::make_unique<PersonMapUniquePtr>();

    auto p1 = std::make_unique<Person>();
    p1->name_ = std::make_unique<std::string>("John Doe");
    p1->age_ = std::make_unique<std::uint64_t>(30);
    p1->city_ = std::make_unique<std::string>("New York");
    p1->email_ = std::make_unique<std::string>("john@example.com");
    (*persons_map)["john"] = std::move(p1);

    auto p2 = std::make_unique<Person>();
    p2->name_ = std::make_unique<std::string>("Jane Smith");
    p2->age_ = std::make_unique<std::uint64_t>(25);
    p2->city_ = std::make_unique<std::string>("Los Angeles");
    p2->email_ = nullptr;
    (*persons_map)["jane"] = std::move(p2);

    auto p3 = std::make_unique<Person>();
    p3->name_ = std::make_unique<std::string>("Bob Johnson");
    p3->age_ = std::make_unique<std::uint64_t>(35);
    p3->city_ = std::make_unique<std::string>("Chicago");
    p3->email_ = nullptr;
    (*persons_map)["bob"] = std::move(p3);

    PersonDictUniquePtr person_dict;
    person_dict.persons_ = std::move(persons_map);

    boost::json::value json_value = Marshal(person_dict);
    std::cout << "==> TEST_PARSE_JSON_PERSON_DICT_UNIQUE_PTR" << std::endl;
    std::cout << json_value << std::endl;
    std::cout << std::endl;

    PersonDictUniquePtr person_dict2;
    Unmarshal(json_value, person_dict2);

    BOOST_CHECK(person_dict2.persons_ != nullptr);
    BOOST_CHECK(person_dict2.persons_->size() == 3);

    const auto& john_it = person_dict2.persons_->find("john");
    BOOST_CHECK(john_it != person_dict2.persons_->end());
    BOOST_CHECK(john_it->second != nullptr);
    const auto& john = *john_it->second;
    BOOST_CHECK(john.name_ != nullptr && *john.name_ == "John Doe");
    BOOST_CHECK(john.age_ != nullptr && *john.age_ == 30);
    BOOST_CHECK(john.city_ != nullptr && *john.city_ == "New York");
    BOOST_CHECK(john.email_ != nullptr && *john.email_ == "john@example.com");

    const auto& jane_it = person_dict2.persons_->find("jane");
    BOOST_CHECK(jane_it != person_dict2.persons_->end());
    BOOST_CHECK(jane_it->second != nullptr);
    const auto& jane = *jane_it->second;
    BOOST_CHECK(jane.name_ != nullptr && *jane.name_ == "Jane Smith");
    BOOST_CHECK(jane.age_ != nullptr && *jane.age_ == 25);
    BOOST_CHECK(jane.city_ != nullptr && *jane.city_ == "Los Angeles");
    BOOST_CHECK(jane.email_ == nullptr);

    const auto& bob_it = person_dict2.persons_->find("bob");
    BOOST_CHECK(bob_it != person_dict2.persons_->end());
    BOOST_CHECK(bob_it->second != nullptr);
    const auto& bob = *bob_it->second;
    BOOST_CHECK(bob.name_ != nullptr && *bob.name_ == "Bob Johnson");
    BOOST_CHECK(bob.age_ != nullptr && *bob.age_ == 35);
    BOOST_CHECK(bob.city_ != nullptr && *bob.city_ == "Chicago");
    BOOST_CHECK(bob.email_ == nullptr);
  } catch (const std::exception& e) {
    std::cout << "Error in 'TEST_PARSE_JSON_PERSON_DICT_UNIQUE_PTR': "
              << e.what() << std::endl;
    BOOST_FAIL("Exception thrown: " + std::string(e.what()));
  }
}

BOOST_AUTO_TEST_CASE(TEST_PARSE_JSON_PERSON_DICT_SHARED_PTR) {
  try {
    auto persons_map = std::make_shared<PersonMapSharedPtr>();

    auto p1 = std::make_shared<Person>();
    p1->name_ = std::make_unique<std::string>("John Doe");
    p1->age_ = std::make_unique<std::uint64_t>(30);
    p1->city_ = std::make_unique<std::string>("New York");
    p1->email_ = std::make_unique<std::string>("john@example.com");
    (*persons_map)["john"] = p1;

    auto p2 = std::make_shared<Person>();
    p2->name_ = std::make_unique<std::string>("Jane Smith");
    p2->age_ = std::make_unique<std::uint64_t>(25);
    p2->city_ = std::make_unique<std::string>("Los Angeles");
    p2->email_ = nullptr;
    (*persons_map)["jane"] = p2;

    auto p3 = std::make_shared<Person>();
    p3->name_ = std::make_unique<std::string>("Bob Johnson");
    p3->age_ = std::make_unique<std::uint64_t>(35);
    p3->city_ = std::make_unique<std::string>("Chicago");
    p3->email_ = nullptr;
    (*persons_map)["bob"] = p3;

    PersonDictSharedPtr person_dict;
    person_dict.persons_ = persons_map;

    boost::json::value json_value = Marshal(person_dict);
    std::cout << "==> TEST_PARSE_JSON_PERSON_DICT_SHARED_PTR" << std::endl;
    std::cout << json_value << std::endl;
    std::cout << std::endl;

    PersonDictSharedPtr person_dict2;
    Unmarshal(json_value, person_dict2);

    BOOST_CHECK(person_dict2.persons_ != nullptr);
    BOOST_CHECK(person_dict2.persons_->size() == 3);

    const auto& john_it = person_dict2.persons_->find("john");
    BOOST_CHECK(john_it != person_dict2.persons_->end());
    BOOST_CHECK(john_it->second != nullptr);
    const auto& john = *john_it->second;
    BOOST_CHECK(john.name_ != nullptr && *john.name_ == "John Doe");
    BOOST_CHECK(john.age_ != nullptr && *john.age_ == 30);
    BOOST_CHECK(john.city_ != nullptr && *john.city_ == "New York");
    BOOST_CHECK(john.email_ != nullptr && *john.email_ == "john@example.com");

    const auto& jane_it = person_dict2.persons_->find("jane");
    BOOST_CHECK(jane_it != person_dict2.persons_->end());
    BOOST_CHECK(jane_it->second != nullptr);
    const auto& jane = *jane_it->second;
    BOOST_CHECK(jane.name_ != nullptr && *jane.name_ == "Jane Smith");
    BOOST_CHECK(jane.age_ != nullptr && *jane.age_ == 25);
    BOOST_CHECK(jane.city_ != nullptr && *jane.city_ == "Los Angeles");
    BOOST_CHECK(jane.email_ == nullptr);

    const auto& bob_it = person_dict2.persons_->find("bob");
    BOOST_CHECK(bob_it != person_dict2.persons_->end());
    BOOST_CHECK(bob_it->second != nullptr);
    const auto& bob = *bob_it->second;
    BOOST_CHECK(bob.name_ != nullptr && *bob.name_ == "Bob Johnson");
    BOOST_CHECK(bob.age_ != nullptr && *bob.age_ == 35);
    BOOST_CHECK(bob.city_ != nullptr && *bob.city_ == "Chicago");
    BOOST_CHECK(bob.email_ == nullptr);
  } catch (const std::exception& e) {
    std::cout << "Error in 'TEST_PARSE_JSON_PERSON_DICT_SHARED_PTR': "
              << e.what() << std::endl;
    BOOST_FAIL("Exception thrown: " + std::string(e.what()));
  }
}

BOOST_AUTO_TEST_CASE(TEST_PARSE_JSON_PERSON_DICT_OPTIONAL) {
  try {
    PersonMapOptional persons_map;

    Person p1;
    p1.name_ = std::make_unique<std::string>("John Doe");
    p1.age_ = std::make_unique<std::uint64_t>(30);
    p1.city_ = std::make_unique<std::string>("New York");
    p1.email_ = std::make_unique<std::string>("john@example.com");
    persons_map["john"] = std::move(p1);

    Person p2;
    p2.name_ = std::make_unique<std::string>("Jane Smith");
    p2.age_ = std::make_unique<std::uint64_t>(25);
    p2.city_ = std::make_unique<std::string>("Los Angeles");
    p2.email_ = nullptr;
    persons_map["jane"] = std::move(p2);

    persons_map["bob"] = std::nullopt;

    Person p4;
    p4.name_ = std::make_unique<std::string>("Alice Cooper");
    p4.age_ = std::make_unique<std::uint64_t>(28);
    p4.city_ = std::make_unique<std::string>("Boston");
    p4.email_ = nullptr;
    persons_map["alice"] = std::move(p4);

    PersonDictOptional person_dict;
    person_dict.persons_ = std::move(persons_map);

    boost::json::value json_value = Marshal(person_dict);
    std::cout << "==> TEST_PARSE_JSON_PERSON_DICT_OPTIONAL" << std::endl;
    std::cout << json_value << std::endl;
    std::cout << std::endl;

    PersonDictOptional person_dict2;
    Unmarshal(json_value, person_dict2);

    BOOST_CHECK(person_dict2.persons_.has_value());
    BOOST_CHECK(person_dict2.persons_->size() == 4);

    const auto& john_it = person_dict2.persons_->find("john");
    BOOST_CHECK(john_it != person_dict2.persons_->end());
    BOOST_CHECK(john_it->second.has_value());
    const auto& john = *john_it->second;
    BOOST_CHECK(john.name_ != nullptr && *john.name_ == "John Doe");
    BOOST_CHECK(john.age_ != nullptr && *john.age_ == 30);
    BOOST_CHECK(john.city_ != nullptr && *john.city_ == "New York");
    BOOST_CHECK(john.email_ != nullptr && *john.email_ == "john@example.com");

    const auto& jane_it = person_dict2.persons_->find("jane");
    BOOST_CHECK(jane_it != person_dict2.persons_->end());
    BOOST_CHECK(jane_it->second.has_value());
    const auto& jane = *jane_it->second;
    BOOST_CHECK(jane.name_ != nullptr && *jane.name_ == "Jane Smith");
    BOOST_CHECK(jane.age_ != nullptr && *jane.age_ == 25);
    BOOST_CHECK(jane.city_ != nullptr && *jane.city_ == "Los Angeles");
    BOOST_CHECK(jane.email_ == nullptr);

    const auto& bob_it = person_dict2.persons_->find("bob");
    BOOST_CHECK(bob_it != person_dict2.persons_->end());
    BOOST_CHECK(!bob_it->second.has_value());

    const auto& alice_it = person_dict2.persons_->find("alice");
    BOOST_CHECK(alice_it != person_dict2.persons_->end());
    BOOST_CHECK(alice_it->second.has_value());
    const auto& alice = *alice_it->second;
    BOOST_CHECK(alice.name_ != nullptr && *alice.name_ == "Alice Cooper");
    BOOST_CHECK(alice.age_ != nullptr && *alice.age_ == 28);
    BOOST_CHECK(alice.city_ != nullptr && *alice.city_ == "Boston");
    BOOST_CHECK(alice.email_ == nullptr);
  } catch (const std::exception& e) {
    std::cout << "Error in 'TEST_PARSE_JSON_PERSON_DICT_OPTIONAL': " << e.what()
              << std::endl;
    BOOST_FAIL("Exception thrown: " + std::string(e.what()));
  }
}

BOOST_AUTO_TEST_CASE(TEST_PARSE_JSON_ARBITRARY_LIST) {
  try {
    ArbitraryList values;

    values.push_back(boost::json::value("hello world"));
    values.push_back(boost::json::value(42));
    values.push_back(boost::json::value(true));
    values.push_back(boost::json::value({{"key", "value"}, {"number", 123}}));
    values.push_back(boost::json::value({1, 2, 3, 4, 5}));

    boost::json::value json_value = Marshal(values);
    std::cout << "==> TEST_PARSE_JSON_ARBITRARY_LIST" << std::endl;
    std::cout << json_value << std::endl;
    std::cout << std::endl;

    ArbitraryList values2;
    Unmarshal(json_value, values2);

    BOOST_CHECK(values2.size() == 5);

    BOOST_CHECK(values2[0].is_string() &&
                values2[0].as_string() == "hello world");
    BOOST_CHECK(values2[1].is_int64() && values2[1].as_int64() == 42);
    BOOST_CHECK(values2[2].is_bool() && values2[2].as_bool() == true);
    BOOST_CHECK(values2[3].is_object());
    BOOST_CHECK(values2[4].is_array() && values2[4].as_array().size() == 5);

  } catch (const std::exception& e) {
    std::cout << "Error in 'TEST_PARSE_JSON_ARBITRARY_LIST': " << e.what()
              << std::endl;
    BOOST_FAIL("Exception thrown: " + std::string(e.what()));
  }
}

BOOST_AUTO_TEST_CASE(TEST_PARSE_JSON_ARBITRARY_LIST_UNIQUE_PTR) {
  try {
    ArbitraryListUniquePtr values;

    values.push_back(std::make_unique<boost::json::value>("hello world"));
    values.push_back(std::make_unique<boost::json::value>(42));
    values.push_back(std::make_unique<boost::json::value>(true));
    values.push_back(nullptr);
    values.push_back(std::make_unique<boost::json::value>(
        boost::json::object{{"key", "value"}}));
    values.push_back(
        std::make_unique<boost::json::value>(boost::json::array{1, 2, 3}));

    boost::json::value json_value = Marshal(values);
    std::cout << "==> TEST_PARSE_JSON_ARBITRARY_LIST_UNIQUE_PTR" << std::endl;
    std::cout << json_value << std::endl;
    std::cout << std::endl;

    ArbitraryListUniquePtr values2;
    Unmarshal(json_value, values2);

    BOOST_CHECK(values2.size() == 6);

    BOOST_CHECK(values2[0] != nullptr && values2[0]->is_string() &&
                values2[0]->as_string() == "hello world");
    BOOST_CHECK(values2[1] != nullptr && values2[1]->is_int64() &&
                values2[1]->as_int64() == 42);
    BOOST_CHECK(values2[2] != nullptr && values2[2]->is_bool() &&
                values2[2]->as_bool() == true);
    BOOST_CHECK(values2[3] == nullptr);
    BOOST_CHECK(values2[4] != nullptr && values2[4]->is_object());
    BOOST_CHECK(values2[5] != nullptr && values2[5]->is_array() &&
                values2[5]->as_array().size() == 3);

  } catch (const std::exception& e) {
    std::cout << "Error in 'TEST_PARSE_JSON_ARBITRARY_LIST_UNIQUE_PTR': "
              << e.what() << std::endl;
    BOOST_FAIL("Exception thrown: " + std::string(e.what()));
  }
}

BOOST_AUTO_TEST_CASE(TEST_PARSE_JSON_ARBITRARY_LIST_SHARED_PTR) {
  try {
    ArbitraryListSharedPtr values;

    values.push_back(std::make_shared<boost::json::value>("hello world"));
    values.push_back(std::make_shared<boost::json::value>(42));
    values.push_back(std::make_shared<boost::json::value>(true));
    values.push_back(nullptr);
    values.push_back(std::make_shared<boost::json::value>(
        boost::json::object{{"key", "value"}}));
    values.push_back(
        std::make_shared<boost::json::value>(boost::json::array{1, 2, 3}));

    boost::json::value json_value = Marshal(values);
    std::cout << "==> TEST_PARSE_JSON_ARBITRARY_LIST_SHARED_PTR" << std::endl;
    std::cout << json_value << std::endl;
    std::cout << std::endl;

    ArbitraryListSharedPtr values2;
    Unmarshal(json_value, values2);

    BOOST_CHECK(values2.size() == 6);

    BOOST_CHECK(values2[0] != nullptr && values2[0]->is_string() &&
                values2[0]->as_string() == "hello world");
    BOOST_CHECK(values2[1] != nullptr && values2[1]->is_int64() &&
                values2[1]->as_int64() == 42);
    BOOST_CHECK(values2[2] != nullptr && values2[2]->is_bool() &&
                values2[2]->as_bool() == true);
    BOOST_CHECK(values2[3] == nullptr);
    BOOST_CHECK(values2[4] != nullptr && values2[4]->is_object());
    BOOST_CHECK(values2[5] != nullptr && values2[5]->is_array() &&
                values2[5]->as_array().size() == 3);

  } catch (const std::exception& e) {
    std::cout << "Error in 'TEST_PARSE_JSON_ARBITRARY_LIST_SHARED_PTR': "
              << e.what() << std::endl;
    BOOST_FAIL("Exception thrown: " + std::string(e.what()));
  }
}

BOOST_AUTO_TEST_CASE(TEST_PARSE_JSON_ARBITRARY_LIST_OPTIONAL) {
  try {
    ArbitraryListOptional values;

    values.push_back(boost::json::value("hello world"));
    values.push_back(boost::json::value(42));
    values.push_back(std::nullopt);
    values.push_back(boost::json::value(true));
    values.push_back(boost::json::value(boost::json::object{{"key", "value"}}));
    values.push_back(boost::json::value(boost::json::array{1, 2, 3}));

    boost::json::value json_value = Marshal(values);
    std::cout << "==> TEST_PARSE_JSON_ARBITRARY_LIST_OPTIONAL" << std::endl;
    std::cout << json_value << std::endl;
    std::cout << std::endl;

    ArbitraryListOptional values2;
    Unmarshal(json_value, values2);

    BOOST_CHECK(values2.size() == 6);

    BOOST_CHECK(values2[0].has_value() && values2[0]->is_string() &&
                values2[0]->as_string() == "hello world");
    BOOST_CHECK(values2[1].has_value() && values2[1]->is_int64() &&
                values2[1]->as_int64() == 42);
    BOOST_CHECK(!values2[2].has_value());
    BOOST_CHECK(values2[3].has_value() && values2[3]->is_bool() &&
                values2[3]->as_bool() == true);
    BOOST_CHECK(values2[4].has_value() && values2[4]->is_object());
    BOOST_CHECK(values2[5].has_value() && values2[5]->is_array() &&
                values2[5]->as_array().size() == 3);

  } catch (const std::exception& e) {
    std::cout << "Error in 'TEST_PARSE_JSON_ARBITRARY_LIST_OPTIONAL': "
              << e.what() << std::endl;
    BOOST_FAIL("Exception thrown: " + std::string(e.what()));
  }
}

BOOST_AUTO_TEST_CASE(TEST_PARSE_JSON_ARBITRARY_MAP) {
  try {
    ArbitraryMap values;

    values["string"] = boost::json::value("hello world");
    values["number"] = boost::json::value(42);
    values["boolean"] = boost::json::value(true);
    values["object"] = boost::json::value({{"key", "value"}, {"number", 123}});
    values["array"] = boost::json::value({1, 2, 3, 4, 5});

    boost::json::value json_value = Marshal(values);
    std::cout << "==> TEST_PARSE_JSON_ARBITRARY_MAP" << std::endl;
    std::cout << json_value << std::endl;
    std::cout << std::endl;

    ArbitraryMap values2;
    Unmarshal(json_value, values2);

    BOOST_CHECK(values2.size() == 5);

    BOOST_CHECK(values2["string"].is_string() &&
                values2["string"].as_string() == "hello world");
    BOOST_CHECK(values2["number"].is_int64() &&
                values2["number"].as_int64() == 42);
    BOOST_CHECK(values2["boolean"].is_bool() &&
                values2["boolean"].as_bool() == true);
    BOOST_CHECK(values2["object"].is_object());
    BOOST_CHECK(values2["array"].is_array() &&
                values2["array"].as_array().size() == 5);

  } catch (const std::exception& e) {
    std::cout << "Error in 'TEST_PARSE_JSON_ARBITRARY_MAP': " << e.what()
              << std::endl;
    BOOST_FAIL("Exception thrown: " + std::string(e.what()));
  }
}

BOOST_AUTO_TEST_CASE(TEST_PARSE_JSON_ARBITRARY_MAP_UNIQUE_PTR) {
  try {
    ArbitraryMapUniquePtr values;

    values["string"] = std::make_unique<boost::json::value>("hello world");
    values["number"] = std::make_unique<boost::json::value>(42);
    values["boolean"] = std::make_unique<boost::json::value>(true);
    values["null"] = nullptr;
    values["object"] = std::make_unique<boost::json::value>(
        boost::json::object{{"key", "value"}});
    values["array"] =
        std::make_unique<boost::json::value>(boost::json::array{1, 2, 3});

    boost::json::value json_value = Marshal(values);
    std::cout << "==> TEST_PARSE_JSON_ARBITRARY_MAP_UNIQUE_PTR" << std::endl;
    std::cout << json_value << std::endl;
    std::cout << std::endl;

    ArbitraryMapUniquePtr values2;
    Unmarshal(json_value, values2);

    BOOST_CHECK(values2.size() == 6);

    BOOST_CHECK(values2["string"] != nullptr &&
                values2["string"]->is_string() &&
                values2["string"]->as_string() == "hello world");
    BOOST_CHECK(values2["number"] != nullptr && values2["number"]->is_int64() &&
                values2["number"]->as_int64() == 42);
    BOOST_CHECK(values2["boolean"] != nullptr &&
                values2["boolean"]->is_bool() &&
                values2["boolean"]->as_bool() == true);
    BOOST_CHECK(values2["null"] == nullptr);
    BOOST_CHECK(values2["object"] != nullptr && values2["object"]->is_object());
    BOOST_CHECK(values2["array"] != nullptr && values2["array"]->is_array() &&
                values2["array"]->as_array().size() == 3);

  } catch (const std::exception& e) {
    std::cout << "Error in 'TEST_PARSE_JSON_ARBITRARY_MAP_UNIQUE_PTR': "
              << e.what() << std::endl;
    BOOST_FAIL("Exception thrown: " + std::string(e.what()));
  }
}

BOOST_AUTO_TEST_CASE(TEST_PARSE_JSON_ARBITRARY_MAP_SHARED_PTR) {
  try {
    ArbitraryMapSharedPtr values;

    values["string"] = std::make_shared<boost::json::value>("hello world");
    values["number"] = std::make_shared<boost::json::value>(42);
    values["boolean"] = std::make_shared<boost::json::value>(true);
    values["null"] = nullptr;
    values["object"] = std::make_shared<boost::json::value>(
        boost::json::object{{"key", "value"}});
    values["array"] =
        std::make_shared<boost::json::value>(boost::json::array{1, 2, 3});

    boost::json::value json_value = Marshal(values);
    std::cout << "==> TEST_PARSE_JSON_ARBITRARY_MAP_SHARED_PTR" << std::endl;
    std::cout << json_value << std::endl;
    std::cout << std::endl;

    ArbitraryMapSharedPtr values2;
    Unmarshal(json_value, values2);

    BOOST_CHECK(values2.size() == 6);

    BOOST_CHECK(values2["string"] != nullptr &&
                values2["string"]->is_string() &&
                values2["string"]->as_string() == "hello world");
    BOOST_CHECK(values2["number"] != nullptr && values2["number"]->is_int64() &&
                values2["number"]->as_int64() == 42);
    BOOST_CHECK(values2["boolean"] != nullptr &&
                values2["boolean"]->is_bool() &&
                values2["boolean"]->as_bool() == true);
    BOOST_CHECK(values2["null"] == nullptr);
    BOOST_CHECK(values2["object"] != nullptr && values2["object"]->is_object());
    BOOST_CHECK(values2["array"] != nullptr && values2["array"]->is_array() &&
                values2["array"]->as_array().size() == 3);

  } catch (const std::exception& e) {
    std::cout << "Error in 'TEST_PARSE_JSON_ARBITRARY_MAP_SHARED_PTR': "
              << e.what() << std::endl;
    BOOST_FAIL("Exception thrown: " + std::string(e.what()));
  }
}

BOOST_AUTO_TEST_CASE(TEST_PARSE_JSON_ARBITRARY_MAP_OPTIONAL) {
  try {
    ArbitraryMapOptional values;

    values["string"] = boost::json::value("hello world");
    values["number"] = boost::json::value(42);
    values["null"] = std::nullopt;
    values["boolean"] = boost::json::value(true);
    values["object"] =
        boost::json::value(boost::json::object{{"key", "value"}});
    values["array"] = boost::json::value(boost::json::array{1, 2, 3});

    boost::json::value json_value = Marshal(values);
    std::cout << "==> TEST_PARSE_JSON_ARBITRARY_MAP_OPTIONAL" << std::endl;
    std::cout << json_value << std::endl;
    std::cout << std::endl;

    ArbitraryMapOptional values2;
    Unmarshal(json_value, values2);

    BOOST_CHECK(values2.size() == 6);

    BOOST_CHECK(values2["string"].has_value() &&
                values2["string"]->is_string() &&
                values2["string"]->as_string() == "hello world");
    BOOST_CHECK(values2["number"].has_value() &&
                values2["number"]->is_int64() &&
                values2["number"]->as_int64() == 42);
    BOOST_CHECK(!values2["null"].has_value());
    BOOST_CHECK(values2["boolean"].has_value() &&
                values2["boolean"]->is_bool() &&
                values2["boolean"]->as_bool() == true);
    BOOST_CHECK(values2["object"].has_value() &&
                values2["object"]->is_object());
    BOOST_CHECK(values2["array"].has_value() && values2["array"]->is_array() &&
                values2["array"]->as_array().size() == 3);

  } catch (const std::exception& e) {
    std::cout << "Error in 'TEST_PARSE_JSON_ARBITRARY_MAP_OPTIONAL': "
              << e.what() << std::endl;
    BOOST_FAIL("Exception thrown: " + std::string(e.what()));
  }
}

BOOST_AUTO_TEST_CASE(TEST_PARSE_JSON_ARBITRARY) {
  try {
    Arbitrary value;
    value.value_ = boost::json::value("hello world");

    boost::json::value json_value = Marshal(value);
    std::cout << "==> TEST_PARSE_JSON_ARBITRARY" << std::endl;
    std::cout << json_value << std::endl;
    std::cout << std::endl;

    Arbitrary value2;
    Unmarshal(json_value, value2);

    BOOST_CHECK(value2.value_.is_string() &&
                value2.value_.as_string() == "hello world");

  } catch (const std::exception& e) {
    std::cout << "Error in 'TEST_PARSE_JSON_ARBITRARY': " << e.what()
              << std::endl;
    BOOST_FAIL("Exception thrown: " + std::string(e.what()));
  }
}

BOOST_AUTO_TEST_CASE(TEST_PARSE_JSON_ARBITRARY_UNIQUE_PTR) {
  try {
    ArbitraryUniquePtrStruct value = std::make_unique<Arbitrary>();
    value->value_ = boost::json::value(42);

    boost::json::value json_value = Marshal(value);
    std::cout << "==> TEST_PARSE_JSON_ARBITRARY_UNIQUE_PTR" << std::endl;
    std::cout << json_value << std::endl;
    std::cout << std::endl;

    ArbitraryUniquePtrStruct value2;
    Unmarshal(json_value, value2);

    BOOST_CHECK(value2 != nullptr && value2->value_.is_int64() &&
                value2->value_.as_int64() == 42);

  } catch (const std::exception& e) {
    std::cout << "Error in 'TEST_PARSE_JSON_ARBITRARY_UNIQUE_PTR': " << e.what()
              << std::endl;
    BOOST_FAIL("Exception thrown: " + std::string(e.what()));
  }
}

BOOST_AUTO_TEST_CASE(TEST_PARSE_JSON_ARBITRARY_SHARED_PTR) {
  try {
    ArbitrarySharedPtrStruct value = std::make_shared<Arbitrary>();
    value->value_ = boost::json::value(true);

    boost::json::value json_value = Marshal(value);
    std::cout << "==> TEST_PARSE_JSON_ARBITRARY_SHARED_PTR" << std::endl;
    std::cout << json_value << std::endl;
    std::cout << std::endl;

    ArbitrarySharedPtrStruct value2;
    Unmarshal(json_value, value2);

    BOOST_CHECK(value2 != nullptr && value2->value_.is_bool() &&
                value2->value_.as_bool() == true);

  } catch (const std::exception& e) {
    std::cout << "Error in 'TEST_PARSE_JSON_ARBITRARY_SHARED_PTR': " << e.what()
              << std::endl;
    BOOST_FAIL("Exception thrown: " + std::string(e.what()));
  }
}

BOOST_AUTO_TEST_CASE(TEST_PARSE_JSON_ARBITRARY_OPTIONAL) {
  try {
    ArbitraryOptionalStruct value;
    value.emplace();
    value->value_ = boost::json::value({{"key", "value"}});

    boost::json::value json_value = Marshal(value);
    std::cout << "==> TEST_PARSE_JSON_ARBITRARY_OPTIONAL" << std::endl;
    std::cout << json_value << std::endl;
    std::cout << std::endl;

    ArbitraryOptionalStruct value2;
    Unmarshal(json_value, value2);

    BOOST_CHECK(value2.has_value() && value2->value_.is_object());

  } catch (const std::exception& e) {
    std::cout << "Error in 'TEST_PARSE_JSON_ARBITRARY_OPTIONAL': " << e.what()
              << std::endl;
    BOOST_FAIL("Exception thrown: " + std::string(e.what()));
  }
}

BOOST_AUTO_TEST_CASE(TEST_PARSE_JSON_ARBITRARY_UNIQUE_PTR_MEMBER) {
  try {
    ArbitraryUniquePtr value;
    value.value_ = std::make_unique<boost::json::value>("hello world");

    boost::json::value json_value = Marshal(value);
    std::cout << "==> TEST_PARSE_JSON_ARBITRARY_UNIQUE_PTR_MEMBER" << std::endl;
    std::cout << json_value << std::endl;
    std::cout << std::endl;

    ArbitraryUniquePtr value2;
    Unmarshal(json_value, value2);

    BOOST_CHECK(value2.value_ != nullptr && value2.value_->is_string() &&
                value2.value_->as_string() == "hello world");

  } catch (const std::exception& e) {
    std::cout << "Error in 'TEST_PARSE_JSON_ARBITRARY_UNIQUE_PTR_MEMBER': "
              << e.what() << std::endl;
    BOOST_FAIL("Exception thrown: " + std::string(e.what()));
  }
}

BOOST_AUTO_TEST_CASE(TEST_PARSE_JSON_ARBITRARY_SHARED_PTR_MEMBER) {
  try {
    ArbitrarySharedPtr value;
    value.value_ = std::make_shared<boost::json::value>(42);

    boost::json::value json_value = Marshal(value);
    std::cout << "==> TEST_PARSE_JSON_ARBITRARY_SHARED_PTR_MEMBER" << std::endl;
    std::cout << json_value << std::endl;
    std::cout << std::endl;

    ArbitrarySharedPtr value2;
    Unmarshal(json_value, value2);

    BOOST_CHECK(value2.value_ != nullptr && value2.value_->is_int64() &&
                value2.value_->as_int64() == 42);

  } catch (const std::exception& e) {
    std::cout << "Error in 'TEST_PARSE_JSON_ARBITRARY_SHARED_PTR_MEMBER': "
              << e.what() << std::endl;
    BOOST_FAIL("Exception thrown: " + std::string(e.what()));
  }
}

BOOST_AUTO_TEST_CASE(TEST_PARSE_JSON_ARBITRARY_OPTIONAL_MEMBER) {
  try {
    ArbitraryOptional value;
    value.value_ = boost::json::value(true);

    boost::json::value json_value = Marshal(value);
    std::cout << "==> TEST_PARSE_JSON_ARBITRARY_OPTIONAL_MEMBER" << std::endl;
    std::cout << json_value << std::endl;
    std::cout << std::endl;

    ArbitraryOptional value2;
    Unmarshal(json_value, value2);

    BOOST_CHECK(value2.value_.has_value() && value2.value_->is_bool() &&
                value2.value_->as_bool() == true);

  } catch (const std::exception& e) {
    std::cout << "Error in 'TEST_PARSE_JSON_ARBITRARY_OPTIONAL_MEMBER': "
              << e.what() << std::endl;
    BOOST_FAIL("Exception thrown: " + std::string(e.what()));
  }
}

BOOST_AUTO_TEST_CASE(TEST_PARSE_JSON_ARBITRARY_VECTOR) {
  try {
    ArbitraryVector value;
    value.value_.push_back(boost::json::value("hello"));
    value.value_.push_back(boost::json::value(42));
    value.value_.push_back(boost::json::value(true));

    boost::json::value json_value = Marshal(value);
    std::cout << "==> TEST_PARSE_JSON_ARBITRARY_VECTOR" << std::endl;
    std::cout << json_value << std::endl;
    std::cout << std::endl;

    ArbitraryVector value2;
    Unmarshal(json_value, value2);

    BOOST_CHECK(value2.value_.size() == 3);
    BOOST_CHECK(value2.value_[0].is_string() &&
                value2.value_[0].as_string() == "hello");
    BOOST_CHECK(value2.value_[1].is_int64() &&
                value2.value_[1].as_int64() == 42);
    BOOST_CHECK(value2.value_[2].is_bool() &&
                value2.value_[2].as_bool() == true);

  } catch (const std::exception& e) {
    std::cout << "Error in 'TEST_PARSE_JSON_ARBITRARY_VECTOR': " << e.what()
              << std::endl;
    BOOST_FAIL("Exception thrown: " + std::string(e.what()));
  }
}

BOOST_AUTO_TEST_CASE(TEST_PARSE_JSON_ARBITRARY_DICT) {
  try {
    ArbitraryDict value;
    value.value_["string"] = boost::json::value("hello");
    value.value_["number"] = boost::json::value(42);
    value.value_["boolean"] = boost::json::value(true);

    boost::json::value json_value = Marshal(value);
    std::cout << "==> TEST_PARSE_JSON_ARBITRARY_DICT" << std::endl;
    std::cout << json_value << std::endl;
    std::cout << std::endl;

    ArbitraryDict value2;
    Unmarshal(json_value, value2);

    BOOST_CHECK(value2.value_.size() == 3);
    BOOST_CHECK(value2.value_["string"].is_string() &&
                value2.value_["string"].as_string() == "hello");
    BOOST_CHECK(value2.value_["number"].is_int64() &&
                value2.value_["number"].as_int64() == 42);
    BOOST_CHECK(value2.value_["boolean"].is_bool() &&
                value2.value_["boolean"].as_bool() == true);

  } catch (const std::exception& e) {
    std::cout << "Error in 'TEST_PARSE_JSON_ARBITRARY_DICT': " << e.what()
              << std::endl;
    BOOST_FAIL("Exception thrown: " + std::string(e.what()));
  }
}