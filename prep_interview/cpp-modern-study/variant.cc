#include <iostream>
#include <string>
#include <variant>

int main() {
  std::variant<int, double, std::string> v;

  v = 42;
  v = 3.14;
  v = "hello";

  std::visit([](auto &&x) { std::cout << x << '\n'; }, v); // prints: hello
}