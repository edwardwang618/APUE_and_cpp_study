#include <absl/container/flat_hash_set.h>
#include <iostream>
#include <string>

int main() {
  // Create a set
  absl::flat_hash_set<std::string> fruits;

  // Insert elements
  fruits.insert("apple");
  fruits.insert("banana");
  fruits.insert("orange");
  fruits.emplace("mango");

  // Check if element exists
  if (fruits.contains("apple")) {
    std::cout << "Found apple!" << std::endl;
  }

  // Size
  std::cout << "Size: " << fruits.size() << std::endl;

  // Iterate
  for (const auto &fruit : fruits) {
    std::cout << fruit << std::endl;
  }

  // Erase
  fruits.erase("banana");

  // Find (returns iterator)
  auto it = fruits.find("orange");
  if (it != fruits.end()) {
    std::cout << "Found: " << *it << std::endl;
  }

  // Clear
  fruits.clear();

  // Initialize with values
  absl::flat_hash_set<int> numbers = {1, 2, 3, 4, 5};

  return 0;
}