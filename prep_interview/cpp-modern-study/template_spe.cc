#include <print>

template <typename T> void Func(T arg) { std::println("Here {}.", arg); }

template <> void Func<int>(int arg) { std::println("There {}!", arg); }

int main() {
  Func("Hello");
  Func(1);
}