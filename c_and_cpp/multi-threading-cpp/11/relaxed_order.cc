#include <atomic>
#include <iostream>
#include <thread>

std::atomic<bool> x, y;
std::atomic<int> z;

void write_x_then_y() {
  x.store(true, std::memory_order_relaxed);
  y.store(true, std::memory_order_relaxed);
}

void read_y_then_x() {
  while (!y.load(std::memory_order_relaxed)) {
  }
  if (x.load(std::memory_order_relaxed)) {
    ++z;
  }
}

int main() {
  int zero_count = 0;

  for (int i = 0; i < 1000000; ++i) {
    x = false;
    y = false;
    z = 0;

    std::thread t1(write_x_then_y);
    std::thread t2(read_y_then_x);

    t1.join();
    t2.join();

    if (z == 0) {
      ++zero_count;
    }
  }

  std::cout << "z=0 happened " << zero_count << " times" << std::endl;
}