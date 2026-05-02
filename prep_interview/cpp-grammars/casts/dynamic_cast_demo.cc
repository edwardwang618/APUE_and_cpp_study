// dynamic_cast_demo.cpp
// compile: g++ -std=c++17 -o dynamic_cast_demo dynamic_cast_demo.cpp
#include <iostream>
#include <typeinfo> // for std::bad_cast

class Shape {
public:
  // 至少一个 virtual 函数，使类成为 polymorphic
  // without this, dynamic_cast won't compile
  virtual ~Shape() = default;
  virtual double area() const = 0;
};

class Circle : public Shape {
  double r_;

public:
  Circle(double r) : r_(r) {}
  double area() const override { return 3.14159 * r_ * r_; }
  double radius() const { return r_; }
};

class Rectangle : public Shape {
  double w_, h_;

public:
  Rectangle(double w, double h) : w_(w), h_(h) {}
  double area() const override { return w_ * h_; }
  double width() const { return w_; }
  double height() const { return h_; }
};

void describe(Shape *s) {
  // 尝试转换为 Circle*
  // runtime check: examines the vtable / type_info of *s
  if (Circle *c = dynamic_cast<Circle *>(s)) {
    // 成功：s 确实指向 Circle
    std::cout << "Circle, radius=" << c->radius() << ", area=" << c->area()
              << std::endl;
  }
  // 尝试转换为 Rectangle*
  else if (Rectangle *r = dynamic_cast<Rectangle *>(s)) {
    std::cout << "Rectangle, " << r->width() << "x" << r->height()
              << ", area=" << r->area() << std::endl;
  } else {
    std::cout << "Unknown shape, area=" << s->area() << std::endl;
  }
}

void describe_ref(Shape &s) {
  // 引用版本：失败时抛出 std::bad_cast，不返回 nullptr
  try {
    Circle &c = dynamic_cast<Circle &>(s);
    std::cout << "It's a circle (ref), radius=" << c.radius() << std::endl;
  } catch (const std::bad_cast &e) {
    std::cout << "Not a circle: " << e.what() << std::endl;
  }
}

int main() {
  Circle circle(5.0);
  Rectangle rect(3.0, 4.0);

  describe(&circle); // Circle, radius=5, area=78.5398
  describe(&rect);   // Rectangle, 3x4, area=12

  describe_ref(circle); // It's a circle (ref), radius=5
  describe_ref(rect);   // Not a circle: std::bad_cast

  return 0;
}