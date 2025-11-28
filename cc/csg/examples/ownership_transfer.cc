#include <iostream>
#include <memory>

class Resource {
 public:
  Resource() { std::cout << "Resource acquired" << std::endl; }
  ~Resource() { std::cout << "Resource released" << std::endl; }
  void use() { std::cout << "Using resource" << std::endl; }
};

std::unique_ptr<Resource> create_resource() {
  return std::make_unique<Resource>();
}

void take_ownership(std::unique_ptr<Resource> ptr) {
  ptr->use();  // Now owns the resource
}

int main() {
  auto res = create_resource();  // Ownership transferred efficiently via move
                                 // (implicit)
  take_ownership(
      std::move(res));  // Explicit move to transfer ownership to function

  // Example with lambda capturing by move
  auto another_res = create_resource();
  auto lambda = [owned = std::move(another_res)]() mutable {
    take_ownership(std::move(owned));
  };
  lambda();
  return 0;
}