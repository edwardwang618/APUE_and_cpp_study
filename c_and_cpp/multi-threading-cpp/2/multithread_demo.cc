#include <chrono>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <thread>
#include <vector>

template <typename Iterator, typename T>
T parallel_accumulate(Iterator first, Iterator last, T init) {
  unsigned long const length = std::distance(first, last);

  if (!length)
    return init;

  unsigned long const min_per_thread = 25;
  unsigned long const max_threads =
      (length + min_per_thread - 1) / min_per_thread;

  unsigned long const hardware_threads = std::thread::hardware_concurrency();

  unsigned long const num_threads =
      std::min(hardware_threads != 0 ? hardware_threads : 2, max_threads);

  unsigned long const block_size = length / num_threads;

  std::vector<T> results(num_threads);
  std::vector<std::thread> threads(num_threads - 1);

  Iterator block_start = first;

  for (unsigned long i = 0; i < (num_threads - 1); ++i) {
    Iterator block_end = block_start;
    std::advance(block_end, block_size);

    threads[i] = std::thread([block_start, block_end, &results, i]() {
      results[i] = std::accumulate(block_start, block_end, T{});
    });

    block_start = block_end;
  }

  results[num_threads - 1] = std::accumulate(block_start, last, T{});

  for (auto &t : threads) {
    t.join();
  }

  return std::accumulate(results.begin(), results.end(), init);
}

int main() {
  std::cout << "Hardware threads: " << std::thread::hardware_concurrency()
            << "\n\n";

  std::vector<size_t> sizes = {1000,    10000,    100000,
                               1000000, 10000000, 100000000};

  std::cout << std::setw(15) << "Size" << std::setw(15) << "Single (ms)"
            << std::setw(15) << "Parallel (ms)" << std::setw(15) << "Speedup"
            << std::endl;
  std::cout << std::string(60, '-') << std::endl;

  for (auto size : sizes) {
    std::vector<long long> data(size);
    std::iota(data.begin(), data.end(), 1LL);

    // Single thread
    auto start1 = std::chrono::high_resolution_clock::now();
    volatile long long result1 = std::accumulate(data.begin(), data.end(), 0LL);
    auto end1 = std::chrono::high_resolution_clock::now();
    auto time1 =
        std::chrono::duration<double, std::milli>(end1 - start1).count();

    // Multi thread
    auto start2 = std::chrono::high_resolution_clock::now();
    volatile long long result2 =
        parallel_accumulate(data.begin(), data.end(), 0LL);
    auto end2 = std::chrono::high_resolution_clock::now();
    auto time2 =
        std::chrono::duration<double, std::milli>(end2 - start2).count();

    double speedup = time1 / time2;

    std::cout << std::setw(15) << size << std::setw(15) << std::fixed
              << std::setprecision(3) << time1 << std::setw(15) << std::fixed
              << std::setprecision(3) << time2 << std::setw(14) << std::fixed
              << std::setprecision(2) << speedup << "x" << std::endl;

    (void)result1;
    (void)result2;
  }

  return 0;
}