#include <thread>
#include <mutex>
#include <condition_variable>
#include <iostream>

class CountingSemaphore {
private:
    std::mutex mutex_;
    std::condition_variable cond_;
    int count_;

public:
    CountingSemaphore(int initial) : count_(initial) {
        // Do Nothing ...
    }

    void acquire() {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [this] { 
            // unlock if count_ > 0
            return count_ > 0; 
        });
        --count_;
    }

    void release() {
        std::unique_lock<std::mutex> lock(mutex_);
        ++count_;
        lock.unlock();
        cond_.notify_one();
    }

    void wait_for_count(int target) {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [this, target] { 
            // unlock if count_ >= target
            return count_ >= target; 
        });
    }

    int get_count() {
        std::unique_lock<std::mutex> lock(mutex_);
        return count_;
    }
};

struct Semaphore {
    CountingSemaphore sem{0};
};

void producer(Semaphore& context) {
    for (int i = 0; i < 3; ++i) {
        context.sem.release();
        std::cout << "Producer: Released permit, count = " << context.sem.get_count() << "\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void consumer(Semaphore& context) {
    context.sem.wait_for_count(2);
    std::cout << "Consumer: Semaphore count reached 2\n";
    for (int i = 0; i < 2; ++i) {
        context.sem.acquire();
    }
    std::cout << "Consumer: Consumed 2 permits\n";
}

int main() {
    Semaphore context;
    std::thread prod(producer, std::ref(context));
    std::thread cons(consumer, std::ref(context));

    prod.join();
    cons.join();
    return 0;
}