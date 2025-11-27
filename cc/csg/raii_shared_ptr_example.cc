#include <iostream>
#include <memory>
#include <mutex>
#include <thread>

class Resource {
 public:
  Resource(int id) : id_(id), count_(0) {
    std::cout << "Resource " << id_ << " created" << std::endl;
  }
  ~Resource() {
    std::cout << "Resource " << id_ << " destroyed (used " << count_
              << " times)" << std::endl;
  }
  void use() {
    std::lock_guard<std::mutex> lock(mtx_);
    count_++;
    std::cout << "Using resource " << id_ << std::endl;
  }

 private:
  int id_, count_;
  std::mutex mtx_;
};

void worker(std::shared_ptr<Resource> res) { res->use(); }

int main() {
  std::cout << "RAII with shared_ptr and threads" << std::endl;

  auto res = std::make_shared<Resource>(1);
  std::cout << "use_count: " << res.use_count() << std::endl;

  // Lambda capturing shared_ptr
  auto lambda = [res]() {
    // use the resource
    res->use();
  };

  // Threads using shared resource
  std::thread t1(worker, res);
  std::thread t2(worker, res);

  lambda();  // Use in lambda

  t1.join();
  t2.join();

  std::cout << "use_count: " << res.use_count() << std::endl;
  return 0;
}