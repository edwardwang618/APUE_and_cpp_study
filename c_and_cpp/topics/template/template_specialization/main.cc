#include "container.h"
#include <iostream>

int main() {
  // 1. Basic template usage
  Container<int> intContainer(42);
  intContainer.print();

  // 2. Full specialization for string
  Container<std::string> strContainer("Hello");
  strContainer.print();
  std::cout << "String length: " << strContainer.length() << std::endl;

  // 3. Partial specialization for pointers
  int num = 100;
  Container<int *> ptrContainer(&num);
  ptrContainer.print();
  std::cout << "Is null? " << std::boolalpha << ptrContainer.isNull()
            << std::endl;

  Container<int *> nullContainer(nullptr);
  nullContainer.print();
  std::cout << "Is null? " << nullContainer.isNull() << std::endl;

  // 4. Basic function template
  std::cout << "\nFunction template examples:" << std::endl;
  std::cout << "max(10, 20): " << max_value(10, 20) << std::endl;

  // 5. Specialized function template for const char*
  const char *str1 = "apple";
  const char *str2 = "banana";
  std::cout << "max(\"apple\", \"banana\"): " << max_value(str1, str2)
            << std::endl;

  // 6. Array specialization
  int arr[] = {1, 2, 3, 4, 5};
  Container<int[5]> arrContainer(arr);
  arrContainer.print();

  return 0;
}
