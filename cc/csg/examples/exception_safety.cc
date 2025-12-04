#include <iostream>
#include <memory>
#include <optional>

class Resource {
public:
  Resource() {
    // Constructor ...
    std::cout << "Resource acquired" << std::endl;
  }
  ~Resource() {
    // Destructor ...
    std::cout << "Resource released" << std::endl;
  }
  void use() {
    // Using ...
    std::cout << "Using resource" << std::endl;
  }
};

void safe_function() {
  auto ptr = std::make_unique<Resource>();
  ptr->use();
  throw std::runtime_error("Simulated exception");
  // This line won't execute, but ptr is still cleaned up
}

void safe_function_with_smart_pointers(std::unique_ptr<Resource> unique_res,
 std::shared_ptr<Resource> shared_res,
 std::optional<Resource> opt_val) {
  if (unique_res) {
    unique_res->use();
  }
  if (shared_res) {
    shared_res->use();
  }
  if (opt_val) {
    opt_val->use();
  }
  // Simulate exception
  throw std::runtime_error("Simulated exception");
}

int main() {
  try {
    safe_function();
  } catch (...) {
    std::cout << "Exception caught, but resource was cleaned up" << std::endl;
  }

  auto unique = std::make_unique<Resource>();
  auto shared = std::make_shared<Resource>();
  std::optional<Resource> opt = Resource{};

  try {
    safe_function_with_smart_pointers(std::move(unique), shared, opt);
  } catch (...) {
    std::cout << "Exception caught, resources managed safely" << std::endl;
  }
  return 0;
}