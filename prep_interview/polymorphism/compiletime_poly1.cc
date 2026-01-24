#include <iostream>

void print(int x) { std::cout << "int: " << x << "\n"; }
void print(double x) { std::cout << "double: " << x << "\n"; }
void print(const std::string &x) { std::cout << "string: " << x << "\n"; }

int main() {
  print(42);      // 调用 print(int)
  print(3.14);    // 调用 print(double)
  print("hello"); // 调用 print(const std::string&)
}