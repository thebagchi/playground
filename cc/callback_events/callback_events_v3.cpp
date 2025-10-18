#include <atomic>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

using CallbackFunc = std::function<void(void)>;

std::atomic_bool event_occured_(false);
std::queue<CallbackFunc> callback_funcs_;

std::mutex callback_mutex_;
std::atomic_bool executing_callbacks_(false);

std::atomic_uint64_t register_count_(0);
std::atomic_uint64_t executed_count_(0);

// Wrapper to execute a callback and increment executed_count_
void executeCallbackFunc(const CallbackFunc &func) {
  func();
  executed_count_.fetch_add(1, std::memory_order_relaxed);
}

// Unified function for registering and executing callbacks
void registerOrExecuteCallback(const CallbackFunc &func) {
  register_count_.fetch_add(1, std::memory_order_relaxed);
  if (event_occured_.load(std::memory_order_acquire)) {
    // Event has occurred, execute immediately
    executeCallbackFunc(func);
  } else {
    // Event not yet occurred, queue for later
    std::lock_guard<std::mutex> lock(callback_mutex_);
    callback_funcs_.push(func);
  }
}

void fireEvent() {
  auto expected = false;
  if (event_occured_.compare_exchange_strong(expected, true,
                                             std::memory_order_acq_rel)) {
    // Execute all queued callbacks
    std::queue<CallbackFunc> local_callbacks;
    {
      std::lock_guard<std::mutex> lock(callback_mutex_);
      std::swap(callback_funcs_, local_callbacks);
    }
    while (!local_callbacks.empty()) {
      CallbackFunc func = std::move(local_callbacks.front());
      local_callbacks.pop();
      executeCallbackFunc(func);
    }
  } else {
    std::cout << "event has already occurred" << std::endl;
  }
}

int main(int argc, char **argv) {
  registerOrExecuteCallback([]() {
    std::cout << "callback#1 executed!" << std::endl;
    registerOrExecuteCallback([]() {
      std::cout << "callback#4 executed from callback#1!" << std::endl;
    });
  });

  registerOrExecuteCallback([]() {
    std::cout << "callback#2 executed!" << std::endl;
    registerOrExecuteCallback([]() {
      std::cout << "callback#5 executed from callback#2!" << std::endl;
    });
  });

  std::thread eventThread([]() {
    // fire the event ...
    fireEvent();
  });
  eventThread.join();

  registerOrExecuteCallback([]() {
    std::cout << "callback#3 executed!" << std::endl;
    registerOrExecuteCallback([]() {
      std::cout << "callback#6 executed from callback#3!" << std::endl;
    });
  });

  for (auto i = 0; i < 1000 * 1000; i++) {
    registerOrExecuteCallback([i]() {
      std::cout << "callback#" << i << " executed!" << std::endl;
      registerOrExecuteCallback([i]() {
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
