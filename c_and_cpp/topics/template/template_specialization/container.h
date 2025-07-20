#pragma once
#include <iostream>
#include <string>
#include <type_traits>

// 1. Basic class template
template <typename T> class Container {
public:
  Container(T val) : value(val) {}

  void print() const {
    std::cout << "Generic template: " << value << std::endl;
  }

  T getValue() const { return value; }

private:
  T value;
};

// 2. Full specialization for std::string
template <> class Container<std::string> {
public:
  Container(const std::string &val) : value(val) {}

  void print() const {
    std::cout << "String specialization: \"" << value << "\"" << std::endl;
  }

  std::string getValue() const { return value; }

  // Additional string-specific functionality
  int length() const { return value.length(); }

private:
  std::string value;
};

// 3. Partial specialization for pointers
template <typename T> class Container<T *> {
public:
  Container(T *val) : value(val) {}

  void print() const {
    std::cout << "Pointer specialization: ";
    if (value) {
      std::cout << *value;
    } else {
      std::cout << "nullptr";
    }
    std::cout << std::endl;
  }

  T *getValue() const { return value; }

  // Pointer-specific functionality
  bool isNull() const { return value == nullptr; }

private:
  T *value;
};

// 4. Function template
template <typename T> T max_value(T a, T b) { return (a > b) ? a : b; }

// 5. Function template specialization for const char*
template <> const char *max_value(const char *a, const char *b) {
  return (std::string(a) > std::string(b)) ? a : b;
}

// 6. Partial specialization for array types
template <typename T, size_t Size> class Container<T[Size]> {
public:
  Container(const T (&arr)[Size]) {
    for (size_t i = 0; i < Size; ++i) {
      data[i] = arr[i];
    }
  }

  void print() const {
    std::cout << "Array specialization: [";
    for (size_t i = 0; i < Size; ++i) {
      if (i > 0)
        std::cout << ", ";
      std::cout << data[i];
    }
    std::cout << "]" << std::endl;
  }

private:
  T data[Size];
};
