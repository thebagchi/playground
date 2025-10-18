#include <atomic>
#include <thread>
#include <iostream>

class FizzBuzz {
private:
    int n;
    std::atomic<int> current{1};

public:
    FizzBuzz(int n) : n(n) {}

    void fizz() {
        while (true) {
            int curr = current.load(std::memory_order_acquire);
            if (curr > n) {
                break;
            }
            if ((curr < n) && (curr % 3 == 0 && curr % 5 != 0)) {
                std::cout << curr << ": Fizz" << std::endl;
                current.fetch_add(1, std::memory_order_release); 
            }
        }
    }

    void buzz() {
        while (true) {
            int curr = current.load(std::memory_order_acquire);
            if (curr > n) {
                break;
            }
            if ((curr < n) && (curr % 5 == 0 && curr % 3 != 0)) {
                std::cout << curr << ": Buzz" << std::endl;
                current.fetch_add(1, std::memory_order_release);
            }
        }
    }

    void fizzbuzz() {
        while (true) {
            int curr = current.load(std::memory_order_acquire);
            if (curr > n) {
                break;
            }
            if ((curr < n) && (curr % 3 == 0 && curr % 5 == 0)) {
                std::cout << curr << ": FizzBuzz" << std::endl;
                current.fetch_add(1, std::memory_order_release);
            }
        }
    }

    void number() {
        while (true) {
            int curr = current.load(std::memory_order_acquire);
            if (curr > n) {
                break;
            }
            if ((curr < n) && (curr % 3 != 0 && curr % 5 != 0)) {
                std::cout << curr << ": " << curr << std::endl;
                current.fetch_add(1, std::memory_order_release);
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