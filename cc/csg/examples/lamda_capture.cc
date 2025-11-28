#include <functional>
#include <iostream>
#include <memory>

// Lambda capture with smart pointers
// Use lambda captures when you need to create callable objects that maintain
// references to smart pointers. This is particularly useful for:
// - Asynchronous operations and callbacks
// - Event handlers that need to access shared resources
// - Algorithms that require custom comparators or operations on smart
// pointer-managed objects
//
// Key considerations:
// - Capture by reference ([&]) when the lambda lifetime is shorter than the
// captured objects
// - Capture by value ([=]) only for shared_ptr (unique_ptr cannot be copied)
// - Use specific captures ([ptr = std::move(unique_ptr)]) to transfer ownership
// into the lambda
// - Mark lambdas as mutable when they need to modify moved-in objects

void demonstrate_lambda_capture() {
  auto unique_ptr = std::make_unique<int>(42);
  auto shared_ptr = std::make_shared<std::string>("Hello");

  // Capture by reference - call this first before moving
  auto lambda_shared_ptr_ref = [&shared_ptr]() {
    // Reference capture lifetime: The lambda captures shared_ptr by reference.
    // This is safe as long as the lambda is executed before the function ends
    // and shared_ptr remains in scope. However, if called from another thread
    // after the function returns, it would result in accessing a dangling
    // reference. Thus its not safe to use in multi-threaded scenario.

    std::cout << "Shared ptr by ref: " << *shared_ptr << "\n";
  };
  auto lambda_unique_ptr_ref = [&unique_ptr]() {
    // Reference capture lifetime: The lambda captures unique_ptr by reference.
    // This is safe as long as the lambda is executed before the function ends
    // and unique_ptr remains in scope. Also, this lambda must be used before
    // unique_ptr is moved or invalidated. However, if called from another
    // thread after the function returns, it would result in accessing a
    // dangling reference. Thus its not safe to use in multi-threaded scenario.
    std::cout << "Unique ptr by ref: " << *unique_ptr << "\n";
  };

  lambda_shared_ptr_ref();  // Demonstrate shared_ptr capture by reference
  lambda_unique_ptr_ref();  // Demonstrate unique_ptr capture by reference

  // Capture shared_ptr by value (copy is allowed)
  auto lambda_shared_ptr_value = [=]() {
    // shared_ptr itself is thread-safe, but accessing inner data (*shared_ptr)
    // requires RAII-based protection (e.g., mutex) for thread safety
    std::cout << "Captured by value - Shared: " << *shared_ptr << "\n";
  };

  // Capture specific variables - move unique_ptr
  auto lambda_unique_ptr_value = [ptr = std::move(unique_ptr)]() mutable {
    // The moved unique_ptr (ptr) is now owned exclusively by this lambda. Since
    // unique_ptr ensures single ownership, synchronization may not required for
    // accessing *ptr.
    std::cout << "Moved unique_ptr: " << *ptr << "\n";
  };
  // After moving unique_ptr into the lambda, the original unique_ptr is now
  // null and should not be used.

  lambda_shared_ptr_value();
  lambda_unique_ptr_value();
}

int main() {
  demonstrate_lambda_capture();
  return 0;
}