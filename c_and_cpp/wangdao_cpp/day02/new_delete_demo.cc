#include <cstddef>
#include <iostream>
#include <new>

class Widget {
public:
  Widget() { std::cout << "constructed\n"; }
  ~Widget() { std::cout << "destroyed\n"; }

  static void* operator new(size_t size) {
    std::cout << "Custom allocation: " << size << " bytes\n";
    return ::operator new(size);
  }

  static void operator delete(void *p) {
    std::cout << "Custom deallocation\n";
    ::operator delete(p);
  }
};

int main() {
  // malloc: NO constructor/destructor called!
  Widget *w1 = (Widget *)malloc(sizeof(Widget));
  free(w1);

  // new: constructor AND destructor called
  Widget *w2 = new Widget(); // "constructed"
  delete w2;                 // "destroyed"
}