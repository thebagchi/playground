#ifndef RFC6902_H_INCLUDED
#define RFC6902_H_INCLUDED

#include <boost/json.hpp>
#include <boost/optional.hpp>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace rfc6902 {

  // Get the kind of a JSON value as a string
  inline const char* Kind(const boost::json::value& val) {
    if (val.is_null()) {
      return "null";
    }
    if (val.is_bool()) {
      return "bool";
    }
    if (val.is_number()) {
      return "number";
    }
    if (val.is_string()) {
      return "string";
    }
    if (val.is_array()) {
      return "array";
    }
    if (val.is_object()) {
      return "object";
    }
    // This should never happen with valid boost::json::value
    throw std::runtime_error("Unknown JSON value kind");
  }

  // Extract a value from a JSON document using a JSON Pointer path
  // Returns empty optional if the path doesn't exist or is invalid
  inline boost::optional<const boost::json::value&> Extract(const boost::json::value& value,
                                                            const std::string& path) {
    if (path.empty()) {
      return value;
    }

    std::string_view normalized_path(path);

    // Remove leading slash if present
    if (!normalized_path.empty() && normalized_path[0] == '/') {
      normalized_path = normalized_path.substr(1);
    }

    // Split path by "/"
    std::vector<std::string_view> parts;
    size_t start = 0;
    size_t end = normalized_path.find('/');

    while (end != std::string_view::npos) {
      if (start != end) { // Skip empty parts
        parts.push_back(normalized_path.substr(start, end - start));
      }
      start = end + 1;
      end = normalized_path.find('/', start);
    }

    if (start < normalized_path.length()) {
      parts.push_back(normalized_path.substr(start));
    }

    boost::json::value* current = const_cast<boost::json::value*>(&value);

    for (const auto& part : parts) {
      if (part.empty()) {
        continue; // skip empty parts
      }

      if (current->is_object()) {
        const auto& obj = current->as_object();
        // Convert string_view to string for object lookup
        auto it = obj.find(std::string(part));
        if (it == obj.end()) {
          return boost::none; // path not found
        }
        current = const_cast<boost::json::value*>(&it->value());
      } else if (current->is_array()) {
        const auto& arr = current->as_array();
        try {
          // Convert string_view to string for stoul
          size_t index = std::stoul(std::string(part));
          if (index >= arr.size()) {
            return boost::none; // index out of bounds
          }
          current = const_cast<boost::json::value*>(&arr[index]);
        } catch (const std::exception&) {
          return boost::none; // invalid index
        }
      } else {
        return boost::none; // not an object or array, can't navigate further
      }
    }

    return *current;
  }

  // Type checking functions using Boost JSON
  inline bool IsNumber(const boost::json::value& val) {
    return val.is_number();
  }

  inline bool IsString(const boost::json::value& val) {
    return val.is_string();
  }

  inline bool IsNull(const boost::json::value& val) {
    return val.is_null();
  }

  inline bool IsBool(const boost::json::value& val) {
    return val.is_bool();
  }

  inline bool IsList(const boost::json::value& val) {
    return val.is_array();
  }

  inline bool IsDict(const boost::json::value& val) {
    return val.is_object();
  }

} // namespace rfc6902

#endif // RFC6902_H_INCLUDED
