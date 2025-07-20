#pragma once
#include <iostream>
#include <string>
#include <type_traits>

// Example class demonstrating template member function and variable constraints
template <typename T> class Container {
public:
  // This is OK - template member function
  template <typename U> void convert(const U &value) {
    if constexpr (std::is_same_v<T, std::string>) {
      data = std::to_string(value);
    } else {
      data = static_cast<T>(value);
    }
  }

  // This would be illegal if uncommented - virtual template member function
  /*
  template<typename U>
  virtual void virtualConvert(const U& value) {  // ERROR: templates can't be
  virtual data = static_cast<T>(value);
  }
  */

  // This is OK - static template variable outside class
  static inline T default_value = T();

  // This would be illegal if uncommented - non-static template variable inside
  // class
  /*
  template<typename U>
  U wrong_variable;  // ERROR: non-static member variable can't be a template
  */

  void print() const { std::cout << "Data: " << data << std::endl; }

private:
  T data;
};
