#include <iostream>
#include <vector>

struct A {
  A() { puts("Ctr"); }
  ~A() { puts("Dtr"); }
};

int main() {
  std::vector<A> v(2);
  v.erase(v.begin());
}