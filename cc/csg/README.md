# Smart Pointer Safety Guidelines in C++

---

## 1. Ownership Principles

- Always use **smart pointers** (`std::unique_ptr` or `std::shared_ptr`) for objects with dynamic lifetime.
- Avoid raw `new` and `delete`. Use `std::make_unique` or `std::make_shared` for creation.
- Ensure exception safety: Smart pointers provide RAII, ensuring cleanup even if constructors throw. Prefer `std::make_unique` and `std::make_shared` for exception-safe object creation.

Source: [`exception_safety_example.cc`](exception_safety_example.cc)

```
+-------------------+
|    Object         |
+-------------------+
       ^
       |
+-------------------+    +-------------------+
| std::unique_ptr   |    | std::shared_ptr   |
+-------------------+    +-------------------+
```

## 2. Passing Objects to Functions
a) Non-owning Access

- Prefer const T& or T& when the object must exist (i.e. it is required).
- Use T* or const T* only when nullptr is a meaningful, valid input (i.e. the parameter is truly optional).
- Never use a raw pointer when the object is mandatory — if you write if (ptr) { ... } or assert(ptr != nullptr), you should have taken a reference instead.

For safe reference access from smart pointers, use utility functions like `Ref()` that check for null before dereferencing.

Source: [`non_owning_access_example.cc`](non_owning_access_example.cc)

### Non-owning Access

```
+----------------+      +------------------+
| Caller         | ---> | Function uses    |
| (owns via      |      | reference/pointer|
| smart ptr)     |      +------------------+
+----------------+
```

b) Ownership Transfer

- Use std::unique_ptr<T> when transferring ownership.
- Use std::shared_ptr<T> only when shared ownership is truly needed.
- Leverage move semantics for efficient ownership transfer with std::unique_ptr, avoiding unnecessary copies.

### Ownership Transfer

```
+-------------------+      +-------------------+
| Caller owns via   | ---> | Function takes    |
| unique_ptr        |      | ownership         |
+-------------------+      +-------------------+
```

Move semantics enhance efficiency when transferring ownership of smart pointers, particularly with std::unique_ptr. Using std::move allows transferring ownership without copying the underlying object, which is crucial for performance in resource-intensive applications.

Source: [`ownership_transfer_example.cc`](ownership_transfer_example.cc)

c) Avoid Passing Smart Pointers Everywhere

- Pass smart pointers only when ownership semantics matter.

### Avoid Passing Everywhere

```
+-------------------+
| Function          |
+-------------------+
       |
       v
+-------------------+
| Smart Pointer     |
| (only if needed)  |
+-------------------+
```

d) Lambda Captures

Use lambda captures when you need to create callable objects that maintain references to smart pointers. This is particularly useful for asynchronous operations, callbacks, event handlers, and algorithms requiring custom comparators or operations on smart pointer-managed objects.

### Key Considerations

- Capture by reference (`[&]`) when the lambda lifetime is shorter than the captured objects.
- Capture by value (`[=]`) only for `shared_ptr` (unique_ptr cannot be copied).
- Use specific captures (`[ptr = std::move(unique_ptr)]`) to transfer ownership into the lambda.
- Mark lambdas as `mutable` when they need to modify moved-in objects.

### Examples

#### Reference Capture

```cpp
auto lambda_shared_ptr_ref = [&shared_ptr]() {
  // Reference capture lifetime: The lambda captures shared_ptr by reference.
  // This is safe as long as the lambda is executed before the function ends
  // and shared_ptr remains in scope. However, if called from another thread
  // after the function returns, it would result in accessing a dangling
  // reference. Thus it's not safe to use in multi-threaded scenarios.
  std::cout << "Shared ptr by ref: " << *shared_ptr << "\n";
};
```

```cpp
auto lambda_unique_ptr_ref = [&unique_ptr]() {
  // Reference capture lifetime: The lambda captures unique_ptr by reference.
  // This is safe as long as the lambda is executed before the function ends
  // and unique_ptr remains in scope. Also, this lambda must be used before
  // unique_ptr is moved or invalidated. However, if called from another
  // thread after the function returns, it would result in accessing a
  // dangling reference. Thus it's not safe to use in multi-threaded scenarios.
  std::cout << "Unique ptr by ref: " << *unique_ptr << "\n";
};
```

#### Value Capture

```cpp
auto lambda_shared_ptr_value = [=]() {
  // shared_ptr itself is thread-safe, but accessing inner data (*shared_ptr)
  // requires RAII-based protection (e.g., mutex) for thread safety
  std::cout << "Captured by value - Shared: " << *shared_ptr << "\n";
};
```

#### Ownership Transfer (Move Capture)

```cpp
auto lambda_unique_ptr_value = [ptr = std::move(unique_ptr)]() mutable {
  // The moved unique_ptr (ptr) is now owned exclusively by this lambda. Since unique_ptr
  // ensures single ownership, synchronization may not required for accessing *ptr.
  std::cout << "Moved unique_ptr: " << *ptr << "\n";
};
// After moving unique_ptr into the lambda, the original unique_ptr is now
// null and should not be used.
```

