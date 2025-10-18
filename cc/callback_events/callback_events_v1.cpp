#include <atomic>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

using CallbackFunc = std::function<void(void)>;

std::atomic_bool event_occured_(false);
std::queue<CallbackFunc> callback_funcs_ = {};
std::mutex callback_mutex_;
std::atomic_bool executing_callbacks_(false);

std::atomic_uint64_t register_count_(0);
std::atomic_uint64_t executed_count_(0);

void execFunc(const CallbackFunc &func) {
  func();
  executed_count_.fetch_add(1, std::memory_order_relaxed);
}

void executeCallbacks() {
  if (executing_callbacks_.exchange(true, std::memory_order_acquire)) {
    return;
  }

  while (true) {
    CallbackFunc func;
    {
      std::lock_guard<std::mutex> lock(callback_mutex_);
      if (callback_funcs_.empty()) {
        executing_callbacks_.store(false, std::memory_order_release);
        return;
      }
      func = callback_funcs_.front();
      callback_funcs_.pop();
    }
    execFunc(func);
  }
}

void registerCallback(const CallbackFunc &func) {
  register_count_.fetch_add(1, std::memory_order_relaxed);
  {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    callback_funcs_.push(func);
  }

  if (event_occured_.load(std::memory_order_acquire)) {
    executeCallbacks();
  }
}

void fireEvent() {
  auto expected = false;
  if (event_occured_.compare_exchange_strong(expected, true,
                                             std::memory_order_acq_rel)) {
    executeCallbacks();
  } else {
    std::cout << "event has already occurred" << std::endl;
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

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  registerCallback([]() {
    std::cout << "callback#3 executed!" << std::endl;
    registerCallback([]() {
      std::cout << "callback#6 executed from callback#3!" << std::endl;
    });
  });

  for (auto i = 0; i < 1000 * 1000; i++) {
    registerCallback([i]() {
      std::cout << "callback#" << i << " executed!" << std::endl;
      registerCallback(
          [i]() { std::cout << "inner callback#" << i << std::endl; });
    });
  }

  std::cout << "register_count_: "
            << register_count_.load(std::memory_order_acquire) << std::endl;
  std::cout << "executed_count_: "
            << executed_count_.load(std::memory_order_acquire) << std::endl;
  return 0;
}