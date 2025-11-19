#ifndef TEST_DATA_H_INCLUDED
#define TEST_DATA_H_INCLUDED

#include <string>

const std::string data = R"(
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

const std::string data_with_optional = R"(
  {
    "persons": [
      {
        "name": "Ivy Chen",
        "age": 33,
        "city": "Denver",
        "email": "ivy@example.com"
      },
      {
        "name": "Jack Ryan",
        "age": 38,
        "city": "Portland"
      },
      null,
      {
        "name": "Kate Bishop",
        "age": 26,
        "city": "Seattle",
        "email": null
      }
    ]
  }
  )";

#endif  // TEST_DATA_H_INCLUDED