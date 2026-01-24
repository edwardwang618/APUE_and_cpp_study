#include <iostream>
#include <list>
#include <vector>

// 没有 Concepts
template <typename Container> void print_all_v1(const Container &c) {
  for (const auto &item : c) {
    std::cout << item << " ";
  }
  std::cout << "\n";
}

// 有 Concepts
template <typename T>
concept Iterable = requires(T t) {
  t.begin();
  t.end();
};

void print_all_v2(const Iterable auto &c) {
  for (const auto &item : c) {
    std::cout << item << " ";
  }
  std::cout << "\n";
}

int main() {
  std::vector<int> v{1, 2, 3};
  print_all_v1(v); // OK
  print_all_v2(v); // OK

  // print_all_v1(42);  // 错误信息很长很乱
  // print_all_v2(42);  // 错误信息清晰：int 不满足 Iterable
}