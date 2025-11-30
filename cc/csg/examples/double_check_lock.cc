#include <atomic>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>

class Resource {
public:
  Resource() {
    std::cout << "Resource created (expensive operation)" << std::endl;
  }
  void use() {
    // Use resource ...
    std::cout << "Using resource" << std::endl;
  }
};

std::atomic<bool> initialized{ false };
std::mutex mtx;
std::shared_ptr<Resource> resource;

std::shared_ptr<Resource> get_resource() {
  if (!initialized.load(std::memory_order_acquire)) {
    std::lock_guard<std::mutex> lock(mtx);
    if (!initialized.load(std::memory_order_relaxed)) {
      resource = std::make_shared<Resource>();
      initialized.store(true, std::memory_order_release);
    }
  }
  return resource;
}

void thread_func() {
  // Create and use resource ...
  get_resource()->use();
}

int main() {
  std::cout << "Double-checked locking example" << std::endl;

  // Multiple threads calling get_resource
  std::thread t1(thread_func);
  std::thread t2(thread_func);
  std::thread t3(thread_func);

  t1.join();
  t2.join();
  t3.join();

  std::cout << "All threads completed" << std::endl;
  return 0;
}