Source: [`lamda_capture.cc`](lamda_capture.cc)

## 3. Returning Objects

Return std::unique_ptr<T> for exclusive ownership.
Return std::shared_ptr<T> for shared ownership.
Return by value or reference when ownership does not change.

```cpp
// Example functions
std::unique_ptr<int> create_unique() {
    return std::make_unique<int>(42);
}

std::shared_ptr<std::string> create_shared() {
    return std::make_shared<std::string>("Hello");
}

const std::string& get_reference(const std::string& str) {
    return str;
}
```


## 4. Safety Rules

- Never store raw pointers beyond the lifetime of the smart pointer.
- Avoid mixing raw and smart ownership.
- Use std::weak_ptr to break cycles.
- Ensure strong exception safety: Smart pointers provide automatic cleanup, but exceptions during object creation or transfer can lead to resource leaks if not handled. Use RAII consistently and prefer std::make_unique/std::make_shared for exception-safe construction.

### Cycle Problem

```
+-----------+      +-----------+
| shared A  | <--> | shared B  |
+-----------+      +-----------+
```

This diagram illustrates a cyclic reference between two shared_ptr objects, preventing automatic memory deallocation and causing memory leaks.

### Break Cycle

```
+-----------+      +-----------+
| shared A  | ---> | weak B    |
+-----------+      +-----------+

+-----------+      +-----------+
| shared B  | ---> | weak A    |
+-----------+      +-----------+
```

The cycle can be broken by making either pointer weak, depending on the ownership semantics.

## 5. Performance Considerations

- std::unique_ptr provides zero-overhead performance, equivalent to raw pointers, making it the best choice for performance-critical code.
- Use std::unique_ptr to avoid overhead from reference counting and atomic operations associated with other smart pointers.
- For hot paths, profile and ensure unique ownership with std::unique_ptr to optimize performance.


## 6. Example Pattern (Detailed)

Source: [`counter_example.cc`](counter_example.cc)

Key Points in Example:

Demonstrates const correctness and ownership semantics.
Shows std::unique_ptr with std::move for ownership transfer.


## 7. Example: Using std::weak_ptr to Avoid Cyclic Dependency

Source: [`weak_ptr_example.cc`](weak_ptr_example.cc)

Key Points:

std::weak_ptr does not increase reference count, preventing memory leaks from cyclic references.
Use .lock() to safely convert weak_ptr to shared_ptr before accessing the object.
Always check if .lock() returns a non-null pointer before using it.


## 8. Advanced Usage: Placement New with std::unique_ptr

Both std::unique_ptr and std::shared_ptr can be combined with placement new for custom memory management. Use a wrapper function with a lambda deleter to handle destruction and deallocation.
Source: [`placement_new_example.cc`](placement_new_example.cc)


### Key Points:

- The lambda deleter ensures proper destruction and deallocation.
- Use std::function<void(T*)> for the deleter type to handle the lambda.
- This is useful for custom allocators or memory pools.
- **Custom Deleter**: Essential because std::unique_ptr's default deleter uses delete, which assumes new was used for allocation. For placement new, you must manually handle destruction and deallocation.
- **Memory Management**: Ensure the memory is allocated (e.g., via malloc) and deallocated (e.g., via free) appropriately in the deleter.
- **Use Cases**: This is useful for custom allocators, embedded systems, or when integrating with legacy code that requires specific memory placement.
- **Alternatives**: If possible, prefer standard new with std::make_unique for simplicity. Placement new with smart pointers is advanced and error-prone if not handled correctly.


## 9. Advanced Topics and Edge Cases
Custom Deleters

Custom deleters allow std::unique_ptr to manage resources that require special cleanup logic, such as FILE pointers that need fclose() instead of delete.

Example:
```cpp
auto custom_deleter = [](int* ptr) {
    // Custom cleanup logic
    delete ptr;
};
std::unique_ptr<int, decltype(custom_deleter)> ptr(new int(42), custom_deleter);
```

Source: [`unique_ptr_deleter_example.cc`](unique_ptr_deleter_example.cc)

## 10. RAII with Shared Pointers

RAII (Resource Acquisition Is Initialization) ensures resources are properly managed through object lifetime. With `std::shared_ptr`, RAII provides thread-safe reference counting and automatic cleanup.

Source: [`raii_shared_ptr_example.cc`](raii_shared_ptr_example.cc)

Key Points:

- RAII ensures resources are cleaned up when objects go out of scope
- `std::shared_ptr` provides thread-safe reference counting
- Thread-safe access with `std::lock_guard`
- Safe to pass `shared_ptr` to threads and lambda functions
- Automatic cleanup prevents resource leaks

### Thread Safety

- std::shared_ptr reference counting is thread-safe (atomic operations), but the pointed-to object is not. Protect access to the object with mutexes if shared across threads.
- **Note:** Multiple threads may access the same shared_ptr instance safely (control block is atomic), but never access the same shared_ptr from different threads without synchronization if one modifies it (e.g., reset(), assignment).
- std::unique_ptr and std::weak_ptr are not thread-safe; use external synchronization

