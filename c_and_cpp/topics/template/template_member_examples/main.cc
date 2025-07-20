#include "container.h"
#include <string>

int main() {
  // Demonstrate template member function
  Container<std::string> strContainer;
  strContainer.convert(123); // converts int to string
  std::cout << "String container after converting int: ";
  strContainer.print();

  Container<double> doubleContainer;
  doubleContainer.convert(42); // converts int to double
  std::cout << "Double container after converting int: ";
  doubleContainer.print();

  // Demonstrate static template variable
  Container<int>::default_value = 42;
  std::cout << "Default value for int container: "
            << Container<int>::default_value << std::endl;

  Container<std::string>::default_value = "default";
  std::cout << "Default value for string container: "
            << Container<std::string>::default_value << std::endl;

  return 0;
}
