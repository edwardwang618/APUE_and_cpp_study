#include <atomic>
#include <iostream>
#include <linux/futex.h>
#include <sys/syscall.h>
#include <thread>
#include <unistd.h>

// The futex - just an int!
std::atomic<int> futex_var{0};

// Wrapper for futex syscall
long futex(std::atomic<int> *addr, int op, int val) {
  return syscall(SYS_futex, addr, op, val, nullptr, nullptr, 0);
}

void futex_lock(std::atomic<int> *f) {
  while (true) {
    // Try to acquire: 0 -> 1
    int expected = 0;
    if (f->compare_exchange_strong(expected, 1)) {
      return; // Got it! No syscall
    }

    // Failed - sleep until woken
    std::cout << "Thread " << std::this_thread::get_id() << " sleeping..."
              << std::endl;
    futex(f, FUTEX_WAIT, 1); // Sleep if value is still 1
    std::cout << "Thread " << std::this_thread::get_id() << " woke up!"
              << std::endl;
  }
}

void futex_unlock(std::atomic<int> *f) {
  f->store(0);             // Release lock
  futex(f, FUTEX_WAKE, 1); // Wake one waiter
}

int main() {
  // Thread 1 - holds lock for 2 seconds
  std::thread t1([]() {
    std::cout << "T1: locking..." << std::endl;
    futex_lock(&futex_var);
    std::cout << "T1: got lock!" << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(2));

    std::cout << "T1: unlocking..." << std::endl;
    futex_unlock(&futex_var);
  });

  // Small delay to ensure T1 gets lock first
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Thread 2 - will wait for T1
  std::thread t2([]() {
    std::cout << "T2: locking..." << std::endl;
    futex_lock(&futex_var);
    std::cout << "T2: got lock!" << std::endl;

    futex_unlock(&futex_var);
  });

  t1.join();
  t2.join();

  return 0;
}