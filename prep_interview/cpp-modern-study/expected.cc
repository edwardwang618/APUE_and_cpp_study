#include <expected>
#include <iostream>

enum class MathError { DivByZero, NegativeSqrt };

std::expected<double, MathError> safe_div(double a, double b) {
  if (b == 0)
    return std::unexpected(MathError::DivByZero);
  return a / b;
}

int main() {
  auto r = safe_div(10, 0);
  if (r) {
    std::cout << *r << '\n';
  } else {
    switch (r.error()) {
    case MathError::DivByZero:
      std::cout << "div by zero\n";
      break;
    case MathError::NegativeSqrt:
      std::cout << "neg sqrt\n";
      break;
    }
  }
}