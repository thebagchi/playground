#include <atomic>
#include <functional>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

using CallbackFunc = std::function<void(void)>;
std::vector<CallbackFunc> callbacks_;
bool event_fired_ = false;
std::mutex mutex_;

std::atomic_uint64_t register_count_(0);
std::atomic_uint64_t executed_count_(0);

void executeFunc(const CallbackFunc &func) {
  func();
  executed_count_.fetch_add(1, std::memory_order_relaxed);
}

void registerCallback(const CallbackFunc &func) {
  register_count_.fetch_add(1, std::memory_order_relaxed);
  std::vector<CallbackFunc> temp;
  do {
    if (event_fired_) {
      std::lock_guard lock(mutex_);
      temp.swap(callbacks_);
      temp.push_back(func);
      break;
    }
    {
      std::lock_guard lock(mutex_);
      if (event_fired_) {
        temp.swap(callbacks_);
        temp.push_back(func);
      } else {
        callbacks_.push_back(func);
      }
    }
  } while (0);
  for (const auto &func : temp) {
    executeFunc(func);
  }
}

void fireEvent() {
  std::vector<CallbackFunc> temp;
  do {
    if (event_fired_) {
      std::cout << "event already occured..." << std::endl;
      break;
    }
    {
      std::lock_guard lock(mutex_);
      do {
        if (event_fired_) {
          break;
        }
        event_fired_ = true;
        temp.swap(callbacks_);
      } while (0);
    }
  } while (0);
  for (const auto &func : temp) {
    executeFunc(func);
  }
}

int main(int argc, char **argv) {
  registerCallback([]() {
    std::cout << "callback#1 executed!" << std::endl;
    registerCallback([]() {
      std::cout << "callback#4 executed from callback#1!" << std::endl;
    });
  });

  registerCallback([]() {
    std::cout << "callback#2 executed!" << std::endl;
    registerCallback([]() {
      std::cout << "callback#5 executed from callback#2!" << std::endl;
    });
  });

  std::thread eventThread([]() {
    // fire the event ...
    fireEvent();
  });
  eventThread.join();

  registerCallback([]() {
    std::cout << "callback#3 executed!" << std::endl;
    registerCallback([]() {
      std::cout << "callback#6 executed from callback#3!" << std::endl;
    });
  });

  for (auto i = 0; i < 1000 * 1000; i++) {
    registerCallback([i]() {
      std::cout << "callback#" << i << " executed!" << std::endl;
      registerCallback([i]() {
        std::cout << "inner callback#" << i << " executed!" << std::endl;
      });
    });
  }
  std::cout << "register_count_: "
            << register_count_.load(std::memory_order_acquire) << std::endl;
  std::cout << "executed_count_: "
            << executed_count_.load(std::memory_order_acquire) << std::endl;
  return 0;
}