#include <concepts>
#include <iostream>
#include <string>

// 没有 Concepts：要写一堆重载
void print_v1(int x) { std::cout << "int: " << x << "\n"; }
void print_v1(long x) { std::cout << "long: " << x << "\n"; }
void print_v1(short x) { std::cout << "short: " << x << "\n"; }
void print_v1(unsigned int x) { std::cout << "uint: " << x << "\n"; }
void print_v1(long long x) { std::cout << "llong: " << x << "\n"; }
// ... 还有 unsigned long, unsigned short, char, unsigned char...

// 用 Concepts：一个函数处理所有整数
void print_v2(std::integral auto x) { std::cout << "integer: " << x << "\n"; }

void print_v2(std::floating_point auto x) {
  std::cout << "float: " << x << "\n";
}

int main() {
  print_v2(42);        // integer: 42
  print_v2(42L);       // integer: 42
  print_v2(42LL);      // integer: 42
  print_v2((short)42); // integer: 42
  print_v2(42U);       // integer: 42
  print_v2(3.14);      // float: 3.14
  print_v2(3.14f);     // float: 3.14
}