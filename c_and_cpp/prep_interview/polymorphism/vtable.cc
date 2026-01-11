#include <iostream>

class Base {
public:
  Base() {
    // 此时 vptr 指向 Base 的 vtable
    foo(); // 调用 Base::foo，不是 Derived::foo！
  }
  virtual void foo() { std::cout << "Base\n"; }
};

class Derived : public Base {
public:
  Derived() {
    // 此时 vptr 已改为指向 Derived 的 vtable
    foo(); // 调用 Derived::foo
  }
  void foo() override { std::cout << "Derived\n"; }
};

int main() {
  Derived d;
  // 输出：
  // Base      ← 构造 Base 时
  // Derived   ← 构造 Derived 时

  std::cout << sizeof(Base) << std::endl;
  std::cout << sizeof(Derived) << std::endl;
}