#include <iostream>
#include <unistd.h>
#include <new>

int main() {
  long size = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
  std::cout << "Cache line size: " << size << " bytes\n";
  std::cout << "Cache line size: "
            << std::hardware_destructive_interference_size << " bytes\n";
  return 0;
}