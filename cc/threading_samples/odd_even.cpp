#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

constexpr int MAX_NUMBER = 20;

std::mutex mtx;
std::condition_variable cv_even, cv_odd;
int current_number = 1;
bool is_even_turn = false;
bool is_odd_turn = false;

void emitter() {
    while (current_number <= MAX_NUMBER) {
        std::unique_lock<std::mutex> lock(mtx);
        if (current_number % 2 == 0) {
            is_even_turn = true;
            cv_even.notify_one();
        } else {
            is_odd_turn = true;
            cv_odd.notify_one();
        }
        // wait until the processor thread prints the number
        cv_even.wait(lock, [] { 
            // unlock if even processor is not active 
            return !is_even_turn; 
        });
        cv_odd.wait(lock, [] { 
            // unlock if odd processor is not active
            return !is_odd_turn; 
        });
    }
}

void even_processor() {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        cv_even.wait(lock, [] { 
            // unlock if even number is emitted ...
            return is_even_turn || current_number > MAX_NUMBER; 
        });

        if (current_number > MAX_NUMBER) {
            // reached max
            break;
        }

        std::cout << "Even Processor: " << current_number << std::endl;
        ++current_number;
        is_even_turn = false;
        // wake emitter
        cv_even.notify_one(); 
    }
}

void odd_processor() {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        cv_odd.wait(lock, [] { 
            // unlock if odd number is emitted ...
            return is_odd_turn || current_number > MAX_NUMBER; 
        });

        if (current_number > MAX_NUMBER) { 
            // reached max
            break;
        }

        std::cout << "Odd Processor: " << current_number << std::endl;
        ++current_number;
        is_odd_turn = false;
        // wake emitter
        cv_odd.notify_one(); 
    }
}

int main() {
    std::thread t1(emitter);
    std::thread t2(even_processor);
    std::thread t3(odd_processor);

    t1.join();
    t2.join();
    t3.join();

    return 0;
}