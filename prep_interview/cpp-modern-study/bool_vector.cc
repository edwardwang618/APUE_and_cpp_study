#include <iostream>
#include <vector>

int main() {
  std::vector<bool> v{true, false, true};
  for (auto x : v)
    std::cout << x << std::endl;
  auto a = v[0];
  a = !a;
  for (auto x : v)
    std::cout << x << std::endl;
}