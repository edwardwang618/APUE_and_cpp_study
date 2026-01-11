#include <iostream>
#include <new>
#include <vector>

// Global cache that can be freed in emergency
std::vector<char> *emergencyMemory = nullptr;

void outOfMemoryHandler() {
  std::cerr << "Memory allocation failed!\n";

  if (emergencyMemory) {
    std::cerr << "Freeing emergency reserve...\n";
    delete emergencyMemory;
    emergencyMemory = nullptr;
    return; // Retry allocation
  }

  // No more memory to free, give up
  std::cerr << "No recovery possible, throwing...\n";
  throw std::bad_alloc();
}

int main() {
  // Reserve some emergency memory
  emergencyMemory = new std::vector<char>(10 * 1024 * 1024); // 10MB

  // Set our handler
  std::set_new_handler(outOfMemoryHandler);

  try {
    // Try to allocate huge amount
    while (true) {
      new char[1024 * 1024 * 100]; // 100MB chunks
      std::cout << "Allocated 100MB\n";
    }
  } catch (std::bad_alloc &e) {
    std::cerr << "Caught bad_alloc\n";
  }
}