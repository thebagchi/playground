#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>

class MyClass {
 public:
  MyClass(int val) : value(val) {
    std::cout << "Constructed with value: " << value << std::endl;
  }
  ~MyClass() { std::cout << "Destroyed" << std::endl; }
  void show() const { std::cout << "Value: " << value << std::endl; }

 private:
  int value;
};

template <typename T, typename... Args>
std::unique_ptr<T, std::function<void(T*)>> placement_unique(void* memory,
                                                             Args&&... args) {
  T* obj = new (memory) T(std::forward<Args>(args)...);
  auto deleter = [memory](T* ptr) {
    if (ptr) {
      ptr->~T();
      free(memory);
    }
  };
  return std::unique_ptr<T, std::function<void(T*)>>(obj, deleter);
}

int main() {
  void* memory = malloc(sizeof(MyClass));
  if (!memory) return 1;

  auto ptr = placement_unique<MyClass>(memory, 42);

  ptr->show();

  return 0;
}