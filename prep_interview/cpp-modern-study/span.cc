#include <cstdio>
#include <iostream>
#include <span>
#include <vector>

void modify(std::span<int> s) {
  s[0] = 42; // OK — mutating element
  for (int &x : s)
    x *= 2; // OK
}

void readOnly(std::span<const int> s) {
  // s[0] = 42;           // ERROR: assignment of read-only location
  int sum = 0;
  for (int x : s)
    sum += x; // OK — just reading
  std::cout << sum << '\n';
}

int main() {
  std::vector<int> a{1, 2, 3};
  for (int x : a)
    std::cout << x << std::endl;
  puts("=======================");
  modify(a);
  for (int x : a)
    std::cout << x << std::endl;
}