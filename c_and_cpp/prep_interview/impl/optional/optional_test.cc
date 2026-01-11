#include <iostream>
#include <optional>

int main() {
  std::optional<int> op1{1}, op2{2};
  std::cout << op1.has_value() << " " << op2.has_value() << std::endl;
  op1 = std::move(op2);
  std::cout << op1.has_value() << " " << op2.has_value() << std::endl;
}