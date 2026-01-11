#include <chrono>
#include <iostream>
#include <thread>

struct BadData {
  int counter1;
  int counter2;
};

struct GoodData {
  alignas(64) int counter1; // Own cache line
  alignas(64) int counter2; // Own cache line
};

void test_bad() {
  BadData data{0, 0};

  auto start = std::chrono::high_resolution_clock::now();

  std::thread t1([&]() {
    for (int i = 0; i < 200000000; ++i)
      data.counter1++;
  });

  std::thread t2([&]() {
    for (int i = 0; i < 200000000; ++i)
      data.counter2++;
  });

  t1.join();
  t2.join();

  auto end = std::chrono::high_resolution_clock::now();
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

  std::cout << "Bad (false sharing): " << ms.count() << " ms\n";
}

void test_good() {
  GoodData data{0, 0};

  auto start = std::chrono::high_resolution_clock::now();

  std::thread t1([&]() {
    for (int i = 0; i < 100000000; ++i)
      data.counter1++;
  });

  std::thread t2([&]() {
    for (int i = 0; i < 100000000; ++i)
      data.counter2++;
  });

  t1.join();
  t2.join();

  auto end = std::chrono::high_resolution_clock::now();
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

  std::cout << "Good (no false sharing): " << ms.count() << " ms\n";
}

int main() {
  test_bad();
  test_good();
  return 0;
}