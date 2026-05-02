#include <bit>
#include <iostream>

int main() {
  std::cout << (std::endian::native == std::endian::little) << std::endl;
}