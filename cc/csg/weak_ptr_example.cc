#include <iostream>
#include <memory>

class B;  // Forward declaration

class A {
 public:
  void SetPartner(const std::shared_ptr<B>& partner) {
    // B is partner of A
    partner_ = partner;
  }

  void ShowPartner() const {
    if (auto sp = partner_.lock()) {
      std::cout << "Partner B exists." << std::endl;
    } else {
      std::cout << "Partner B no longer exists." << std::endl;
    }
  }

 private:
  std::weak_ptr<B> partner_;
};

class B {
 public:
  void SetPartner(const std::shared_ptr<A>& partner) {
    // A is partner of B
    partner_ = partner;
  }

  void ShowPartner() const {
    if (auto sp = partner_.lock()) {
      std::cout << "Partner A exists." << std::endl;
    } else {
      std::cout << "Partner A no longer exists." << std::endl;
    }
  }

 private:
  std::weak_ptr<A> partner_;
};

int main() {
  auto a = std::make_shared<A>();
  auto b = std::make_shared<B>();

  a->SetPartner(b);
  b->SetPartner(a);

  a->ShowPartner();
  b->ShowPartner();

  b.reset();  // Destroy one object

  a->ShowPartner();  // Safe check using weak_ptr
}