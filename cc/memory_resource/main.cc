#include <iostream>
#include <memory>
#include <vector>
#include <cstdint>
#include <chrono>
#include <iomanip>
#include <memory_resource>
#include <unistd.h>

void func1() {
  std::vector<std::unique_ptr<uint8_t>> vec1;
  for (int i = 0; i < 1000; ++i) {
    vec1.push_back(std::make_unique<uint8_t>(i % 256));
  }

  std::vector<std::unique_ptr<uint16_t>> vec2;
  for (int i = 0; i < 1000; ++i) {
    vec2.push_back(std::make_unique<uint16_t>(i));
  }

  std::vector<std::unique_ptr<uint32_t>> vec3;
  for (int i = 0; i < 1000; ++i) {
    vec3.push_back(std::make_unique<uint32_t>(i));
  }

  std::vector<std::unique_ptr<uint64_t>> vec4;
  for (int i = 0; i < 1000; ++i) {
    vec4.push_back(std::make_unique<uint64_t>(i));
  }
}

void run1() {
  auto start_time = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < 1000000; ++i) {
    func1();
  }
  auto end_time = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> time_taken = end_time - start_time;
  std::cout << "Ran func1() 1,000,000 times" << std::endl;
  std::cout << "Time taken is: " << std::fixed << std::setprecision(4) << time_taken.count()
            << " seconds ..." << std::endl;
}

void func2(std::pmr::monotonic_buffer_resource& resource) {
  // Allocate and store pointers to uint8_t using polymorphic allocator
  std::pmr::polymorphic_allocator<uint8_t> alloc1(&resource);
  std::vector<uint8_t*> vec1;
  for (int i = 0; i < 1000; ++i) {
    auto temp = alloc1.allocate(1);
    *temp = static_cast<uint8_t>(i % 256);
    vec1.push_back(temp);
  }

  // Allocate and store pointers to uint16_t using polymorphic allocator
  std::pmr::polymorphic_allocator<uint16_t> alloc2(&resource);
  std::vector<uint16_t*> vec2;
  for (int i = 0; i < 1000; ++i) {
    auto temp = alloc2.allocate(1);
    *temp = static_cast<uint16_t>(i);
    vec2.push_back(temp);
  }

  // Allocate and store pointers to uint32_t using polymorphic allocator
  std::pmr::polymorphic_allocator<uint32_t> alloc3(&resource);
  std::vector<uint32_t*> vec3;
  for (int i = 0; i < 1000; ++i) {
    auto temp = alloc3.allocate(1);
    *temp = static_cast<uint32_t>(i);
    vec3.push_back(temp);
  }

  // Allocate and store pointers to uint64_t using polymorphic allocator
  std::pmr::polymorphic_allocator<uint64_t> alloc4(&resource);
  std::vector<uint64_t*> vec4;
  for (int i = 0; i < 1000; ++i) {
    auto temp = alloc4.allocate(1);
    *temp = static_cast<uint64_t>(i);
    vec4.push_back(temp);
  }
  resource.release();
}

void run2() {
  auto page_size = sysconf(_SC_PAGESIZE);
  std::pmr::monotonic_buffer_resource resource(page_size);
  auto start_time = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < 1000000; ++i) {
    func2(resource);
  }
  auto end_time = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> time_taken = end_time - start_time;
  std::cout << "Ran func2() 1,000,000 times" << std::endl;
  std::cout << "Time taken is: " << std::fixed << std::setprecision(4) << time_taken.count()
            << " seconds ..." << std::endl;
}

int main(int argc, char* argv[]) {
  std::cout << "Hello World" << std::endl;
  run1();
  run2();
  return 0;
}
