#include <iostream>

template <typename... Args> void print(Args... args) {
  ((std::cout << args << " "), ...);
  std::cout << "\n";
}

template <typename... Args> int sum(Args... args) {
  if constexpr ((sizeof...(Args)) == 0)
    return 0;
  else
    return (args + ...);
}

int main() {
  print(1, 2.5, "hello"); // 1 2.5 hello
  print(sum(1, 2, 3));
  print(sum());
  return 0;
}