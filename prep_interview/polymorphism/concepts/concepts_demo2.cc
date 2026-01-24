#include <concepts>
#include <iostream>
#include <string>

template <typename T>
concept Stringifiable = requires(const T &t) {
  { t.to_string() } -> std::convertible_to<std::string>;
};

template <typename T>
concept Orderable = requires(const T &a, const T &b) {
  { a < b } -> std::convertible_to<bool>;
};

// Mixin 同时要求两个能力
template <typename Derived> class PrintableAndComparable {
public:
  void print() const
    requires Stringifiable<Derived>
  {
    std::cout << static_cast<const Derived *>(this)->to_string() << "\n";
  }

  bool operator>(const Derived &other) const
    requires Orderable<Derived>
  {
    return other < static_cast<const Derived &>(*this);
  }

  bool operator<=(const Derived &other) const
    requires Orderable<Derived>
  {
    return !(static_cast<const Derived &>(*this) > other);
  }

  bool operator>=(const Derived &other) const
    requires Orderable<Derived>
  {
    return !(static_cast<const Derived &>(*this) < other);
  }
};

class Person : public PrintableAndComparable<Person> {
  std::string name;
  int age;

public:
  Person(std::string n, int a) : name(std::move(n)), age(a) {}

  std::string to_string() const {
    return name + " (" + std::to_string(age) + ")";
  }

  bool operator<(const Person &other) const { return age < other.age; }
};

int main() {
  Person alice("Alice", 30);
  Person bob("Bob", 25);

  alice.print();                      // Alice (30)
  std::cout << (alice > bob) << "\n"; // 1
}