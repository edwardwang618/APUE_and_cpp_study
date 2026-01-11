#include <iostream>
#include <string>

int main() {
  std::string s = "hello";

  // Just a reference - NO move happens
  std::string &&ref = std::move(s);
  std::cout << "After ref: s = '" << s << "'" << std::endl;

  // Actual move - move constructor called!
  std::string new_str = std::move(s);
  std::cout << "After move: s = '" << s << "'" << std::endl;
}