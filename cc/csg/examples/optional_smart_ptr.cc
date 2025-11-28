#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <string>

// Function that may or may not create a unique_ptr
std::optional<std::unique_ptr<int>> create_optional_unique(bool create) {
  if (create) {
    return std::make_unique<int>(42);
  }
  return std::nullopt;  // Empty optional
}

// Function that may or may not create a shared_ptr
std::optional<std::shared_ptr<std::string>> create_optional_shared(
    bool create) {
  if (create) {
    return std::make_shared<std::string>("Hello Optional");
  }
  return std::nullopt;
}

int main() {
  // Example with optional unique_ptr
  auto opt_unique = create_optional_unique(true);
  if (opt_unique) {
    std::cout << "Optional unique value: " << **opt_unique << std::endl;
  } else {
    std::cout << "No value in optional unique" << std::endl;
  }

  // Example with optional shared_ptr
  auto opt_shared = create_optional_shared(true);
  if (opt_shared) {
    std::cout << "Optional shared value: " << **opt_shared << std::endl;
  }

  // Example with thread safety
  std::mutex mtx;
  std::optional<std::shared_ptr<int>> shared_opt = std::make_shared<int>(100);

  // Simulate thread-safe access
  {
    std::lock_guard<std::mutex> lock(mtx);
    if (shared_opt) {
      **shared_opt = 200;
      std::cout << "Modified shared optional value: " << **shared_opt
                << std::endl;
    }
  }

  // Moving from optional unique_ptr
  auto opt_unique2 = create_optional_unique(true);
  if (opt_unique2) {
    std::unique_ptr<int> moved_ptr = std::move(*opt_unique2);
    std::cout << "Moved value: " << *moved_ptr << std::endl;
    // After moving, the unique_ptr inside optional is now null
    // The optional still contains the null unique_ptr
    if (opt_unique2 && !*opt_unique2) {
      std::cout << "Unique_ptr inside optional is now null" << std::endl;
    }
    // To make the optional empty, we can reset it
    opt_unique2.reset();
    if (!opt_unique2) {
      std::cout << "Optional is now empty after reset" << std::endl;
    }
  }

  return 0;
}