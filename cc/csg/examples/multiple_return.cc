#include <iostream>
#include <memory>
#include <tuple>

std::tuple<std::unique_ptr<int>, std::shared_ptr<double>, bool> create_resources() {
  auto up = std::make_unique<int>(42);
  auto sp = std::make_shared<double>(3.14);
  bool success = true;
  return std::make_tuple(std::move(up), sp, success);
}

int main() {
  auto [unique_ptr, shared_ptr, ok] = create_resources();
  if (ok) {
    std::cout << "Unique: " << *unique_ptr << ", Shared: " << *shared_ptr << "\n";
  }
  return 0;
}