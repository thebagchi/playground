#include <iostream>
#include <memory>
#include <optional>

// Returns optional<int> for simple nullable value
std::optional<int> get_optional_value(bool available) {
  if (available) {
    return 42;
  }
  return std::nullopt;
}

// Returns unique_ptr<int> for exclusive ownership
std::unique_ptr<int> get_unique_value(bool available) {
  if (available) {
    return std::make_unique<int>(42);
  }
  return nullptr;
}

// Returns shared_ptr<int> for shared ownership
std::shared_ptr<int> get_shared_value(bool available) {
  if (available) {
    return std::make_shared<int>(42);
  }
  return nullptr;
}

int main() {
  // Optional
  auto opt = get_optional_value(true);
  if (opt) {
    std::cout << "Optional value: " << *opt << "\n";
  }

  // Unique
  auto uniq = get_unique_value(true);
  if (uniq) {
    std::cout << "Unique value: " << *uniq << "\n";
  }

  // Shared
  auto shared = get_shared_value(true);
  if (shared) {
    std::cout << "Shared value: " << *shared << "\n";
  }

  return 0;
}