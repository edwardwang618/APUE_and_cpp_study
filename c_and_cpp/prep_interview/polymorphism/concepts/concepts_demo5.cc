#include <concepts>
#include <iostream>
#include <string>
#include <vector>

// 定义：可以用 + 连接的类型
template <typename T>
concept Addable = requires(T a, T b) {
  { a + b } -> std::convertible_to<T>;
};

// 定义：有 size() 方法的类型
template <typename T>
concept HasSize = requires(T t) {
  { t.size() } -> std::convertible_to<std::size_t>;
};

// 定义：可以打印的类型
template <typename T>
concept Printable = requires(T t, std::ostream &os) {
  { os << t } -> std::same_as<std::ostream &>;
};

// 使用
Addable auto sum(Addable auto a, Addable auto b) { return a + b; }

void show_size(const HasSize auto &container) {
  std::cout << "size: " << container.size() << "\n";
}

void print(const Printable auto &x) { std::cout << x << "\n"; }

int main() {
  std::cout << sum(1, 2) << "\n";                               // 3
  std::cout << sum(1.5, 2.5) << "\n";                           // 4
  std::cout << sum(std::string("a"), std::string("b")) << "\n"; // ab

  show_size(std::vector<int>{1, 2, 3}); // size: 3
  show_size(std::string("hello"));      // size: 5

  print(42);
  print("hello");
  print(3.14);
}