#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

constexpr int MAX_NUMBER = 20;

std::mutex mtx;
std::condition_variable cv_even, cv_odd;

std::mutex print_mtx;
std::condition_variable cv_print;
std::queue<int> print_queue;

int current_number = 1;
bool is_even_turn = false;
bool is_odd_turn = false;
bool done = false;

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
        cv_even.wait(lock, [] { 
            // even processor is not active ...
            return !is_even_turn; 
        });
        cv_odd.wait(lock, [] { 
            // odd processor is not active ...
            return !is_odd_turn; 
        });
    }

    // Signal completion
    {
        std::lock_guard<std::mutex> lock(print_mtx);
        done = true;
        cv_print.notify_one();
    }
}

void even_processor() {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        cv_even.wait(lock, [] { 
            // even number emitted ...
            return is_even_turn || current_number > MAX_NUMBER; 
        });

        if (current_number > MAX_NUMBER) {
            // reached max
             break;
        }

        {
            std::lock_guard<std::mutex> print_lock(print_mtx);
            print_queue.push(current_number);
            cv_print.notify_one();
        }

        ++current_number;
        is_even_turn = false;
        cv_even.notify_one();
    }
}

void odd_processor() {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        cv_odd.wait(lock, [] { 
            // odd number emitted ...
            return is_odd_turn || current_number > MAX_NUMBER; 
        });

        if (current_number > MAX_NUMBER) {
            // reached max ...
            break;
        }

        {
            std::lock_guard<std::mutex> print_lock(print_mtx);
            print_queue.push(current_number);
            cv_print.notify_one();
        }

        ++current_number;
        is_odd_turn = false;
        cv_odd.notify_one();
    }
}

void printer() {
    while (true) {
        std::unique_lock<std::mutex> lock(print_mtx);
        cv_print.wait(lock, [] { 
            // unlock if print queue is not empty
            return !print_queue.empty() || done; 
        });

        while (!print_queue.empty()) {
            int num = print_queue.front();
            print_queue.pop();
            std::cout << "Printer: " << num << std::endl;
        }

        if (done && print_queue.empty()) {
            // done and not more print jobs ... 
            break;
        }
    }
}

int main() {
    std::thread t1(emitter);
    std::thread t2(even_processor);
    std::thread t3(odd_processor);
    std::thread t4(printer);

    t1.join();
    t2.join();
    t3.join();
    t4.join();

    return 0;
}