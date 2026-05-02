// static_cast_demo.cpp
// compile: g++ -std=c++17 -o static_cast_demo static_cast_demo.cpp
#include <cstdint>
#include <iostream>

enum class Color { Red, Green, Blue };

class Animal {
public:
  virtual ~Animal() = default;
  virtual void speak() { std::cout << "..." << std::endl; }
};

class Dog : public Animal {
public:
  void speak() override { std::cout << "Woof!" << std::endl; }
  void fetch() { std::cout << "Fetching stick!" << std::endl; }
};

int main() {
  // 1) 数值转换：double -> int，截断小数部分
  double pi = 3.14159;
  int n = static_cast<int>(pi); // n = 3
  std::cout << "pi truncated: " << n << std::endl;

  // 2) 枚举 <-> 整数
  //    scoped enum (enum class) 不允许隐式转换，必须显式 cast
  Color c = Color::Blue;
  int ci = static_cast<int>(c);     // ci = 2
  Color c2 = static_cast<Color>(1); // c2 = Color::Green
  std::cout << "Blue = " << ci << std::endl;

  // 3) void* -> typed pointer
  //    malloc 返回 void*，C++ 中需要 cast（C 中不需要）
  void *raw = new int(42);
  int *ip = static_cast<int *>(raw);
  std::cout << "value: " << *ip << std::endl;
  delete ip;

  // 4) 继承体系中的下行转换（downcast）
  //    编译器不生成任何运行时检查 —— 你必须自己保证类型正确
  //    NOTE: if base doesn't actually point to Dog, this is UB (undefined
  //    behavior)
  Animal *animal = new Dog();
  Dog *dog = static_cast<Dog *>(animal); // no runtime check
  dog->fetch();
  delete animal;

  return 0;
}