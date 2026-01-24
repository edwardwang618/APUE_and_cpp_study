#include <iostream>

template <typename Derived> class Animal {
public:
  void speak() { static_cast<Derived *>(this)->speak_impl(); }
};

class Dog : public Animal<Dog> {
public:
  void speak_impl() { std::cout << "Woof!\n"; }
};

class Cat : public Animal<Cat> {
public:
  void speak_impl() { std::cout << "Meow!\n"; }
};

template <typename T> void make_speak(Animal<T> &animal) { animal.speak(); }

int main() {
  Dog d;
  Cat c;
  make_speak(d); // Woof!
  make_speak(c); // Meow!
}