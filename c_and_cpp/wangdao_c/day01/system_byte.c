#include <iostream>

int main() {
  std::cout << "sizeof(void*): " << sizeof(void *) << " bytes" << std::endl;
  std::cout << "System is " << sizeof(void *) * 8 << "-bit" << std::endl;
  return 0;
}