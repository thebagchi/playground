#include <iostream>
#include <memory>

class Counter {
public:
  explicit Counter(int v = 0) : value_(v) {
    // Do Nothing
  }

  void Increment() {
    // Modification to object
    value_ = value_ + 1;
  }

  int GetValue() const {
    // No modification to object
    return value_;
  }

  void Display() const {
    // No Modification to object
    std::cout << "Value: " << value_ << std::endl;
  }
private:
  int value_;
};

// Non-owning usage
void IncrementCounter(Counter* c) {
  if (c != nullptr) {
    c->Increment();
    c->Display();
  }
}

// Read-only usage
void DisplayCounterValue(const Counter& c) {
  std::cout << "Read-only value: " << c.GetValue() << std::endl;
}

// Shared ownership usage
void IncrementSharedCounter(const std::shared_ptr<Counter>& c) {
  c->Increment();
  std::cout << "Shared counter value: " << c->GetValue() << std::endl;
}

// Unique ownership transfer using move semantics
void TransferUniqueOwnership(std::unique_ptr<Counter> c) {
  c->Increment();
  std::cout << "Unique ownership counter value: " << c->GetValue() << std::endl;
}

int main() {
  auto counter_ptr = std::make_shared<Counter>(10);

  IncrementCounter(counter_ptr.get()); // Non-owning
  DisplayCounterValue(*counter_ptr);   // Read-only
  IncrementSharedCounter(counter_ptr); // Shared ownership

  counter_ptr->Display();

  // Demonstrate unique ownership transfer
  auto unique_counter = std::make_unique<Counter>(20);
  TransferUniqueOwnership(std::move(unique_counter));

  return 0;
}