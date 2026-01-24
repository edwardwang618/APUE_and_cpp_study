#include <iostream>
#include <memory>
#include <vector>

class Animal {
public:
  virtual void speak() const { std::cout << "Some sound\n"; }

  virtual ~Animal() = default; // 虚析构函数很重要！
};

class Dog : public Animal {
public:
  void speak() const override { std::cout << "Woof!\n"; }
};

class Cat : public Animal {
public:
  void speak() const override { std::cout << "Meow!\n"; }
};

class Bird : public Animal {
public:
  void speak() const override { std::cout << "Chirp!\n"; }
};

int main() {
  std::vector<std::unique_ptr<Animal>> animals;
  animals.push_back(std::make_unique<Dog>());
  animals.push_back(std::make_unique<Cat>());
  animals.push_back(std::make_unique<Bird>());

  for (const auto &animal : animals) {
    animal->speak(); // 运行时决定调用哪个版本
  }
}