#include <chrono>
#include <cmath>
#include <execution>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <vector>

// Simulate expensive computation
double heavy_computation(double x) {
  double result = x;
  for (int i = 0; i < 100; ++i) {
    result =
        std::sin(result) * std::cos(result) + std::sqrt(std::abs(result) + 1.0);
    result = std::log(result + 2.0) * std::exp(result * 0.01);
  }
  return result;
}

int main() {
  std::cout << "Hardware threads: " << std::thread::hardware_concurrency()
            << "\n\n";

  std::vector<size_t> sizes = {1000, 10000, 100000, 1000000};

  std::cout << std::setw(10) << "Size" << std::setw(15) << "seq (ms)"
            << std::setw(15) << "par (ms)" << std::setw(15) << "par_unseq (ms)"
            << std::setw(12) << "Speedup" << std::endl;
  std::cout << std::string(67, '-') << std::endl;

  for (auto size : sizes) {
    std::vector<double> data(size);
    std::iota(data.begin(), data.end(), 1.0);

    // Sequential
    auto start1 = std::chrono::high_resolution_clock::now();
    volatile double r1 =
        std::transform_reduce(std::execution::seq, data.begin(), data.end(),
                              0.0, std::plus<>{}, heavy_computation);
    auto end1 = std::chrono::high_resolution_clock::now();
    auto time_seq =
        std::chrono::duration<double, std::milli>(end1 - start1).count();

    // Parallel
    auto start2 = std::chrono::high_resolution_clock::now();
    volatile double r2 =
        std::transform_reduce(std::execution::par, data.begin(), data.end(),
                              0.0, std::plus<>{}, heavy_computation);
    auto end2 = std::chrono::high_resolution_clock::now();
    auto time_par =
        std::chrono::duration<double, std::milli>(end2 - start2).count();

    // Parallel + SIMD
    auto start3 = std::chrono::high_resolution_clock::now();
    volatile double r3 = std::transform_reduce(
        std::execution::par_unseq, data.begin(), data.end(), 0.0, std::plus<>{},
        heavy_computation);
    auto end3 = std::chrono::high_resolution_clock::now();
    auto time_par_unseq =
        std::chrono::duration<double, std::milli>(end3 - start3).count();

    std::cout << std::setw(10) << size << std::setw(15) << std::fixed
              << std::setprecision(2) << time_seq << std::setw(15) << std::fixed
              << std::setprecision(2) << time_par << std::setw(15) << std::fixed
              << std::setprecision(2) << time_par_unseq << std::setw(11)
              << std::fixed << std::setprecision(1) << (time_seq / time_par)
              << "x" << std::endl;

    (void)r1;
    (void)r2;
    (void)r3;
  }

  return 0;
}