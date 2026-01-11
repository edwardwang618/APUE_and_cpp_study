// test_jthread.cpp
#include <iostream>
#include <version>

int main() {
  std::cout << "__cplusplus: " << __cplusplus << std::endl;

#ifdef __cpp_lib_jthread
  std::cout << "jthread supported!" << std::endl;
#else
  std::cout << "jthread NOT supported" << std::endl;
#endif

  return 0;
}