#include <chrono>
#include <cstdio>
#include <iostream>
#include <thread>

void print_str(int i, std::string const &s) {
  std::cout << "i is " << i << " str is " << s << std::endl;
}

// DANGEROUS: Use after free!
void danger_oops(int som_param) {
  char buffer[1024];
  sprintf(buffer, "%i", som_param);

  // buffer (char*) passed by pointer, not copied!
  // Conversion to std::string happens LATER in the new thread
  std::thread t(print_str, 3, buffer);
  t.detach();

  std::cout << "danger oops finished" << std::endl;
} // buffer destroyed here!

// SAFE: Explicit conversion before passing
void safe_oops(int som_param) {
  char buffer[1024];
  sprintf(buffer, "%i", som_param);

  // std::string(buffer) creates string NOW, before thread starts
  // String is copied into thread's storage
  std::thread t(print_str, 3, std::string(buffer));
  t.detach();

  std::cout << "safe oops finished" << std::endl;
}

int main() {
  std::cout << "=== Danger ===" << std::endl;
  danger_oops(42);

  std::this_thread::sleep_for(std::chrono::seconds(1));

  std::cout << "\n=== Safe ===" << std::endl;
  safe_oops(42);

  std::this_thread::sleep_for(std::chrono::seconds(1));

  return 0;
}