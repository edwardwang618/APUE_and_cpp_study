#include <atomic>
#include <iostream>
#include <thread>

volatile int x = 0;
volatile int y = 0;
volatile int r1 = 0;
volatile int r2 = 0;

void thread1() {
  x = 1;
  r1 = y; // Might read y BEFORE x=1 is visible!
}

void thread2() {
  y = 1;
  r2 = x; // Might read x BEFORE y=1 is visible!
}

int main() {
  int both_zero = 0;

  for (int i = 0; i < 100000; ++i) {
    x = 0;
    y = 0;
    r1 = 0;
    r2 = 0;

    std::thread t1(thread1);
    std::thread t2(thread2);
    t1.join();
    t2.join();

    // Logically impossible: if x=1 ran, r2 should see it (or vice versa)
    // But with reordering, BOTH can be 0!
    if (r1 == 0 && r2 == 0) {
      both_zero++;
    }
  }

  std::cout << "Both zero (should be impossible): " << both_zero << std::endl;

  return 0;
}