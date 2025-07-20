#include "traits_example.h"
#include <cstring>
#include <iostream>
#include <string>

int main() {
  // 1. Container traits usage
  std::cout << "=== Container Traits Example ===\n";
  int arr[5] = {1, 2, 3, 4, 5};
  int *ptr = &arr[0];

  std::cout << "Array traits:\n"
            << "Is array: " << std::boolalpha
            << ContainerTraits<decltype(arr)>::is_array << "\n"
            << "Is pointer: " << ContainerTraits<decltype(arr)>::is_pointer
            << "\n"
            << "Size: " << ContainerTraits<decltype(arr)>::size << "\n\n";

  std::cout << "Pointer traits:\n"
            << "Is array: " << ContainerTraits<decltype(ptr)>::is_array << "\n"
            << "Is pointer: " << ContainerTraits<decltype(ptr)>::is_pointer
            << "\n\n";

  // 2. Print traits usage
  std::cout << "=== Print Traits Example ===\n";
  int value = 42;
  int *valuePtr = &value;

  std::cout << "Regular value: ";
  PrintTraits<int>::print(value);
  std::cout << "\nPointer value: ";
  PrintTraits<int *>::print(valuePtr);
  std::cout << "\n\n";

  // 3. Policy traits usage
  std::cout << "=== Policy Traits Example ===\n";
  SortableContainer<char> a('A');
  SortableContainer<char> b('a');
  std::cout << "Default comparison (A < a): " << a.isLessThan(b) << "\n";

  SortableContainer<char, CaseInsensitiveCompare<char>> c('A');
  SortableContainer<char, CaseInsensitiveCompare<char>> d('a');
  std::cout << "Case insensitive comparison (A < a): " << c.isLessThan(d)
            << "\n\n";

  // 4. Type modification traits
  std::cout << "=== Type Modification Traits Example ===\n";
  using IntType = int;
  using ConstInt = MakeConst<IntType>::type;
  std::cout << "Is const: " << std::is_const<ConstInt>::value << "\n\n";

  // 5. SFINAE traits usage
  std::cout << "=== SFINAE Traits Example ===\n";
  try {
    std::cout << "10 / 2 = " << safe_divide(10, 2) << "\n";
    // This would cause a compilation error:
    // safe_divide("string", "error");
  } catch (const std::exception &e) {
    std::cout << "Error: " << e.what() << "\n";
  }

  // 6. Tag dispatch example
  std::cout << "\n=== Tag Dispatch Example ===\n";
  int pod_value;
  std::string non_pod_value;

  initialize(pod_value);     // Uses POD initialization
  initialize(non_pod_value); // Uses non-POD initialization

  std::cout << "POD value initialized: " << pod_value << "\n"
            << "Non-POD value initialized: " << non_pod_value << "\n";

  return 0;
}
