#include <iostream>
#include <memory>
#include <mutex>
#include <vector>

class Resource {
 public:
  Resource(int id) : id_(id) {
    std::cout << "Resource " << id_ << " created" << std::endl;
  }
  ~Resource() { std::cout << "Resource " << id_ << " destroyed" << std::endl; }
  void use() { std::cout << "Using resource " << id_ << std::endl; }

 private:
  int id_;
};

class ObjectPool {
 public:
  ObjectPool(size_t size) {
    for (size_t i = 0; i < size; ++i) {
      pool_.push_back(std::make_unique<Resource>(i));
    }
  }

  std::unique_ptr<Resource> acquire_unique() {
    std::lock_guard<std::mutex> lock(mtx_);
    if (!pool_.empty()) {
      auto res = std::move(pool_.back());
      pool_.pop_back();
      return res;
    }
    return nullptr;
  }

  std::shared_ptr<Resource> acquire_shared() {
    std::lock_guard<std::mutex> lock(mtx_);
    if (!pool_.empty()) {
      auto res = std::move(pool_.back());
      pool_.pop_back();
      return std::shared_ptr<Resource>(res.release(), [this](Resource* ptr) {
        std::lock_guard<std::mutex> lock(mtx_);
        pool_.push_back(std::unique_ptr<Resource>(ptr));
      });
    }
    return nullptr;
  }

  void release(std::unique_ptr<Resource> res) {
    std::lock_guard<std::mutex> lock(mtx_);
    pool_.push_back(std::move(res));
  }

 private:
  std::vector<std::unique_ptr<Resource>> pool_;
  std::mutex mtx_;
};

int main() {
  ObjectPool pool(3);

  // Acquire unique ownership
  auto res1 = pool.acquire_unique();
  if (res1) res1->use();

  // Acquire shared ownership
  auto res2 = pool.acquire_shared();
  if (res2) res2->use();

  // Another shared
  auto res3 = pool.acquire_shared();
  if (res3) res3->use();

  // Try to acquire when pool is empty
  auto res4 = pool.acquire_unique();
  if (!res4) std::cout << "Pool is empty" << std::endl;

  // Release one
  pool.release(std::move(res1));

  // Now acquire again
  auto res5 = pool.acquire_unique();
  if (res5) res5->use();

  return 0;
}