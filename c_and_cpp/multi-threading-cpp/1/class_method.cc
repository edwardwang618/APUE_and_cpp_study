#include <functional>
#include <iostream>
#include <thread>

class X {
public:
  int value = 0;

  void increment() { value++; }
};

int main() {
  X my_x;

  std::cout << "Initial: " << my_x.value << std::endl;

  // Pointer - modifies original
  std::thread t1(&X::increment, &my_x);
  t1.join();
  std::cout << "After pointer: " << my_x.value << std::endl;

  // Reference wrapper - modifies original
  std::thread t2(&X::increment, std::ref(my_x));
  t2.join();
  std::cout << "After std::ref: " << my_x.value << std::endl;

  // Copy - modifies copy, NOT original!
  std::thread t3(&X::increment, my_x);
  t3.join();
  std::cout << "After copy: " << my_x.value << std::endl;

  return 0;
}