RAII Mutex Locking
To ensure thread-safe access to shared objects managed by smart pointers, use RAII with std::lock_guard or std::unique_lock for automatic mutex unlocking:
```cpp
#include <mutex>
#include <memory>

std::mutex mtx;
std::shared_ptr<int> shared_data = std::make_shared<int>(42);

void thread_safe_access() {
    std::lock_guard<std::mutex> lock(mtx);
    // Access shared_data safely
    *shared_data = 100;
}
```
For more flexibility (e.g., conditional locking or timeouts), use std::unique_lock:
```cpp
void conditional_access() {
    std::unique_lock<std::mutex> lock(mtx, std::try_to_lock);
    if (lock.owns_lock()) {
        // Access shared_data safely
        *shared_data = 200;
    } else {
        // Handle lock failure
    }
}
```
This ensures the mutex is released automatically when the scope ends, preventing deadlocks and leaks.

### Double-Checked Locking with Atomics

Double-checked locking is a thread-safe singleton pattern that minimizes locking overhead by checking the initialization flag twice - once without locking and once with locking.

Source: [`double_check_lock_example.cc`](double_check_lock_example.cc)

## 11. Integration with Containers

Prefer std::vector<MyClass> for simplicity and performance.
Use std::unique_ptr<MyClass> for polymorphic objects.
Use std::shared_ptr<MyClass> only when truly shared.
Avoid raw pointers in containers.

## 12. Object Pooling with Smart Pointers

Use custom deleters in std::shared_ptr or std::unique_ptr to implement object pooling, recycling objects to reduce allocation overhead.
Example: ObjectPool for efficient string management. This demonstrates object pooling with smart pointers, using custom deleters for automatic recycling.
Objects need not be put back in the pool manually - they are recycled automatically when the smart pointer goes out of scope using custom deleters.

Source: [`memory_pool_example.cc`](memory_pool_example.cc)

## 13. Best Practices Checklist

- Use std::make_unique and std::make_shared for object creation.
- Prefer std::unique_ptr for exclusive ownership.
- Use std::shared_ptr only when multiple owners are required.
- Pass by reference (T& or const T&) whenever possible.
- Use raw pointers only for non-owning, optional access.
- Avoid passing smart pointers unless ownership semantics are needed.
- Use std::move when transferring ownership with std::unique_ptr.
- Break cyclic dependencies using std::weak_ptr.
- Never store raw pointers beyond the lifetime of their owner.
- Avoid mixing raw and smart ownership for the same object.

## 14. Building and Running

This project uses CMake for building. All examples require C++17 or later.

### Prerequisites
- CMake 3.10+
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)

### Build Instructions

```bash
# Create build directory
mkdir build
cd build

# Configure with CMake
cmake ..

# Build all examples
make
```

## 15. Decision Tree for Function Parameters

```
Need to pass an object to a function?
├── Does the function need to modify the object?
│   ├── Yes: Pass by reference (T&)
│   └── No: Pass by const reference (const T&)
├── Is nullptr a valid value?
│   ├── Yes: Pass raw pointer (T*)
│   └── No: Pass by reference
└── Does ownership transfer?
    ├── Yes: Pass smart pointer (unique_ptr or shared_ptr)
    └── No: See above
```

## 16. Integration with std::optional

std::optional (C++17) can be combined with smart pointers to represent optional ownership or values. This is useful when a function may or may not return or store an object.

### Key Points

- std::optional does not manage resources itself, but when containing smart pointers, it provides optional resource management.
- Use std::optional<std::unique_ptr<T>> for optional exclusive ownership.
- Use std::optional<std::shared_ptr<T>> for optional shared ownership.
- Prefer std::optional over raw pointers for optional parameters or return values.

### Examples

```cpp
#include <optional>
#include <memory>
#include <iostream>

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
```

This example demonstrates three approaches for handling potentially unavailable values or objects:

- `std::optional<int>`: For nullable values without ownership semantics.
- `std::unique_ptr<int>`: For nullable exclusive ownership of dynamically allocated objects.
- `std::shared_ptr<int>`: For nullable shared ownership of dynamically allocated objects.

**Key Insight:** All three methods enable graceful failure handling by allowing the caller to check for availability and respond appropriately (e.g., provide defaults, retry operations, or log errors) without relying on exceptions or undefined behavior. Choose based on whether you need a simple value, exclusive ownership, or shared ownership.

### Safety Considerations

- When moving from std::optional<std::unique_ptr>, the optional becomes empty.
- std::optional<std::shared_ptr> benefits from shared_ptr's thread-safe reference counting.
- Always check if the optional has a value before dereferencing.
- Use std::move when transferring ownership from optional unique_ptr.

Source: [`optional_smart_ptr_example.cc`](optional_smart_ptr_example.cc)

References

[C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)
[cppreference.com - Smart Pointers](https://en.cppreference.com/w/cpp/memory)