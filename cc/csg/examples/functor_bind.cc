#include <functional>
#include <iostream>
#include <memory>

void free_function(int value) {
  std::cout << "Free function called with: " << value << std::endl;
}

// Function that takes a callback
void process_with_callback(std::function<void(int)> callback, int value) {
  std::cout << "Processing with callback..." << std::endl;
  callback(value);
  std::cout << "Callback processed." << std::endl;
}

int main() {
  // Using std::function with free function
  process_with_callback(free_function, 42);

  // Using std::function with lambda
  auto lambda_callback = [](int value) {
    std::cout << "Lambda callback called with: " << value << std::endl;
  };
  process_with_callback(lambda_callback, 100);

  // Using std::function with bound function
  auto bound_callback = std::bind(free_function, std::placeholders::_1);
  process_with_callback(bound_callback, 200);

  // Using shared_ptr in lambda callback - this works because shared_ptr is
  // copyable
  auto shared_ptr = std::make_shared<int>(300);
  auto shared_callback = [ptr = shared_ptr](int value) {
    std::cout << "Smart pointer callback with value: " << value
              << ", ptr value: " << *ptr << std::endl;
  };
  process_with_callback(shared_callback, 400);

  // Using unique_ptr in lambda callback
  auto unique_ptr = std::make_unique<int>(500);
  auto unique_callback = [ptr = std::move(unique_ptr)](int value) mutable {
    if (ptr) {
      std::cout << "Unique pointer callback with value: " << value
                << ", ptr value: " << *ptr << std::endl;
    }
  };
  // std::function requires copyable callables, so using unique_ptr with
  // std::function is not possible We call it directly instead
  // Following line is not possible
  // process_with_callback(unique_callback, 400);
  unique_callback(600);

  return 0;
}