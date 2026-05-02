// const_cast_demo.cpp
// compile: g++ -std=c++17 -o const_cast_demo const_cast_demo.cpp
#include <cstring>
#include <iostream>

// 模拟一个旧的 C 库函数：参数没有标 const，但实际上不修改 s
// simulating a legacy C API that forgot to add const
void legacy_print(char *s) {
  // 只读取，不修改
  while (*s) {
    std::putchar(*s);
    ++s;
  }
  std::putchar('\n');
}

// 一个合理的 const_cast 使用场景：
// 你的字符串是 const 的，但 legacy API 需要 char*
void safe_usage() {
  const char *msg = "Hello from const string";

  // const_cast 移除 const，传给旧 API
  // safe because legacy_print doesn't actually modify the data
  legacy_print(const_cast<char *>(msg));
}

// 一个危险的例子（undefined behavior）
void dangerous_usage() {
  const int x = 42;
  // x 本身是 const —— 编译器可能把它优化成立即数
  // the compiler may have replaced all reads of x with the literal 42

  int *p = const_cast<int *>(&x);
  // *p = 100;  // UB! 取消注释会导致未定义行为
  //            // 可能"成功"修改，可能崩溃，可能毫无效果
  //            // the value of x might still read as 42 due to compiler
  //            optimization

  std::cout << "x = " << x << std::endl; // 几乎肯定输出 42
  std::cout << "address of x: " << &x << std::endl;
  std::cout << "p points to:  " << p << std::endl;
}

// 另一个合法场景：在 const 成员函数中获取 non-const this
// 用于实现 const 和 non-const 版本共享逻辑
class TextBuffer {
  char *data_;
  size_t len_;

public:
  TextBuffer(const char *s) : len_(std::strlen(s)) {
    data_ = new char[len_ + 1];
    std::strcpy(data_, s);
  }
  ~TextBuffer() { delete[] data_; }

  // const 版本
  const char &operator[](size_t i) const {
    // 假设这里有边界检查等复杂逻辑
    return data_[i];
  }

  // non-const 版本：复用 const 版本的实现，避免代码重复
  // Scott Meyers 的经典技巧（Effective C++ Item 3）
  char &operator[](size_t i) {
    // add const to *this, call const version, then remove const from result
    return const_cast<char &>(static_cast<const TextBuffer &>(*this)[i]);
  }
};

int main() {
  safe_usage();
  dangerous_usage();

  TextBuffer buf("Hello");
  buf[0] = 'h'; // 调用 non-const operator[]
  std::cout << buf[0] << buf[1] << buf[2] << buf[3] << buf[4] << std::endl;

  return 0;
}