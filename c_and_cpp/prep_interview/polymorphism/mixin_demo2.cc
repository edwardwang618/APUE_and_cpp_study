#include <iostream>
#include <string>

// 基础类
struct Empty {};

// Mixin: 添加计数功能
template <typename Base> class WithCounter : public Base {
  int count = 0;

public:
  void increment() { ++count; }
  int get_count() const { return count; }
};

// Mixin: 添加名字功能
template <typename Base> class WithName : public Base {
  std::string name;

public:
  void set_name(const std::string &n) { name = n; }
  std::string get_name() const { return name; }
};

// Mixin: 添加时间戳功能
template <typename Base> class WithTimestamp : public Base {
  std::time_t created = std::time(nullptr);

public:
  std::time_t get_created() const { return created; }
};

// 组合使用
using CountedNamed = WithCounter<WithName<Empty>>;
using FullFeatured = WithTimestamp<WithCounter<WithName<Empty>>>;

int main() {
  FullFeatured obj;
  obj.set_name("Widget");
  obj.increment();
  obj.increment();

  std::cout << obj.get_name() << "\n";  // Widget
  std::cout << obj.get_count() << "\n"; // 2
}