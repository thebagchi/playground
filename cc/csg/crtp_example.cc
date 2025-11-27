#include <iostream>
#include <memory>

template <typename Derived>
class Base {
 public:
  void interface() { static_cast<Derived*>(this)->implementation(); }
};

class Derived1 : public Base<Derived1> {
 public:
  void implementation() { std::cout << "Derived1 implementation\n"; }
};

class Derived2 : public Base<Derived2> {
 public:
  void implementation() { std::cout << "Derived2 implementation\n"; }
};

template <typename T>
void call_interface(Base<T>& obj) {
  obj.interface();
}

int main() {
  auto d1 = std::make_unique<Derived1>();
  auto d2 = std::make_shared<Derived2>();

  call_interface(*d1);  // Calls Derived1::implementation
  call_interface(*d2);  // Calls Derived2::implementation

  return 0;
}