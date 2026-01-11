#include <iostream>
#include <thread>

void change_param(int &param) {
  param++;
  std::cout << "Inside thread: " << param << std::endl;
}

// WITHOUT std::ref - doesn't work!
void ref_oops(int some_param) {
  std::cout << "before: " << some_param << std::endl;

  // This actually won't compile!
  // std::thread tries to pass copy to int& parameter
  // std::thread t2(change_param, some_param);  // ✗ Compile error!

  // If it did compile, it would modify a copy, not original
}

// WITH std::ref - works correctly!
void ref_works(int some_param) {
  std::cout << "before: " << some_param << std::endl;

  std::thread t2(change_param, std::ref(some_param));
  t2.join();

  std::cout << "after: " << some_param << std::endl;
}

int main() {
  // ref_oops(5);
  ref_works(5);
  return 0;
}