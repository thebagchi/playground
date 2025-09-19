#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>
#include <vector>

using Matrix = std::vector<std::vector<double>>;
using Section = std::tuple<std::uint64_t, std::uint64_t, std::uint64_t>;

void matrix_multiply_section(const Matrix &A, const Matrix &B, Matrix &C,
                             Section section) {
  // Not thinking about optimization ...
  // Purpose here is to load the thread ...
  auto [start, end, size] = section;
  for (auto i = start; i < end; i++) {
    for (auto j = 0; j < size; j++) {
      double temp = 0.0;
      for (auto k = 0; k < size; k++) {
        temp = temp + (A[i][k] * B[k][j]);
      }
      C[i][j] = temp;
    }
  }
}

Matrix make_matrix(std::uint64_t size, double value = 0.0) {
  return Matrix(size, std::vector<double>(size, value));
}

void parallel_matrix_multiply(std::uint64_t size, std::uint64_t num) {
  auto A = make_matrix(size, 1.1);
  auto B = make_matrix(size, 2.2);
  auto C = make_matrix(size);
  std::vector<std::thread> threads;
  std::uint64_t rows_per_thread = size / num;
  for (auto i = 0; i < num; ++i) {
    auto srow = i * rows_per_thread;
    auto erow = (i == num - 1) ? size : srow + rows_per_thread;
    threads.emplace_back(matrix_multiply_section, std::cref(A), std::cref(B),
                         std::ref(C), std::make_tuple(srow, erow, size));
  }
  for (auto &thread : threads) {
    thread.join();
  }
}

int main(int argc, char **argv) {
  std::uint64_t size = 256 * 4 * 2 * 1;
  std::vector<std::uint64_t> counts = {1, 2, 4, 8, 16, 32, 64, 128, 256};
  for (auto count : counts) {
    auto start_time = std::chrono::high_resolution_clock::now();
    parallel_matrix_multiply(size, count);
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_taken = end_time - start_time;
    std::cout << "Time taken using: " << std::setw(4) << count
              << " threads is: " << std::fixed << std::setprecision(4)
              << time_taken.count() << " seconds ..." << std::endl;
  }
  return 0;
}