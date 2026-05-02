#include <iostream>
#include <stdexcept>
#include <string>

struct Tracer {
  std::string name;
  Tracer(std::string n) : name(std::move(n)) {
    std::cout << "  CTOR " << name << "\n";
  }
  ~Tracer() { std::cout << "  DTOR " << name << "\n"; }
};

void level_3() {
  std::cout << "entering level_3\n";
  Tracer c1("c1");
  Tracer c2("c2");
  std::cout << "  about to throw\n";
  throw std::runtime_error("boom"); // <-- THROW SITE
  std::cout << "  never runs\n";
}

void level_2() noexcept {
  std::cout << "entering level_2\n";
  Tracer b1("b1");
  level_3();       // <-- CALL SITE
  Tracer b2("b2"); // never constructed
  std::cout << "  never runs\n";
}

void level_1() {
  std::cout << "entering level_1\n";
  Tracer a1("a1");
  try {
    level_2(); // <-- CALL SITE WITH HANDLER
  } catch (const std::exception &e) {
    std::cout << "  CAUGHT: " << e.what() << "\n";
  }
  std::cout << "  level_1 continues normally\n";
}

int main() {
  level_1();
  std::cout << "main done\n";
}