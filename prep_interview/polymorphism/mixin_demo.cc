#include <iostream>

// Mixin 1: 添加打印功能
template <typename Derived> class Printable {
public:
  void print() const {
    // 调用派生类的 to_string()
    std::cout << static_cast<const Derived *>(this)->to_string() << "\n";
  }

  void print_to(std::ostream &os) const {
    os << static_cast<const Derived *>(this)->to_string();
  }
};

// Mixin 2: 添加比较操作符
template <typename Derived> class Comparable {
public:
  // 只要求派生类实现 operator<，其他自动生成
  bool operator>(const Derived &other) const {
    return other < static_cast<const Derived &>(*this);
  }
  bool operator<=(const Derived &other) const {
    return !(static_cast<const Derived &>(*this) > other);
  }
  bool operator>=(const Derived &other) const {
    return !(static_cast<const Derived &>(*this) < other);
  }
  bool operator!=(const Derived &other) const {
    return static_cast<const Derived &>(*this) < other ||
           other < static_cast<const Derived &>(*this);
  }
  bool operator==(const Derived &other) const { return !(*this != other); }
};

// Mixin 3: 添加克隆功能
template <typename Derived> class Cloneable {
public:
  Derived clone() const { return Derived(*static_cast<const Derived *>(this)); }
};

// 使用：像积木一样组合
class Person : public Printable<Person>,
               public Comparable<Person>,
               public Cloneable<Person> {
  std::string name;
  int age;

public:
  Person(std::string n, int a) : name(std::move(n)), age(a) {}
  Person(const Person &) = default;

  // Printable 需要这个
  std::string to_string() const {
    return name + " (" + std::to_string(age) + ")";
  }

  // Comparable 需要这个
  bool operator<(const Person &other) const { return age < other.age; }
};

int main() {
  Person p1("Alice", 30);
  Person p2("Bob", 25);

  p1.print();                      // Alice (30)
  std::cout << (p1 > p2) << "\n";  // 1
  std::cout << (p1 == p2) << "\n"; // 0

  Person p3 = p1.clone();
  p3.print(); // Alice (30)

  p3.print_to(std::cout);
}