#include <cstddef>
#include <cstdio>
#include <iostream>

template <size_t N> struct Factorial {
  static constexpr int value = Factorial<N - 1>::value * N;
};

template <> struct Factorial<0> {
  static constexpr int value = 1;
};

consteval int Factorial2(size_t N) {
  if (N == 1) {
    return 1;
  } else {
    return Factorial2(N - 1) * N;
  }
}

int main() {
  static_assert(Factorial<5>::value == 120);
  static_assert(Factorial2(5) == 120);
}