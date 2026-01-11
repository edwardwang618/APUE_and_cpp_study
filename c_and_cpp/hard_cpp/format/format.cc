#include <format>
#include <iostream>
#include <string>

int main() {
  // Basic usage
  std::string s = std::format("Hello, {}!", "world");
  std::cout << s << '\n';

  // Multiple arguments
  std::cout << std::format("{} + {} = {}\n", 1, 2, 3);

  // Positional arguments
  std::cout << std::format("{1} before {0}\n", "second", "first");

  // Number formatting
  int n = 42;
  std::cout << std::format("decimal: {}\n", n);
  std::cout << std::format("hex:     {:x}\n", n);
  std::cout << std::format("binary:  {:b}\n", n);
  std::cout << std::format("octal:   {:o}\n", n);

  // Width and alignment
  std::cout << std::format("|{:10}|\n",
                           "left"); // left align (default for strings)
  std::cout << std::format("|{:<10}|\n", 42);  // left align
  std::cout << std::format("|{:>10}|\n", 42);  // right align
  std::cout << std::format("|{:^10}|\n", 42);  // center align
  std::cout << std::format("|{:*^10}|\n", 42); // center with fill char

  // Floating point
  double pi = 3.14159265359;
  std::cout << std::format("default:   {}\n", pi);
  std::cout << std::format("fixed:     {:.2f}\n", pi);
  std::cout << std::format("scientific:{:.2e}\n", pi);

  // Leading zeros
  std::cout << std::format("{:05}\n", 42); // 00042

  // Sign
  std::cout << std::format("{:+}\n", 42);  // +42
  std::cout << std::format("{:+}\n", -42); // -42

  return 0;
}