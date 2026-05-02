// bit_cast_demo.cpp
// compile: g++ -std=c++20 -o bit_cast_demo bit_cast_demo.cpp
#include <array>
#include <bit> // std::bit_cast
#include <cstdint>
#include <cstring>
#include <iostream>

int main() {
  // 1) 查看 float 的 IEEE 754 位模式
  //    the safe, modern, defined-behavior way to type-pun
  float f = 3.14f;
  uint32_t bits = std::bit_cast<uint32_t>(f);
  std::cout << "3.14f = 0x" << std::hex << bits << std::dec << std::endl;
  // 输出：0x4048f5c3

  // 反过来：从位模式还原 float
  float f2 = std::bit_cast<float>(uint32_t(0x40490fdb));
  std::cout << "0x40490fdb = " << f2 << std::endl;
  // 输出：3.14159 (approximately pi)

  // 2) double 的位模式
  double d = 1.0;
  uint64_t dbits = std::bit_cast<uint64_t>(d);
  std::cout << "1.0 = 0x" << std::hex << dbits << std::dec << std::endl;
  // 输出：0x3ff0000000000000

  // 3) 对比旧方法（memcpy）—— 功能相同，但 bit_cast 更简洁且 constexpr
  double d2 = 2.0;
  uint64_t dbits2;
  std::memcpy(&dbits2, &d2, sizeof(d2)); // old way, also safe
  std::cout << "2.0 = 0x" << std::hex << dbits2 << std::dec << std::endl;

  // 4) 编译期使用（constexpr）
  //    this is impossible with memcpy or reinterpret_cast
  constexpr float pi_f = 3.14159265f;
  constexpr uint32_t pi_bits = std::bit_cast<uint32_t>(pi_f);
  static_assert(pi_bits == 0x40490fdb, "unexpected bit pattern");
  std::cout << "constexpr bit_cast works!" << std::endl;

  // 5) 大小不匹配 -> 编译错误（这是安全性的体现）
  // uint16_t small = std::bit_cast<uint16_t>(f);  // ERROR: sizeof mismatch
  // 编译器会告诉你 sizeof(uint16_t) != sizeof(float)

  return 0;
}