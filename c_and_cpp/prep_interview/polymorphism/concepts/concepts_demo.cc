#include <concepts>
#include <iostream>
#include <vector>

template <typename T>
concept Printable = requires(T t) {
  { std::cout << t } -> std::same_as<std::ostream &>;
};

template <typename Container>
concept Iterable = requires(Container t) {
  { t.begin() } -> std::same_as<typename Container::iterator>;
  { t.end() } -> std::same_as<typename Container::iterator>;
};

template <typename Container>
  requires Iterable<Container> && Printable<typename Container::value_type>
void print_all(const Container &c) {
  for (const auto &item : c) {
    std::cout << item << " ";
  }
  std::cout << "\n";
}

int main() {
  std::vector<int> v{1, 2, 3};
  print_all(v); // OK: int 是 Printable

  struct Foo {};
  std::vector<Foo> f;
  // print_all(f);  // 错误：Foo 不是 Printable
}