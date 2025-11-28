#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>

class Object {
 public:
  Object(int value) : value_(value) {
    // Constructor ...
    std::cout << "Object created with value: " << value_ << std::endl;
  }
  ~Object() {
    // Destructor ...
    std::cout << "Object destroyed" << std::endl;
  }
  int getValue() const {
    // Constant function ...
    return value_;
  }
  void setValue(int value) {
    // Object is modified ...
    value_ = value;
    std::cout << "Object value set to: " << value_ << std::endl;
  }

 private:
  int value_;
};

// Utility function to safely get reference from smart pointer
template <typename T>
T& Ref(const std::unique_ptr<T>& ptr) {
  if (!ptr) {
    throw std::runtime_error("Cannot dereference null unique_ptr");
  }
  return *ptr;
}

template <typename T>
T& Ref(const std::shared_ptr<T>& ptr) {
  if (!ptr) {
    throw std::runtime_error("Cannot dereference null shared_ptr");
  }
  return *ptr;
}

template <typename T>
const T& Ref(const std::optional<T>& opt) {
  if (!opt.has_value()) {
    throw std::runtime_error("Cannot dereference empty optional");
  }
  return *opt;
}

// Required → reference
void Process(const Object& w) {
  std::cout << "Processing read-only object with value: " << w.getValue()
            << std::endl;
}

// Required and modifiable → non-const reference
void ProcessMutable(Object& w) {
  std::cout << "Processing mutable object, changing value..." << std::endl;
  w.setValue(w.getValue() + 10);
}

// Truly optional → raw pointer (acceptable)
void ProcessOptional(const Object* w) {
  if (w) {
    std::cout << "Processing optional object with value: " << w->getValue()
              << std::endl;
  } else {
    std::cout << "No object provided (nullptr)" << std::endl;
  }
}

int main() {
  auto obj = std::make_unique<Object>(42);
  std::optional<Object> opt_obj = Object(100);

  // Demonstrate non-owning access patterns using Ref utility
  try {
    Process(Ref(obj));         // Required → reference
    ProcessMutable(Ref(obj));  // Required and modifiable → non-const reference
    Process(Ref(opt_obj));     // Required from optional → reference
  } catch (const std::exception& e) {
    std::cout << "Exception: " << e.what() << std::endl;
  }
  ProcessOptional(obj.get());  // Truly optional → raw pointer

  // Demonstrate optional with nullptr
  ProcessOptional(nullptr);

  return 0;
}