#include <iostream>
#include <mutex>
#include <thread>

class FizzBuzz {
private:
  int max_;
  int current_; 
  std::mutex mutex_;

public:
  FizzBuzz(int n) : max_(n), current_(1) {
    // Do Nothing ...
  }

  void fizz() {
    while (true) {
      {
        std::lock_guard<std::mutex> lock(mutex_);
        if (current_ > max_) {
          break;
        }
        if (current_ % 3 == 0 && current_ % 5 != 0) {
          std::cout << current_ << ": Fizz" << std::endl;
          ++current_;
        }
      } 
    }
  }

  void buzz() {
    while (true) {
      {
        std::lock_guard<std::mutex> lock(mutex_);
        if (current_ > max_) {
          break;
        }
        if (current_ % 5 == 0 && current_ % 3 != 0) {
          std::cout << current_ << ": Buzz" << std::endl;
          ++current_;
        }
      }
    }
  }

  void fizzbuzz() {
    while (true) {
      {
        std::lock_guard<std::mutex> lock(mutex_);
        if (current_ > max_) {
          break;
        }
        if (current_ % 3 == 0 && current_ % 5 == 0) {
          std::cout << current_ << ": FizzBuzz" << std::endl;
          ++current_;
        }
      }
    }
  }

  void number() {
    while (true) {
      {
        std::lock_guard<std::mutex> lock(mutex_);
        if (current_ > max_) {
          break;
        }
        if (current_ % 3 != 0 && current_ % 5 != 0) {
          std::cout << current_ << ": " << current_ << std::endl;
          ++current_;
        }
      }
    }
  }
};

int main() {
  int n = 15;
  FizzBuzz fb(n);

  std::thread t1(&FizzBuzz::fizz, &fb);
  std::thread t2(&FizzBuzz::buzz, &fb);
  std::thread t3(&FizzBuzz::fizzbuzz, &fb);
  std::thread t4(&FizzBuzz::number, &fb);

  t1.join();
  t2.join();
  t3.join();
  t4.join();

  return 0;
}