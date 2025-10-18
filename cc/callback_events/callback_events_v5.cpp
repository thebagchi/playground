#include <atomic>
#include <functional>
#include <iostream>
#include <mutex>
#include <vector>
#include <thread>

std::atomic_int executing_callbacks_{0};
std::atomic_bool event_fired_{false};

using CallbackFunc = std::function<void(void)>;
std::vector<CallbackFunc> callback_queue_;
std::mutex callback_mutex_;

std::atomic_uint64_t register_count_(0);
std::atomic_uint64_t executed_count_(0);

void executeFunc(const CallbackFunc &func) {
  func();
  executed_count_.fetch_add(1, std::memory_order_relaxed);
}

void executeCallbacks() {
  do {
    int prev = executing_callbacks_.fetch_add(1, std::memory_order_acq_rel);
    if (prev != 0) {
      break;
    }
    while (executing_callbacks_.load(std::memory_order_acquire) != 0) {
      std::vector<CallbackFunc> local_callbacks;
      {
        std::lock_guard<std::mutex> lock(callback_mutex_);
        local_callbacks.swap(callback_queue_);
      }
      for (auto &cb : local_callbacks) {
        executeFunc(cb);
      }
      executing_callbacks_.fetch_sub(1, std::memory_order_acq_rel);
    }
  } while (0);
}

void registerCallback(const CallbackFunc &func) {
  register_count_.fetch_add(1, std::memory_order_relaxed);
  {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    callback_queue_.push_back(func);
  }
  if (event_fired_.load(std::memory_order_acquire)) {
    executeCallbacks();
  }
}

void fireEvent() {
  bool expected = false;
  if (event_fired_.compare_exchange_strong(expected, true,
                                           std::memory_order_acq_rel)) {
    executeCallbacks();
  } else {
    std::cout << "event already occured..." << std::endl;
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