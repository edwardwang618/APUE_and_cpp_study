#include <atomic>
#include <cassert>
#include <iostream>
#include <thread>

std::atomic<bool> x{false}, y{false};
std::atomic<int> z{0};

void write_x() {
  x.store(true, std::memory_order_seq_cst); // A
}

void write_y() {
  y.store(true, std::memory_order_seq_cst); // B
}

void read_x_then_y() {
  while (!x.load(std::memory_order_seq_cst)) {
  } // C
  if (y.load(std::memory_order_seq_cst))
    ++z; // D
}

void read_y_then_x() {
  while (!y.load(std::memory_order_seq_cst)) {
  } // E
  if (x.load(std::memory_order_seq_cst))
    ++z; // F
}

int main() {
  std::thread t1(write_x);
  std::thread t2(write_y);
  std::thread t3(read_x_then_y);
  std::thread t4(read_y_then_x);

  t1.join();
  t2.join();
  t3.join();
  t4.join();

  assert(z != 0); // ALWAYS passes with seq_cst
}