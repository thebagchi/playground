#include <thread>
#include <mutex>
#include <condition_variable>
#include <iostream>

class FizzBuzz {
private:
    int max_;
    int current_; 
    std::mutex mutex_;
    std::condition_variable cond_;

public:
    FizzBuzz(int n) : max_(n), current_(1) {
        // Do Nothing ...
    }

    void fizz() {
        while (true) {
            std::unique_lock<std::mutex> lock(mutex_);
            cond_.wait(lock, [this] { 
                return current_ > max_ || (current_ % 3 == 0 && current_ % 5 != 0); 
            });
            if (current_ > max_) {
                return;
            }
            std::cout << current_ << ": Fizz" << std::endl;
            ++current_;
            lock.unlock();
            cond_.notify_all();
        }
    }

    void buzz() {
        while (true) {
            std::unique_lock<std::mutex> lock(mutex_);
            cond_.wait(lock, [this] { 
                return current_ > max_ || (current_ % 5 == 0 && current_ % 3 != 0); 
            });
            if (current_ > max_) {
                return;
            }
            std::cout << current_ << ": Buzz" << std::endl;
            ++current_;
            lock.unlock();
            cond_.notify_all();
        }
    }

    void fizzbuzz() {
        while (true) {
            std::unique_lock<std::mutex> lock(mutex_);
            cond_.wait(lock, [this] { 
                return current_ > max_ || (current_ % 3 == 0 && current_ % 5 == 0); 
            });
            if (current_ > max_) {
                return;
            }
            std::cout << current_ << ": FizzBuzz" << std::endl;
            ++current_;
            lock.unlock();
            cond_.notify_all();
        }
    }

    void number() {
        while (true) {
            std::unique_lock<std::mutex> lock(mutex_);
            cond_.wait(lock, [this] { 
                return current_ > max_ || (current_ % 3 != 0 && current_ % 5 != 0); 
            });
            if (current_ > max_) {
                return;
            }
            std::cout << current_ << ": " << current_ << std::endl;
            ++current_;
            lock.unlock();
            cond_.notify_all();
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