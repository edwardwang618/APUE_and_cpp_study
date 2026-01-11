#include <cstring>
#include <iostream>
#include <utility>

class String {
  char *_data;
  size_t _size;

public:
  String(const char *s = "") : _size(strlen(s)) {
    _data = new char[_size + 1];
    strcpy(_data, s);
    std::cout << "Constructor: " << _data << "\n";
  }

  ~String() {
    std::cout << "Destructor: " << (_data ? _data : "null") << "\n";
    delete[] _data;
  }

  String(const String &other) : _size(other._size) {
    _data = new char[_size + 1];
    strcpy(_data, other._data);
    std::cout << "COPY constructor: " << _data << "\n";
  }

  String(String &&other) noexcept : _size(other._size), _data(other._data) {
    other._data = nullptr;
    other._size = 0;
    std::cout << "MOVE constructor: " << _data << "\n";
  }

  void swap(String &other) noexcept {
    std::swap(_data, other._data);
    std::swap(_size, other._size);
  }

  String &operator=(String other) {
    std::cout << "Assignment (other has: " << other._data << ")\n";
    swap(other);
    return *this;
  }
};

int main() {
  std::cout << "=== Creating s1, s2 ===\n";
  String s1("hello");
  String s2("world");

  std::cout << "\n=== s1 = s2 (lvalue, should COPY) ===\n";
  s1 = s2;

  std::cout << "\n=== s1 = String(\"temp\") (rvalue, should MOVE) ===\n";
  s1 = String("temp");

  std::cout << "\n=== s1 = std::move(s2) (rvalue, should MOVE) ===\n";
  s1 = std::move(s2);

  std::cout << "\n=== End of main ===\n";
}