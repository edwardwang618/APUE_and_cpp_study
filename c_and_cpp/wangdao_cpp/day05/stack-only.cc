#include <cstddef>

class StackOnly {
public:
  StackOnly() {}
  ~StackOnly() {}

private:
  // Prevent heap allocation
  void *operator new(size_t) = delete;
  void *operator new[](size_t) = delete;
};

int main() {
  StackOnly obj; // ✓ OK: stack
  // StackOnly* p = new StackOnly();  // ✗ ERROR: operator new deleted
}