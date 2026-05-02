// reinterpret_cast_demo.cpp
// compile: g++ -std=c++17 -o reinterpret_cast_demo reinterpret_cast_demo.cpp
#include <cstdint>
#include <cstring>
#include <iostream>

// 一个网络协议的包头（packed struct）
// simulating a binary protocol header
#pragma pack(push, 1) // disable padding
struct PacketHeader {
  uint8_t version;
  uint8_t type;
  uint16_t length;
  uint32_t sequence;
};
#pragma pack(pop)

void demo_pointer_to_integer() {
  int x = 42;
  int *p = &x;

  // 指针 -> 整数：获取内存地址的数值
  // pointer -> integer: get the raw address as a number
  uintptr_t addr = reinterpret_cast<uintptr_t>(p);
  std::cout << "pointer value: " << p << std::endl;
  std::cout << "as integer:    0x" << std::hex << addr << std::dec << std::endl;

  // 整数 -> 指针：还原回来
  int *p2 = reinterpret_cast<int *>(addr);
  std::cout << "restored value: " << *p2 << std::endl;
}

void demo_binary_protocol() {
  // 模拟从网络收到一段原始字节
  // simulating raw bytes received from network
  uint8_t raw_data[] = {
      0x01,       // version = 1
      0x03,       // type = 3
      0x00, 0x20, // length = 32 (big-endian, but we ignore byte order here)
      0x00, 0x00, 0x00, 0x0A // sequence = 10
  };

  // reinterpret raw bytes as a PacketHeader struct
  // 把原始字节重解释为结构体
  // WARNING: this assumes matching endianness and alignment
  PacketHeader *header = reinterpret_cast<PacketHeader *>(raw_data);

  std::cout << "version:  " << (int)header->version << std::endl;
  std::cout << "type:     " << (int)header->type << std::endl;
  std::cout << "length:   " << header->length << std::endl;
  std::cout << "sequence: " << header->sequence << std::endl;
}

void demo_byte_inspection() {
  float f = 3.14f;

  // 想看 float 的内存表示（IEEE 754 bit pattern）
  // 方法 1：reinterpret_cast（技术上违反 strict aliasing rule，UB）
  // Method 1: reinterpret_cast (technically violates strict aliasing, UB)
  uint32_t bits_unsafe = *reinterpret_cast<uint32_t *>(&f);

  // 方法 2：memcpy（安全，defined behavior，编译器会优化成和上面一样的代码）
  // Method 2: memcpy (safe, defined behavior, compiler optimizes to same code)
  uint32_t bits_safe;
  std::memcpy(&bits_safe, &f, sizeof(float));

  std::cout << "float 3.14f bit pattern:" << std::endl;
  std::cout << "  reinterpret_cast: 0x" << std::hex << bits_unsafe << std::endl;
  std::cout << "  memcpy (safe):    0x" << bits_safe << std::dec << std::endl;
  // 两者结果相同：0x4048f5c3
}

int main() {
  std::cout << "=== Pointer <-> Integer ===" << std::endl;
  demo_pointer_to_integer();

  std::cout << "\n=== Binary Protocol ===" << std::endl;
  demo_binary_protocol();

  std::cout << "\n=== Float Bit Inspection ===" << std::endl;
  demo_byte_inspection();

  return 0;
}