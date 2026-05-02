#include <cstdlib>
#include <iostream>
#include <new>

// === operator new ===
void *operator new(std::size_t size) {
  void *p = std::malloc(size);
  std::cout << "[new]       size=" << size << " -> " << p << "\n";
  return p;
}

void *operator new[](std::size_t size) {
  void *p = std::malloc(size);
  std::cout << "[new[]]     size=" << size << " -> " << p << "\n";
  return p;
}

// === operator delete ===
void operator delete(void *p) noexcept {
  std::cout << "[delete(void*)]        p=" << p << "\n";
  std::free(p);
}

void operator delete(void *p, std::size_t size) noexcept {
  std::cout << "[delete(void*, size)]  p=" << p << " size=" << size << "\n";
  std::free(p);
}

void operator delete[](void *p) noexcept {
  std::cout << "[delete[](void*)]      p=" << p << "\n";
  std::free(p);
}

void operator delete[](void *p, std::size_t size) noexcept {
  std::cout << "[delete[](void*, size)] p=" << p << " size=" << size << "\n";
  std::free(p);
}

// === 测试类 ===
class Widget {
public:
  int data[4]; // 16 bytes
  Widget() {}
  ~Widget() {}
};

int main() {
  std::cout << "sizeof(Widget) = " << sizeof(Widget) << "\n";
  std::cout << "sizeof(size_t) = " << sizeof(size_t) << "\n\n";

  std::cout << "=== 单对象 ===\n";
  Widget *p1 = new Widget();
  delete p1;

  std::cout << "\n=== 数组 [1] ===\n";
  Widget *p2 = new Widget[1];
  delete[] p2;

  std::cout << "\n=== 数组 [3] ===\n";
  Widget *p3 = new Widget[3];
  delete[] p3;

  std::cout << "\n=== 数组 [5] ===\n";
  Widget *p4 = new Widget[5];
  delete[] p4;

  std::cout << "\n=== 基本类型 int[10] ===\n";
  int *p5 = new int[10];
  delete[] p5;

  return 0;
}