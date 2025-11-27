#include <iostream>
#include <memory>
#include <mutex>
#include <thread>

// Lambda capture with shared_ptr by reference
// Demonstrates thread-safe patterns for shared_ptr access in lambdas

std::mutex cout_mutex;

void safe_print(const std::string& msg) {
  std::lock_guard<std::mutex> lock(cout_mutex);
  std::cout << msg << std::endl;
}

void demonstrate_shared_ptr_lambda_ref() {
  auto shared_data = std::make_shared<std::string>("Shared Data");

  // Lambda capturing shared_ptr by reference - thread-safe reference counting
  auto lambda_ref = [&shared_data]() {
    // Access is safe due to shared_ptr's thread-safe reference counting
    safe_print("Lambda ref - Data: " + *shared_data);
  };

  // Lambda capturing shared_ptr by value - creates a copy
  auto lambda_value = [shared_data]() {
    safe_print("Lambda value - Data: " + *shared_data);
  };

  // Lambda with explicit reference capture
  auto lambda_explicit_ref = [&shared_data]() {
    safe_print("Lambda explicit ref - Data: " + *shared_data);
  };

  // Demonstrate in single thread first
  lambda_ref();
  lambda_value();
  lambda_explicit_ref();

  // Modify the shared data
  *shared_data = "Modified Shared Data";

  // Call again to show changes are reflected
  lambda_ref();
  lambda_value();
  lambda_explicit_ref();
}

int main() {
  demonstrate_shared_ptr_lambda_ref();
  return 0;
}